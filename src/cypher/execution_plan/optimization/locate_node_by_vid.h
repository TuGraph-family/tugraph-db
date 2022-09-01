/*
 * @Author: gelincheng
 * @Date: 2022-01-11
 * @LastEditors: gelincheng
 * @Description:
 */

#pragma once

#include "opt_pass.h"

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
    void _AdjustNodeVidFilter(ExecutionPlan &plan) {
        OpFilter *op_filter = nullptr;
        ArithExprNode ae;
        std::vector<lgraph::VertexId> target_vids;
        if (_FindNodeVidFilter(plan.Root(), op_filter, target_vids)) {
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
            delete op_filter;
            op_filter = nullptr;
            op_post->AddChild(op_node_by_id_seek);
        }
    }

    lgraph::VertexId getVidFromRangeFilter(const std::shared_ptr<lgraph::Filter> &filter) {
        auto range_filter = std::dynamic_pointer_cast<lgraph::RangeFilter>(filter);
        // FMA_LOG() << range_filter->_ae_left.ToString();
        if (range_filter->_ae_left.type == ArithExprNode::AR_EXP_OP &&
            range_filter->_ae_left.op.func_name == "id" &&
            range_filter->_compare_op == lgraph::LBR_EQ) {
            if (!range_filter->_ae_right.operand.constant.IsInteger()) {
                return -1;  // TODO: 右值可能不是integer,这里用-1做flag合适嘛？
            };
            return range_filter->_ae_right.operand.constant.scalar.integer();
        }
        return -1;
    }

    bool _CheckVidFilter(OpFilter *&op_filter, std::vector<lgraph::VertexId> &target_vids) {
        /**
         * @brief
         *  检查Filter是不是符合有且近包含的Id()操作的filter
         *
         * 适用情况： where id(n) = 1  where id(n) =2 or id(n)=3
         *            where id(n) in [1,2,3]
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
                    target_vids.emplace_back(r.integer());
                }
                return true;
            }
        } else if (filter->Type() == lgraph::Filter::BINARY &&
                   filter->LogicalOp() == lgraph::LBR_OR) {
            /*
             * 将符合条件的Or类型的filter变化成list形式
             *
             * id(n)=1 or id(n)=3  or id(n)=4
             *
             * 混合其他的属性 暂时不管
             *
             *
             * TODO: 加更多的容错判断
             *
             * */
            vid = getVidFromRangeFilter(filter->Right());
            if (vid == -1) return false;
            target_vids.emplace_back(vid);
            while (!filter->Left()->IsLeaf()) {
                filter = filter->Left();
                vid = getVidFromRangeFilter(filter->Right());
                if (vid == -1) return false;
                target_vids.emplace_back(vid);
            }
            vid = getVidFromRangeFilter(filter->Left());
            if (vid == -1) return false;
            target_vids.emplace_back(vid);

            //如果需要按照原来的输入顺序输出的话
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

    int Execute(ExecutionPlan *plan) override {
        _AdjustNodeVidFilter(*plan);
        return 0;
    }
};

}  // namespace cypher
