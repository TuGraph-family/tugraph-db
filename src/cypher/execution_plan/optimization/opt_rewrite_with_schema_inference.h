/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

//
// Created by seijiang on 23-07-19.
//
#pragma once

#include "db/galaxy.h"
#include "cypher/execution_plan/ops/op_expand_all.h"
#include "cypher/execution_plan/ops/op_all_node_scan.h"
#include "cypher/execution_plan/ops/op_all_node_scan_dynamic.h"
#include "cypher/execution_plan/ops/op_node_index_seek.h"
#include "cypher/execution_plan/ops/op_node_index_seek_dynamic.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan_dynamic.h"
#include "cypher/execution_plan/optimization/opt_pass.h"
#include "cypher/execution_plan/optimization/rewrite/schema_rewrite.h"

namespace cypher {

/* Opt Rewrite With Schema Inference:
 * Graph : MovieDemo
 * example Cypher:
 * match p=(n0)-[e0]->(n1)-[e1]->(n2)-[e2]->(m:keyword) return COUNT(p);
 * is equivalent to :
 * match p=(n0:user)-[e0:is_friend]->(n1:user)-[e1:rate]->(n2:movie)-[e2:has_keyword]->(m:keyword)
 *return COUNT(p);
 *
 * Plan before optimization:
 * Produce Results
 *     Aggregate [COUNT(p)]
 *         Expand(All) [n2 --> m ]
 *             Expand(All) [n1 --> n2 ]
 *                 Expand(All) [n0 --> n1 ]
 *                     All Node Scan [n0]
 *
 * Plan after optimization:
 * Produce Results
 *     Aggregate [COUNT(p)]
 *         Expand(All) [n2 --> m ]
 *             Expand(All) [n1 --> n2 ]
 *                 Expand(All) [n0 --> n1 ]
 *                     Node By Label Scan [n0:user]
 **/

// removed test case in cypher_plan_validate.json
/*
"schema_rewrite": [
    {
        "query": "MATCH p=(n1)-[r1]->(n2)-[r2]->(m:Person) return count(p)",
        "plan": "ReadOnly:1\nExecution Plan:\nProduce Results\n    Aggregate [count(p)]\n        Expand(All) [n2 --> m ]\n            Expand(All) [n1 --> n2 ]\n                Node By Label Scan [n1:Person]\n",
        "res": 1
    },
    {
        "query": "MATCH p1=(n1)-[r1]->(n2)-[r2]->(m1:City),p2=(n3)-[r3]->(m2:Film) return count(p1)",
        "plan": "ReadOnly:1\nExecution Plan:\nProduce Results\n    Aggregate [count(p1)]\n        Cartesian Product\n            Expand(All) [n2 --> m1 ]\n                Expand(All) [n1 --> n2 ]\n                    Node By Label Scan [n1:Person]\n            Expand(All) [n3 --> m2 ]\n                Node By Label Scan [n3:Person]\n",
        "res": 1
    },
    {
        "query": "MATCH p1=(n1)-[r1]->(n2)-[r2]->(m1:City) with count(p1) as cp match p1=(n1)-[r1]->(m1:Film) return count(p1)",
        "plan": "ReadOnly:1\nExecution Plan:\nProduce Results\n    Aggregate [count(p1)]\n        Apply\n            Expand(All) [n1 --> m1 ]\n                Node By Label Scan Dynamic [n1:Person]\n                    Argument [cp]\n            Aggregate [cp]\n                Expand(All) [n2 --> m1 ]\n                    Expand(All) [n1 --> n2 ]\n                        Node By Label Scan [n1:Person]\n",
        "res": 1
    }
]
*/

class OptRewriteWithSchemaInference : public OptPass {
    bool check_v_label_valid(const lgraph::SchemaInfo *schema_info, const std::string label) {
        auto vertex_labels = schema_info->v_schema_manager.GetAllLabels();
        if (!label.empty() &&
            std::find(vertex_labels.begin(), vertex_labels.end(), label) == vertex_labels.end()) {
            return false;
        }
        return true;
    }

    bool check_e_labels_valid(const lgraph::SchemaInfo *schema_info,
                              const std::set<std::string> labels) {
        auto edge_labels = schema_info->e_schema_manager.GetAllLabels();
        for (auto label : labels) {
            if (std::find(edge_labels.begin(), edge_labels.end(), label) == edge_labels.end()) {
                return false;
            }
        }
        return true;
    }

    // pattern graph in match clause can be divided to multiple maximal connected subgraphs
    // this function extracts V and E of each subgraphs and labels them after analysis
    void _ExtractStreamAndAddLabels(OpBase *root, const lgraph::SchemaInfo *schema_info) {
        CYPHER_THROW_ASSERT(root->type == OpType::EXPAND_ALL);
        SchemaNodeMap schema_node_map;
        SchemaRelpMap schema_relp_map;
        auto op = root;
        while (true) {
            if (auto expand_all = dynamic_cast<ExpandAll *>(op)) {
                auto start = expand_all->GetStartNode();
                auto relp = expand_all->GetRelationship();
                auto neighbor = expand_all->GetNeighborNode();
                if (!check_v_label_valid(schema_info, start->Label())) {
                    return;
                }
                if (!check_v_label_valid(schema_info, neighbor->Label())) {
                    return;
                }
                if (!check_e_labels_valid(schema_info, relp->Types())) {
                    return;
                }

                schema_node_map[start->ID()] = start->Label();
                schema_node_map[neighbor->ID()] = neighbor->Label();
                std::tuple<NodeID, NodeID, std::set<std::string>, parser::LinkDirection>
                    relp_map_value(start->ID(), neighbor->ID(), relp->Types(), relp->direction_);
                schema_relp_map[relp->ID()] = relp_map_value;
            } else if (op->type == OpType::VAR_LEN_EXPAND) {
                // currently do not process when var len ops exist
                return;
            } else if ((op->IsScan() || op->IsDynamicScan()) && op->type != OpType::ARGUMENT) {
                NodeID id;
                std::string label;
                if (auto all_node_scan = dynamic_cast<AllNodeScan *>(op)) {
                    id = all_node_scan->GetNode()->ID();
                    label = all_node_scan->GetNode()->Label();
                } else if (auto all_node_scan_dy = dynamic_cast<AllNodeScanDynamic *>(op)) {
                    id = all_node_scan_dy->GetNode()->ID();
                    label = all_node_scan_dy->GetNode()->Label();
                } else if (auto node_by_label_scan = dynamic_cast<NodeByLabelScan *>(op)) {
                    id = node_by_label_scan->GetNode()->ID();
                    label = node_by_label_scan->GetNode()->Label();
                } else if (auto node_by_label_scan_dy =
                               dynamic_cast<NodeByLabelScanDynamic *>(op)) {
                    id = node_by_label_scan_dy->GetNode()->ID();
                    label = node_by_label_scan_dy->GetNode()->Label();
                } else if (auto node_index_seek = dynamic_cast<NodeIndexSeek *>(op)) {
                    id = node_index_seek->GetNode()->ID();
                    label = node_index_seek->GetNode()->Label();
                } else if (auto node_index_seek_dy = dynamic_cast<NodeIndexSeekDynamic *>(op)) {
                    id = node_index_seek_dy->GetNode()->ID();
                    label = node_index_seek_dy->GetNode()->Label();
                }
                if (!check_v_label_valid(schema_info, label)) {
                    return;
                }
                schema_node_map[id] = label;
            }

            if (op->children.empty()) {
                break;
            }
            CYPHER_THROW_ASSERT(op->children.size() == 1);
            op = op->children[0];
        }
        // call schema function
        rewrite::SchemaRewrite schema_rewrite;
        std::vector<SchemaGraphMap> schema_graph_maps;
        schema_graph_maps =
            schema_rewrite.GetEffectivePath(*schema_info, &schema_node_map, &schema_relp_map);
        // currently rewrite only when there is just one effective path
        if (schema_graph_maps.size() != 1) {
            return;
        }
        schema_node_map = schema_graph_maps[0].first;
        schema_relp_map = schema_graph_maps[0].second;
        op = root;
        while (true) {
            if (auto expand_all = dynamic_cast<ExpandAll *>(op)) {
                auto start = expand_all->GetStartNode();
                auto relp = expand_all->GetRelationship();
                auto neighbor = expand_all->GetNeighborNode();
                if (schema_node_map.find(start->ID()) != schema_node_map.end()) {
                    start->SetLabel(schema_node_map.find(start->ID())->second);
                }
                if (schema_node_map.find(neighbor->ID()) != schema_node_map.end()) {
                    neighbor->SetLabel(schema_node_map.find(neighbor->ID())->second);
                }
                if (schema_relp_map.find(relp->ID()) != schema_relp_map.end()) {
                    relp->SetTypes(std::get<2>(schema_relp_map.find(relp->ID())->second));
                }
            } else if (auto all_node_scan = dynamic_cast<AllNodeScan *>(op)) {
                auto node = all_node_scan->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
                auto op_label_scan = new NodeByLabelScan(node, all_node_scan->SymTab());
                auto parent = all_node_scan->parent;
                for (auto child : all_node_scan->children) {
                    op_label_scan->AddChild(child);
                }
                parent->RemoveChild(all_node_scan);
                parent->AddChild(op_label_scan);
            } else if (auto all_node_scan_dy = dynamic_cast<AllNodeScanDynamic *>(op)) {
                auto node = all_node_scan_dy->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
                auto op_label_scan =
                    new NodeByLabelScanDynamic(node, all_node_scan_dy->SymTab());
                auto parent = all_node_scan_dy->parent;
                for (auto child : all_node_scan_dy->children) {
                    op_label_scan->AddChild(child);
                }
                parent->RemoveChild(all_node_scan_dy);
                parent->AddChild(op_label_scan);
            } else if (auto node_index_seek = dynamic_cast<NodeIndexSeek *>(op)) {
                auto node = node_index_seek->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
            } else if (auto node_index_seek_dy = dynamic_cast<NodeIndexSeekDynamic *>(op)) {
                auto node = node_index_seek_dy->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
            }
            if (op->children.empty()) {
                if (op->type == OpType::ALL_NODE_SCAN ||
                    op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                    delete op;
                }
                break;
            }
            CYPHER_THROW_ASSERT(op->children.size() == 1);
            auto child = op->children[0];
            if (op->type == OpType::ALL_NODE_SCAN || op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                delete op;
            }
            op = child;
        }
    }

    void _RewriteWithSchemaInference(OpBase *root, const lgraph::SchemaInfo *schema_info) {
        // do not optimize on single node and var len
        if (root->type == OpType::EXPAND_ALL) {
            _ExtractStreamAndAddLabels(root, schema_info);
        } else {
            for (auto child : root->children) {
                _RewriteWithSchemaInference(child, schema_info);
            }
        }
    }

 public:
    cypher::RTContext *_ctx;

    explicit OptRewriteWithSchemaInference(cypher::RTContext *ctx)
        : OptPass(typeid(OptRewriteWithSchemaInference).name()), _ctx(ctx) {}

    bool Gate() override { return true; }

    int Execute(OpBase *root) override {
        if (_ctx->graph_.empty()) {
            // Skip optimization when the graph name is empty.
            return 0;
        }
        _ctx->ac_db_ = std::make_unique<lgraph::AccessControlledDB>(
            _ctx->galaxy_->OpenGraph(_ctx->user_, _ctx->graph_));
        lgraph_api::GraphDB db(_ctx->ac_db_.get(), true);
        _ctx->txn_ = std::make_unique<lgraph_api::Transaction>(db.CreateReadTxn());
        const lgraph::SchemaInfo *schema_info = &_ctx->txn_->GetTxn()->GetSchemaInfo();
        _ctx->txn_.reset(nullptr);
        _RewriteWithSchemaInference(root, schema_info);
        return 0;
    }
};

}  // namespace cypher
