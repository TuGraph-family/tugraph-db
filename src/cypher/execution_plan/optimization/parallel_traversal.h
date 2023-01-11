/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

/*
 * Created by ljr on 7/26/22
 */
#pragma once

#include "opt_pass.h"

namespace cypher {
/*
 * Traversal:
 * MATCH (f:Film)<-[:ACTED_IN]-(p:Person)-[:BORN_IN]->(c:City) return c.name, count(f) as sum order by sum desc
 * MATCH (n)-[hascreator:postHasCreator]->(m) WHERE hascreator.creationDate < 20111217 RETURN n;
 *
 * Plan before optimization:
 * Produce Results 
 *      Sort [{<1>: 1:0}] 
 *          Aggregate [c.name, sum] 
 *              Expand(All)
 *                  Expand(All)
 *                      Node By Label Scan [f:Film]
 *
 * Plan after optimization:
 * Produce Results 
 *      Sort [{<1>: 1:0}] 
 *          Traversal
 */
class ParallelTraversal : public OptPass {
    bool _JudgeRangeFilter(std::shared_ptr<lgraph::Filter> filter) {
        if ((filter->Type() == lgraph::Filter::RANGE_FILTER && filter->LogicalOp() == lgraph::LBR_EMPTY)) {
            return true;
        }
        if (filter->Left() && filter->Right()) {
            return _JudgeRangeFilter(filter->Left()) && _JudgeRangeFilter(filter->Right());
        } else if (filter->Left()) {
            return _JudgeRangeFilter(filter->Left());
        } else if (filter->Right()) {
            return _JudgeRangeFilter(filter->Right());
        }
        return false;
    } 

    bool _AdjustTraversal(ExecutionPlan &plan) {
        // 遍历查询计划，判断是否满足变换traversal条件
        OpBase *leaf_op = plan.Root(); 
        while (!leaf_op->children.empty() && leaf_op->type != OpType::AGGREGATE) {
            // 由于txn_.Abort()，无法传递以及处理其他需要txn的op
            if (leaf_op->children.size() > 1 || 
              (leaf_op->type != OpType::PRODUCE_RESULTS && leaf_op->type != OpType::SORT)) { return false; }
            leaf_op = leaf_op->children[0];
        } 
        if (leaf_op->type != OpType::AGGREGATE) { return false; }

        Aggregate *aggregate = dynamic_cast<Aggregate *>(leaf_op);
        // 目前仅处理单个聚合函数，且key的数量为0或1
        if (aggregate->GetAggregatedExpressions().size() != 1
            || aggregate->GetNoneAggregatedExpressions().size() > 1) { return false; }

        std::string start_alias;
        std::string end_alias;
        std::string start_label;
        std::vector<std::tuple<cypher::Node*, cypher::Node*, 
                    cypher::Relationship*, ExpandTowards, bool>> expands; // <start, neighbor, relp, towards, has_filter> has_filter暂时不用
        while (!leaf_op->children.empty()) {
            if (leaf_op->children.size() > 1) { return false; } 
            leaf_op = leaf_op->children[0];
            if (leaf_op->type == OpType::EXPAND_ALL) {
                // 目前暂不考虑对expand进行filter
                if (leaf_op->parent->type == OpType::FILTER) { return false; }
                if (end_alias.empty()) {
                    end_alias = dynamic_cast<ExpandAll *>(leaf_op)->neighbor_->Alias();
                }
                ExpandAll *expand = dynamic_cast<ExpandAll *>(leaf_op);
                expands.insert(expands.begin(), std::make_tuple(expand->start_, expand->neighbor_, expand->relp_, expand->expand_direction_, false));
            }
        }
        std::shared_ptr<lgraph::Filter> filter;
        if (leaf_op->type == OpType::NODE_BY_LABEL_SCAN) {
            start_alias = dynamic_cast<NodeByLabelScan *>(leaf_op)->GetNode()->Alias();
            start_label = dynamic_cast<NodeByLabelScan *>(leaf_op)->GetLabel();
            if (expands.empty()) { 
                end_alias = start_alias;
            }
            if (leaf_op->parent->type == OpType::FILTER) {
                OpFilter* op_filter = dynamic_cast<OpFilter *>(leaf_op->parent);
                filter = op_filter->Filter();
                // 目前只处理RANGE_FILTER，可嵌套
                if (!_JudgeRangeFilter(filter)) { return false; }
            }
        } else {
            return false; // plan最后节点必须为node_by_label_scan
        }
        if (start_alias.empty() || end_alias.empty()) { return false; }

        ArithExprNode agg_expr;
        ArithExprNode noneagg_expr;
        if (aggregate->GetNoneAggregatedExpressions().size() == 1) { // 有key
            agg_expr = aggregate->GetAggregatedExpressions()[0];
            noneagg_expr = aggregate->GetNoneAggregatedExpressions()[0];
            if (noneagg_expr.operand.variadic.alias != end_alias || agg_expr.op.func_name != "count" 
                || agg_expr.op.children[1].operand.variadic.alias != start_alias) { // 判断是否为头+尾+count
                return false;
            }
        } else { // 无key
            agg_expr = aggregate->GetAggregatedExpressions()[0];
            if (agg_expr.op.func_name != "count" || agg_expr.op.children[1].operand.variadic.alias != start_alias) {
                return false;
            }
        }
        Traversal *traversal_op = new Traversal(start_label, start_alias, aggregate->GetResultSetHeader(), 
                                        agg_expr, noneagg_expr, expands);
        if (filter) {
            traversal_op->SetFilter(filter);
        }
        auto agg_p = aggregate->parent;
        agg_p->RemoveChild(aggregate);
        OpBase::FreeStream(aggregate);
        agg_p->AddChild(traversal_op);
        return true;
    }

 public:
    ParallelTraversal() : OptPass(typeid(ParallelTraversal).name()) {}

    bool Gate() override { return true; }

    int Execute(ExecutionPlan *plan) override {
        _AdjustTraversal(*plan);
        return 0;
    }
};
}  // namespace cypher
