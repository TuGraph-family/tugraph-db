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
#include "cypher/execution_plan/ops/op_all_node_scan.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/optimization/opt_pass.h"
#include "cypher/execution_plan/optimization/property_filter_detector.h"

namespace cypher {

/*

Before:

 Produce Results
    Project [n]
        Filter [{n.name = Vanessa Redgrave}]
            All Node Scan [n]

After:

 Produce Results
    Project [n]
        Node Index Seek [n] name IN [Vanessa Redgrave]
*/

class LocateNodeByIndexedPropV2 : public OptPass {
    void _AdjustNodePropFilter(OpBase *root) {
        OpFilter *op_filter = nullptr;
        std::unordered_map<std::string,
                           std::unordered_map<std::string,
                           std::set<lgraph::FieldData>>> target_value_datas;

        if (FindNodePropFilter(root, op_filter, target_value_datas)) {
            auto op_post = op_filter->parent;

            Node *node;
            const SymbolTable *symtab;

            // TODO(anyone): rewrite this more elegantly
            if (op_filter->children[0]->type == OpType::ALL_NODE_SCAN) {
                node = dynamic_cast<AllNodeScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<AllNodeScan *>(op_filter->children[0])->sym_tab_;
            } else {
                node = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->sym_tab_;
            }

            // try to reuse NodeIndexSeek
            // Here implemention of NodeIndexSeek is tweaked to support multiple static values
            // Reason that not to build unwind：unwind needs one more symbol in symbol table
            auto it = target_value_datas.find(node->Alias());
            if (it == target_value_datas.end()) return;
            CYPHER_THROW_ASSERT(it->second.size() == 1);
            std::vector<lgraph::FieldData> values;
            std::string field;
            for (auto & [f, set] : it->second) {
                values.insert(values.end(), set.begin(), set.end());
                field = f;
            }
            auto op_node_index_seek = new NodeIndexSeek(node, symtab, field, values);
            op_node_index_seek->parent = op_post;
            op_post->RemoveChild(op_filter);
            OpBase::FreeStream(op_filter);
            op_filter = nullptr;
            op_post->AddChild(op_node_index_seek);
        }
    }

    bool _CheckPropFilter(OpFilter *&op_filter, std::unordered_map<std::string,
                          std::unordered_map<std::string,
                          std::set<lgraph::FieldData>>> &target_value_datas) {
        /**
         * @brief
         *  Check if Filter is filters to filter prop values
         *
         *  where n.prop = value  || OR || IN
         */

        auto filter = op_filter->Filter();
        CYPHER_THROW_ASSERT(filter->Type() == lgraph::Filter::GEAX_EXPR_FILTER);
        auto geax_filter = ((lgraph::GeaxExprFilter *)filter.get())->GetArithExpr();
        geax::frontend::Expr *expr = geax_filter.expr_;
        PropertyFilterDetector detector;
        if (!detector.Build(expr)) return false;
        target_value_datas = detector.GetProperties();
        if (target_value_datas.empty()) return false;
        return true;
    }

    bool FindNodePropFilter(OpBase *root, OpFilter *&op_filter,
                            std::unordered_map<std::string,
                            std::unordered_map<std::string,
                            std::set<lgraph::FieldData>>> &target_value_datas) {
        auto op = root;
        if (op->type == OpType::FILTER && op->children.size() == 1 &&
            (op->children[0]->type == OpType::ALL_NODE_SCAN ||
             op->children[0]->type == OpType::NODE_BY_LABEL_SCAN)) {
            op_filter = dynamic_cast<OpFilter *>(op);
            if (_CheckPropFilter(op_filter, target_value_datas)) {
                return true;
            }
        }

        for (auto child : op->children) {
            if (FindNodePropFilter(child, op_filter, target_value_datas)) return true;
        }

        return false;
    }

 public:
    LocateNodeByIndexedPropV2() : OptPass(typeid(LocateNodeByIndexedPropV2).name()) {}
    bool Gate() override { return true; }
    int Execute(OpBase *root) override {
        _AdjustNodePropFilter(root);
        return 0;
    }
};
}  // namespace cypher
