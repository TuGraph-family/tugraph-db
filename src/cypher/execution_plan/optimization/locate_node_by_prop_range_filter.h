/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "core/data_type.h"
#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/optimization/opt_pass.h"
#include "cypher/execution_plan/optimization/property_range_filter_detector.h"

namespace cypher {

/*

Before:

Produce Results
    Limit [5]
        Project [n]
            Filter [((n.born>1922) and (n.born<1989))]
                Node By Label Scan [n:person]

After:

Produce Results
    Limit [5]
        Project [n]
            Filter [((n.born>1922) and (n.born<1989))]
                Node By Label Scan [n:person] n.born[1922,1989)
*/

class LocateNodeByPropRangeFilter : public OptPass {
 public:
    LocateNodeByPropRangeFilter() : OptPass(typeid(LocateNodeByPropRangeFilter).name()) {}
    ~LocateNodeByPropRangeFilter() = default;
    bool Gate() override { return true; }
    int Execute(OpBase *root) override {
        _AdjustNodePropRangeFilter(root);
        return 0;
    }

 private:
    void _AdjustNodePropRangeFilter(OpBase *root) {
        OpFilter *op_filter = nullptr;
        std::unordered_map<
            std::string,
            std::unordered_map<std::string, std::pair<lgraph::FieldData, lgraph::FieldData>>>
            target_value_datas;

        if (FindNodePropRangeFilter(root, op_filter, target_value_datas)) {
            auto op_child = op_filter->children[0];
            CYPHER_THROW_ASSERT(op_child->type == OpType::NODE_BY_LABEL_SCAN);

            Node *node = dynamic_cast<NodeByLabelScan *>(op_child)->GetNode();
            const SymbolTable *symtab = dynamic_cast<NodeByLabelScan *>(op_child)->sym_tab_;

            auto it = target_value_datas.find(node->Alias());
            if (it == target_value_datas.end()) return;

            auto op_node_by_label_scan =
                new NodeByLabelScan(node, symtab, std::move(it->second));
            op_filter->RemoveChild(op_child);
            OpBase::FreeStream(op_child);
            op_child = nullptr;
            op_filter->AddChild(op_node_by_label_scan);
        }
    }

    bool _CheckPropRangeFilter(
        OpFilter *&op_filter,
        std::unordered_map<
            std::string,
            std::unordered_map<std::string, std::pair<lgraph::FieldData, lgraph::FieldData>>>
            &target_value_datas) {
        auto filter = op_filter->Filter();
        CYPHER_THROW_ASSERT(filter->Type() == lgraph::Filter::GEAX_EXPR_FILTER);
        auto geax_filter = ((lgraph::GeaxExprFilter *)filter.get())->GetArithExpr();
        geax::frontend::Expr *expr = geax_filter.expr_;
        PropertyRangeFilterDetector detector;
        if (!detector.Build(expr)) return false;
        target_value_datas = detector.GetProperties();
        if (target_value_datas.empty()) return false;
        return true;
    }

    bool FindNodePropRangeFilter(
        OpBase *root, OpFilter *&op_filter,
        std::unordered_map<
            std::string,
            std::unordered_map<std::string, std::pair<lgraph::FieldData, lgraph::FieldData>>>
            &target_value_datas) {
        auto op = root;
        if (op->type == OpType::FILTER && op->children.size() == 1 &&
            op->children[0]->type == OpType::NODE_BY_LABEL_SCAN) {
            op_filter = dynamic_cast<OpFilter *>(op);
            if (_CheckPropRangeFilter(op_filter, target_value_datas)) {
                return true;
            }
        }

        for (auto child : op->children) {
            if (FindNodePropRangeFilter(child, op_filter, target_value_datas)) return true;
        }

        return false;
    }
};

}  // namespace cypher
