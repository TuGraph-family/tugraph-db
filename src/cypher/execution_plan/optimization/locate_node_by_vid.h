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
 *
 * @Author: gelincheng
 * @Date: 2022-01-11
 * @LastEditors: gelincheng
 * @Description:
 */

#pragma once

#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_all_node_scan.h"
#include "cypher/execution_plan/ops/op_node_by_id_seek.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/optimization/opt_pass.h"

namespace cypher {

/**
 * Plan before optimization:
 *
 *  Produce Results
 *      Project [n]
 *          Filter [{id(false,n) = 1}]
 *              All Node Scan [n]
 *
 * Plan after optimization:
 *
 *
 * Produce Results
 *      Project [n]
 *          Node By Id Seek[1,]
 *
 */

class LocateNodeByVid : public OptPass {
    void _AdjustNodeVidFilter(OpBase *root) {
        OpFilter *op_filter = nullptr;
        ArithExprNode ae;
        std::vector<lgraph::VertexId> target_vids;
        if (_FindNodeVidFilter(root, op_filter, target_vids)) {
            auto op_post = op_filter->parent;
            Node *node;
            const SymbolTable *symtab;

            if (op_filter->children[0]->type == OpType::ALL_NODE_SCAN) {
                node = dynamic_cast<AllNodeScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<AllNodeScan *>(op_filter->children[0])->sym_tab_;
            } else {
                node = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->GetNode();
                symtab = dynamic_cast<NodeByLabelScan *>(op_filter->children[0])->sym_tab_;
            }
            auto op_node_by_id_seek = new NodeByIdSeek(target_vids, node, symtab);
            op_node_by_id_seek->parent = op_post;
            op_post->RemoveChild(op_filter);
            OpBase::FreeStream(op_filter);
            op_filter = nullptr;
            op_post->AddChild(op_node_by_id_seek);
        }
    }

    lgraph::VertexId getVidFromRangeFilter(const std::shared_ptr<lgraph::Filter> &filter) {
        if (filter->Type() != lgraph::Filter::Type::RANGE_FILTER) {
            return -1;
        }
        auto range_filter = std::dynamic_pointer_cast<lgraph::RangeFilter>(filter);
        if (range_filter->GetAeLeft().type == ArithExprNode::AR_EXP_OP &&
            range_filter->GetAeLeft().op.func_name == "id" &&
            range_filter->GetCompareOp() == lgraph::LBR_EQ) {
            if (!range_filter->GetAeRight().operand.constant.IsInteger()) {
                return -1;  // TODO(anyone): return -1 is not ok when AeRight is not an integer.
            }
            return range_filter->GetAeRight().operand.constant.scalar.integer();
        }
        return -1;
    }

    bool _CheckVidFilter(OpFilter *&op_filter, std::vector<lgraph::VertexId> &target_vids) {
        /**
         * @brief
         *  Check if the Filter meets the requirement of having only Id() operation.
         *
         *  Applicable scenarios:
         *      where id(n) = 1
         *      where id(n) = 2 or id(n) = 3
         *      where id(n) in [1,2,3]
         */
        auto filter = op_filter->Filter();
        target_vids.clear();

        lgraph::VertexId vid = -1;
        if (filter->Type() == lgraph::Filter::RANGE_FILTER) {
            vid = getVidFromRangeFilter(filter);
            if (vid == -1) return false;
            target_vids.emplace_back(vid);
            return true;
        } else if (filter->Type() == lgraph::Filter::TEST_IN_FILTER) {
            auto in_filter = std::dynamic_pointer_cast<lgraph::TestInFilter>(filter);
            if (in_filter->ae_left_.type == ArithExprNode::AR_EXP_OP &&
                in_filter->ae_left_.op.func_name == "id") {
                //                ae = in_filter->ae_right_;
                auto right_data = Entry(in_filter->ae_right_.operand.constant);
                if (!right_data.IsArray()) CYPHER_TODO();
                for (auto &r : *right_data.constant.array) {
                    target_vids.emplace_back(r.scalar.integer());
                }
                return true;
            }
        } else if (filter->Type() == lgraph::Filter::BINARY &&
                   filter->LogicalOp() == lgraph::LBR_OR) {
            /*
             * Change the filter of the Or type that meets the conditions into a list form.
             *
             * id(n)=1 or id(n)=3  or id(n)=4
             *
             * Temporarily ignore other mixed conditions.
             *
             * TODO (anyone) Add more error handling.
             *
             **/
            vid = getVidFromRangeFilter(filter->Right());
            if (vid == -1) return false;
            target_vids.emplace_back(vid);
            while (!filter->Left()->IsLeaf()) {
                filter = filter->Left();
                if (filter->Type() != lgraph::Filter::BINARY ||
                    filter->LogicalOp() != lgraph::LBR_OR) {
                    return false;
                }
                vid = getVidFromRangeFilter(filter->Right());
                if (vid == -1) return false;
                target_vids.emplace_back(vid);
            }
            vid = getVidFromRangeFilter(filter->Left());
            if (vid == -1) return false;
            target_vids.emplace_back(vid);

            // If you need to output in the original input order.
            std::reverse(target_vids.begin(), target_vids.end());
            return true;
        }
        return false;
    }

    bool _FindNodeVidFilter(OpBase *root, OpFilter *&op_filter,
                            std::vector<lgraph::VertexId> &target_vids) {
        auto op = root;
        if (op->type == OpType::FILTER && op->children.size() == 1 &&
            (op->children[0]->type == OpType::ALL_NODE_SCAN ||
             op->children[0]->type == OpType::NODE_BY_LABEL_SCAN)) {
            op_filter = dynamic_cast<OpFilter *>(op);
            if (_CheckVidFilter(op_filter, target_vids)) {
                return true;
            }
        }

        for (auto child : op->children) {
            if (_FindNodeVidFilter(child, op_filter, target_vids)) return true;
        }

        return false;
    }

 public:
    LocateNodeByVid() : OptPass(typeid(LocateNodeByVid).name()) {}

    bool Gate() override { return true; }

    int Execute(OpBase *root) override {
        _AdjustNodeVidFilter(root);
        return 0;
    }
};

}  // namespace cypher
