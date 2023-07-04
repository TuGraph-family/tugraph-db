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
// Created by wt on 19-11-27.
//
#pragma once

#include "opt_pass.h"
#include "rewrite/schema_rewrite.h"

namespace cypher {

/* Count edges:
 * // count all edges
 * MATCH ()-[r]->() RETURN count(r)
 * // count edges of a certain type
 * MATCH ()-[r:WORKS_IN]->() RETURN count(r)
 * // count edges in a certain patter
 * MATCH (p:Person)-[r:WORKS_IN]->()
 * RETURN count(r) AS jobs
 *
 * Plan before optimization:
 * Produce Results
 *     Aggregate [count(r)]
 *         Expand(All) [_ANON_NODE_0 --> _ANON_NODE_2]
 *             All Node Scan [_ANON_NODE_0]
 *
 * Plan after optimization:
 * Produce Results
 *     RelationshipCountFromCountStore
 **/

class OptRewriteWithSchemaInference : public OptPass {
    // match子句中的模式图可以分为多个极大连通子图，该函数提取每个极大连通子图的点和边，经过分析后加上标签信息
    void _ExtractStreamAndAddLabels(OpBase *root, const lgraph::SchemaInfo *schema_info) {
        CYPHER_THROW_ASSERT(root->type == OpType::EXPAND_ALL);
        SchemaNodeMap schema_node_map;
        SchemaRelpMap schema_relp_map;
        auto op = root;
        while (true) {
            if (op->type == OpType::EXPAND_ALL) {
                // ExpandAll *op_expand_all=dynamic_cast<ExpandAll *>(op);
                auto start = dynamic_cast<ExpandAll *>(op)->GetStartNode();
                auto relp = dynamic_cast<ExpandAll *>(op)->GetRelationship();
                auto neighbor = dynamic_cast<ExpandAll *>(op)->GetNeighborNode();

                schema_node_map[start->ID()] = start->Label();
                schema_node_map[neighbor->ID()] = neighbor->Label();
                std::tuple<NodeID, NodeID, std::set<std::string>, parser::LinkDirection>
                    relp_map_value(start->ID(), neighbor->ID(), relp->Types(), relp->direction_);
                schema_relp_map[relp->ID()] = relp_map_value;
            }
            // 目前对可变长的情况不予处理
            else if (op->type == OpType::VAR_LEN_EXPAND) {
                return;
            } else if (op->type == OpType::ALL_NODE_SCAN) {
                auto op_node = dynamic_cast<AllNodeScan *>(op);
                auto node = op_node->GetNode();
                schema_node_map[node->ID()] = node->Label();
            } else if (op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                auto op_node = dynamic_cast<AllNodeScanDynamic *>(op);
                auto node = op_node->GetNode();
                schema_node_map[node->ID()] = node->Label();
            } else if (op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                auto op_node = dynamic_cast<NodeByLabelScan *>(op);
                auto node = op_node->GetNode();
                schema_node_map[node->ID()] = node->Label();
            } else if (op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                auto op_node = dynamic_cast<NodeByLabelScanDynamic *>(op);
                auto node = op_node->GetNode();
                schema_node_map[node->ID()] = node->Label();
            } else if (op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                auto op_node = dynamic_cast<NodeIndexSeek *>(op);
                auto node = op_node->GetNode();
                schema_node_map[node->ID()] = node->Label();
            } else if (op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                auto op_node = dynamic_cast<NodeIndexSeekDynamic *>(op);
                auto node = op_node->GetNode();
                schema_node_map[node->ID()] = node->Label();
            }

            if (op->children.empty()) {
                break;
            }
            CYPHER_THROW_ASSERT(op->children.size() == 1);
            op = op->children[0];
        }

        // 调用schema函数
        rewrite_cypher::SchemaRewrite schema_rewrite;
        std::vector<SchemaGraphMap> schema_graph_maps;
        schema_graph_maps =
            schema_rewrite.GetEffectivePath(*schema_info, &schema_node_map, &schema_relp_map);
        // 目前只对一条可行路径的情况进行重写
        if (schema_graph_maps.size() != 1) {
            return;
        }

        schema_node_map = schema_graph_maps[0].first;
        schema_relp_map = schema_graph_maps[0].second;
        op = root;
        while (true) {
            if (op->type == OpType::EXPAND_ALL) {
                auto op_expand_all = dynamic_cast<ExpandAll *>(op);
                auto start = op_expand_all->GetStartNode();
                auto relp = op_expand_all->GetRelationship();
                auto neighbor = op_expand_all->GetNeighborNode();
                if (schema_node_map.find(start->ID()) != schema_node_map.end()) {
                    start->SetLabel(schema_node_map.find(start->ID())->second);
                }
                if (schema_node_map.find(neighbor->ID()) != schema_node_map.end()) {
                    neighbor->SetLabel(schema_node_map.find(neighbor->ID())->second);
                }
                if (schema_relp_map.find(relp->ID()) != schema_relp_map.end()) {
                    relp->SetTypes(std::get<2>(schema_relp_map.find(relp->ID())->second));
                }
            }
            if (op->type == OpType::ALL_NODE_SCAN) {
                auto op_node = dynamic_cast<AllNodeScan *>(op);
                auto node = op_node->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
                auto op_label_scan = new NodeByLabelScan(node, op_node->GetSymbolTable());
                auto parent = op->parent;
                parent->RemoveChild(op);
                parent->AddChild(op_label_scan);
            } else if (op->type == OpType::ALL_NODE_SCAN_DYNAMIC) {
                auto op_node = dynamic_cast<AllNodeScanDynamic *>(op);
                auto node = op_node->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
                auto op_label_scan = new NodeByLabelScanDynamic(node, op_node->GetSymbolTable());
                auto parent = op->parent;
                parent->RemoveChild(op);
                parent->AddChild(op_label_scan);
            } else if (op->type == OpType::NODE_INDEX_SEEK) {
                auto op_node = dynamic_cast<NodeIndexSeek *>(op);
                auto node = op_node->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
            } else if (op->type == OpType::NODE_INDEX_SEEK_DYNAMIC) {
                auto op_node = dynamic_cast<NodeIndexSeekDynamic *>(op);
                auto node = op_node->GetNode();
                if (schema_node_map.find(node->ID()) != schema_node_map.end()) {
                    node->SetLabel(schema_node_map.find(node->ID())->second);
                }
            }

            if (op->children.empty()) {
                break;
            }
            CYPHER_THROW_ASSERT(op->children.size() == 1);
            op = op->children[0];
        }
    }

    void _RewriteWithSchemaInference(OpBase *root, const lgraph::SchemaInfo *schema_info) {
        // 对单独的点和可变长不予优化
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

    OptRewriteWithSchemaInference(cypher::RTContext *ctx)
        : OptPass(typeid(OptRewriteWithSchemaInference).name()), _ctx(ctx) {}

    bool Gate() override { return true; }

    int Execute(ExecutionPlan *plan) override {
        const lgraph::SchemaInfo *schema_info;
        if (_ctx->graph_.empty()) {
            _ctx->ac_db_.reset(nullptr);
            schema_info = nullptr;
        } else {
            _ctx->ac_db_ = std::make_unique<lgraph::AccessControlledDB>(
                _ctx->galaxy_->OpenGraph(_ctx->user_, _ctx->graph_));
            lgraph_api::GraphDB db(_ctx->ac_db_.get(), true);
            _ctx->txn_ = std::make_unique<lgraph_api::Transaction>(db.CreateReadTxn());
            schema_info = &_ctx->txn_->GetTxn()->GetSchemaInfo();
        }
        _ctx->txn_.reset(nullptr);
        _ctx->ac_db_.reset(nullptr);
        _RewriteWithSchemaInference(plan->Root(), schema_info);
        return 0;
    }
};

}  // namespace cypher
