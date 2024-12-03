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

#pragma once

#include "tools/lgraph_log.h"
#include "core/data_type.h"
#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_node_index_seek.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/optimization/opt_pass.h"
#include "cypher/execution_plan/optimization/property_filter_detector.h"

namespace cypher {

typedef std::unordered_map<
    std::string, std::unordered_map<
        std::string, std::set<lgraph::FieldData>>> FilterCollections;

class ReplaceNodeScanWithIndexSeek : public OptPass {
 private:
    RTContext *ctx_ = nullptr;
    const lgraph::SchemaInfo *si_ = nullptr;

    void Impl(OpBase *root) {
        OpBase *op_filter = nullptr;
        FilterCollections filters;
        if (FindNodePropFilter(root, op_filter, filters)) {
            Replace(op_filter, filters);
        }
    }

    bool FindNodePropFilter(OpBase *root, OpBase *&op_filter, FilterCollections &filters) {
        auto op = root;
        if (op->type == OpType::FILTER) {
            auto filter = dynamic_cast<OpFilter *>(op);
            if (_CheckPropFilter(filter, filters)) {
                op_filter = op;
                return true;
            }
        }

        for (auto child : op->children) {
            if (FindNodePropFilter(child, op_filter, filters)) return true;
        }

        return false;
    }

    void Replace(OpBase *root, FilterCollections &filter_collections) {
        if (root->type == OpType::NODE_BY_LABEL_SCAN) {
            auto scan = dynamic_cast<NodeByLabelScan *>(root);
            auto label = scan->GetLabel();
            auto node = scan->GetNode();
            auto n = node->Alias();
            if (!filter_collections.count(n)) {
                return;
            }
            auto& filters = filter_collections.at(n);
            auto schema = si_->v_schema_manager.GetSchema(label);
            if (!schema) {
                return;
            }
            auto pk = schema->GetPrimaryField();
            if (filters.count(pk)) {
                std::vector<lgraph::FieldData> values;
                for (auto& val : filters.at(pk)) {
                    values.push_back(val);
                }
                auto parent = root->parent;
                auto op_node_index_seek = new NodeIndexSeek(node, scan->GetSymtab(), pk, values);
                op_node_index_seek->parent = parent;
                parent->RemoveChild(root);
                OpBase::FreeStream(root);
                parent->AddChild(op_node_index_seek);
                return;
            }
            for (auto& [k, set] : filters) {
                if (k == pk) {
                    continue;
                }
                if (!schema->TryGetFieldExtractor(k)->GetVertexIndex()) {
                    continue;
                }
                std::vector<lgraph::FieldData> values;
                for (auto& val : set) {
                    values.push_back(val);
                }
                auto parent = root->parent;
                auto op_node_index_seek = new NodeIndexSeek(node, scan->GetSymtab(), k, values);
                op_node_index_seek->parent = parent;
                parent->RemoveChild(root);
                OpBase::FreeStream(root);
                parent->AddChild(op_node_index_seek);
                return;
            }
            return;
        } else if (root->type == OpType::ALL_NODE_SCAN) {
            auto scan = dynamic_cast<AllNodeScan *>(root);
            auto node = scan->GetNode();
            auto n = node->Alias();
            if (!filter_collections.count(n)) {
                return;
            }
            const auto& filters = filter_collections.at(n);
            if (filters.size() != 1) {
                return;
            }
            std::vector<lgraph::FieldData> values;
            std::string field;
            for (auto& [k, v] : filters) {
                field = k;
                for (auto &item : v) {
                    values.push_back(item);
                }
                break;
            }
            auto parent = root->parent;
            auto op_node_index_seek = new NodeIndexSeek(node, scan->SymTab(), field, values);
            op_node_index_seek->parent = parent;
            parent->RemoveChild(root);
            OpBase::FreeStream(root);
            parent->AddChild(op_node_index_seek);
            return;
        }
        for (auto child : root->children) {
            Replace(child, filter_collections);
        }
    }

    bool _CheckPropFilter(OpFilter *&op_filter, FilterCollections &filters) {
        auto filter = op_filter->Filter();
        CYPHER_THROW_ASSERT(filter->Type() == lgraph::Filter::GEAX_EXPR_FILTER);
        auto geax_filter = ((lgraph::GeaxExprFilter *)filter.get())->GetArithExpr();
        geax::frontend::Expr *expr = geax_filter.expr_;
        PropertyFilterDetector detector;
        if (!detector.Build(expr)) return false;
        filters = detector.GetProperties();
        if (filters.empty()) return false;
        return true;
    }

 public:
    explicit ReplaceNodeScanWithIndexSeek(RTContext *ctx)
        : OptPass(typeid(ReplaceNodeScanWithIndexSeek).name()), ctx_(ctx) {}
    bool Gate() override { return true; }
    int Execute(OpBase *root) override {
        if (ctx_->graph_.empty()) {
            return 0;
        }
        ctx_->ac_db_ = std::make_unique<lgraph::AccessControlledDB>(
            ctx_->galaxy_->OpenGraph(ctx_->user_, ctx_->graph_));
        lgraph_api::GraphDB db(ctx_->ac_db_.get(), true);
        auto txn = db.CreateReadTxn();
        si_ = &txn.GetTxn()->GetSchemaInfo();
        Impl(root);
        txn.Abort();
        return 0;
    }
};
}  // namespace cypher
