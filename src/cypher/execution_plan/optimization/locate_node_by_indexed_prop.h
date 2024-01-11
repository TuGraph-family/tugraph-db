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
// Created by gelincheng on 1/21/22.
//

#pragma once

#include "tools/lgraph_log.h"
#include "core/data_type.h"
#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_node_index_seek.h"
#include "cypher/execution_plan/ops/op_all_node_scan.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/optimization/opt_pass.h"
#include "geax-front-end/ast/AstDumper.h"
#include "geax-front-end/ast/expr/BEqual.h"
#include "geax-front-end/ast/expr/GetField.h"
#include "geax-front-end/ast/expr/VDouble.h"
#include "geax-front-end/ast/expr/VString.h"

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

class LocateNodeByIndexedProp : public OptPass {
    void _AdjustNodePropFilter(OpBase *root) {
        OpFilter *op_filter = nullptr;
        std::string field;
        lgraph::FieldData value;
        std::vector<lgraph::FieldData> target_value_datas;

        if (FindNodePropFilter(root, op_filter, field, value, target_value_datas)) {
            auto op_post = op_filter->parent;

            Node *node;
            const SymbolTable *symtab;

            // TODO(anyone) 2/9/22 gelincheng: 这里可以写的再优雅些嘛？
            if (op_filter->children[0]->type == OpType::ALL_NODE_SCAN) {
                node = dynamic_cast<AllNodeScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<AllNodeScan *>(op_filter->children[0])->sym_tab_;
            } else {
                node = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->sym_tab_;
            }

            // try to reuse  NodeIndexSeek
            // 这里修改了NodeIndexSeek的实现，使其支持静态多值
            // 不选择直接构造一个unwind的原因：因为unwind需要增加一个符号，需要修改原先的symtab
            if (target_value_datas.empty()) target_value_datas.emplace_back(value);
            auto op_node_index_seek = new NodeIndexSeek(node, symtab, field, target_value_datas);
            op_node_index_seek->parent = op_post;
            op_post->RemoveChild(op_filter);
            OpBase::FreeStream(op_filter);
            op_filter = nullptr;
            op_post->AddChild(op_node_index_seek);
        }
    }

    bool getValueFromRangeFilter(const std::shared_ptr<lgraph::Filter> &filter, std::string &field,
                                 lgraph::FieldData &value) {
        if (filter->Type() != lgraph::Filter::Type::RANGE_FILTER) {
            return false;
        }
        auto range_filter = std::dynamic_pointer_cast<lgraph::RangeFilter>(filter);
        if (range_filter->GetAeLeft().type == ArithExprNode::AR_EXP_OPERAND &&
            range_filter->GetAeLeft().operand.type == ArithOperandNode::AR_OPERAND_VARIADIC &&
            !range_filter->GetAeLeft().operand.variadic.entity_prop.empty() &&
            range_filter->GetCompareOp() == lgraph::LBR_EQ &&
            range_filter->GetAeRight().operand.type == ArithOperandNode::AR_OPERAND_CONSTANT) {
            if (field.empty()) {
                // 右值可能会不是constant? 如果是para类型？需要处理嘛？
                field = range_filter->GetAeLeft().operand.variadic.entity_prop;
            }
            if (field != range_filter->GetAeLeft().operand.variadic.entity_prop) {
                return false;
            }
            // 这里默认是scalar,数组情况后续处理
            value = range_filter->GetAeRight().operand.constant.scalar;
            return true;
        }
        return false;
    }

    bool getValueFromGeaxExprFilter(const std::shared_ptr<lgraph::Filter> &filter,
                                    std::string &field, lgraph::FieldData &value) {
        if (filter->Type() != lgraph::Filter::Type::GEAX_EXPR_FILTER) {
            return false;
        }
        auto geax_expr_filter = std::dynamic_pointer_cast<lgraph::GeaxExprFilter>(filter);
        auto expr = geax_expr_filter->GetArithExpr().expr_;
        if (auto expr_equal = dynamic_cast<geax::frontend::BEqual *>(expr)) {
            if (auto expr_left = dynamic_cast<geax::frontend::GetField *>(expr_equal->left())) {
                field = expr_left->fieldName();
            } else {
                return false;
            }
            if (auto expr_right = dynamic_cast<geax::frontend::VString *>(expr_equal->right())) {
                value = lgraph::FieldData(expr_right->val());
            } else if (auto expr_right =
                           dynamic_cast<geax::frontend::VInt *>(expr_equal->right())) {
                value = lgraph::FieldData(expr_right->val());
            } else if (auto expr_right =
                           dynamic_cast<geax::frontend::VDouble *>(expr_equal->right())) {
                value = lgraph::FieldData(expr_right->val());
            } else if (auto expr_right =
                           dynamic_cast<geax::frontend::VBool *>(expr_equal->right())) {
                value = lgraph::FieldData(expr_right->val());
            } else {
                return false;
            }
            return true;
        }
        return false;
    }

    bool _CheckPropFilter(OpFilter *&op_filter, std::string &field, lgraph::FieldData &value,
                          std::vector<lgraph::FieldData> &target_value_datas) {
        /**
         * @brief
         *  检查Filter是不是符合属性值等值判断的filter
         *
         * 2/8 适用情况： where n.prop = value  || OR || IN
         */

        auto filter = op_filter->Filter();
        if (filter->Type() == lgraph::Filter::RANGE_FILTER) {
            return getValueFromRangeFilter(filter, field, value);
        } else if (filter->Type() == lgraph::Filter::TEST_IN_FILTER) {
            auto in_filter = std::dynamic_pointer_cast<lgraph::TestInFilter>(filter);

            if (in_filter->ae_left_.type == ArithExprNode::AR_EXP_OPERAND &&
                in_filter->ae_left_.operand.type == ArithOperandNode::AR_OPERAND_VARIADIC &&
                !in_filter->ae_left_.operand.variadic.entity_prop.empty()) {
                auto right_data = Entry(in_filter->ae_right_.operand.constant);
                if (!right_data.IsArray()) CYPHER_TODO();
                field = in_filter->ae_left_.operand.variadic.entity_prop;
                for (auto &r : *right_data.constant.array) {
                    target_value_datas.emplace_back(r);
                }
                return true;
            }
        } else if ((filter->Type() == lgraph::Filter::BINARY &&
                    filter->LogicalOp() == lgraph::LBR_OR)) {
            if (!getValueFromRangeFilter(filter->Right(), field, value)) return false;
            target_value_datas.emplace_back(value);
            while (!filter->Left()->IsLeaf()) {
                filter = filter->Left();
                if (filter->Type() != lgraph::Filter::BINARY ||
                    filter->LogicalOp() != lgraph::LBR_OR) {
                    return false;
                }
                if (!getValueFromRangeFilter(filter->Right(), field, value)) return false;
                target_value_datas.emplace_back(value);
            }
            if (!getValueFromRangeFilter(filter->Left(), field, value)) return false;
            target_value_datas.emplace_back(value);
            // 如果需要按照原来的输入顺序输出的话
            std::reverse(target_value_datas.begin(), target_value_datas.end());
            return true;
        } else if (filter->Type() == lgraph::Filter::GEAX_EXPR_FILTER) {
            return getValueFromGeaxExprFilter(filter, field, value);
        }
        return false;
    }

    bool FindNodePropFilter(OpBase *root, OpFilter *&op_filter, std::string &field,
                            lgraph::FieldData &value,
                            std::vector<lgraph::FieldData> &target_value_datas) {
        auto op = root;
        if (op->type == OpType::FILTER && op->children.size() == 1 &&
            (op->children[0]->type == OpType::ALL_NODE_SCAN ||
             op->children[0]->type == OpType::NODE_BY_LABEL_SCAN)) {
            op_filter = dynamic_cast<OpFilter *>(op);
            if (_CheckPropFilter(op_filter, field, value, target_value_datas)) {
                return true;
            }
        }

        for (auto child : op->children) {
            if (FindNodePropFilter(child, op_filter, field, value, target_value_datas)) return true;
        }

        return false;
    }

 public:
    LocateNodeByIndexedProp() : OptPass(typeid(LocateNodeByIndexedProp).name()) {}
    bool Gate() override { return true; }
    int Execute(OpBase *root) override {
        _AdjustNodePropFilter(root);
        return 0;
    }
};
}  // namespace cypher
