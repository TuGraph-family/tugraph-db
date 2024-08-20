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

#include "cypher/execution_plan/ops/op_filter.h"
#include "cypher/execution_plan/ops/op_aggregate.h"
#include "cypher/execution_plan/ops/op_gql_traversal.h"
#include "cypher/execution_plan/ops/op_expand_all.h"
#include "cypher/execution_plan/ops/op_node_by_label_scan.h"
#include "cypher/execution_plan/optimization/opt_pass.h"
#include "cypher/execution_plan/optimization/traversal_filter_detector.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/expr/AggFunc.h"
#include "geax-front-end/ast/expr/GetField.h"
#include "geax-front-end/ast/expr/Ref.h"

namespace cypher {
/*
 * Traversal:
 * MATCH (f:Film)<-[:ACTED_IN]-(p:Person)-[:BORN_IN]->(c:City) return c.name, count(f) as sum order
 * by sum desc MATCH (n)-[hascreator:postHasCreator]->(m) WHERE hascreator.creationDate < 20111217
 * RETURN n;
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
class ParallelTraversalV2 : public OptPass {
    bool _JudgeRangeFilter(geax::frontend::Expr* expr) {
        TraversalFilterDetector detector;
        return detector.Build(expr);
    }

    bool _AdjustTraversal(OpBase *root) {
        // traverse the plan to see if the condition is satisfied.
        OpBase *leaf_op = root;
        while (!leaf_op->children.empty() && leaf_op->type != OpType::AGGREGATE) {
            // can not pass or process other OPs that needs txn due to txn_.Abort()
            if (leaf_op->children.size() > 1 ||
                (leaf_op->type != OpType::PRODUCE_RESULTS && leaf_op->type != OpType::SORT)) {
                return false;
            }
            leaf_op = leaf_op->children[0];
        }
        if (leaf_op->type != OpType::AGGREGATE) {
            return false;
        }

        Aggregate *aggregate = dynamic_cast<Aggregate *>(leaf_op);
        // only process single aggregation functions
        if (aggregate->GetAggregatedExpressions().size() != 1 ||
            aggregate->GetNoneAggregatedExpressions().size() > 1) {
            return false;
        }

        std::string start_alias;
        std::string end_alias;
        std::string start_label;
        // <start, neighbor, relp, towards, has_filter> has_filter is not used currently
        std::vector<
            std::tuple<cypher::Node *, cypher::Node *, cypher::Relationship *, ExpandTowards, bool>>
            expands;
        while (!leaf_op->children.empty()) {
            if (leaf_op->children.size() > 1) {
                return false;
            }
            leaf_op = leaf_op->children[0];
            if (leaf_op->type == OpType::EXPAND_ALL) {
                // do not filter on expand currently
                if (leaf_op->parent->type == OpType::FILTER) {
                    return false;
                }
                if (end_alias.empty()) {
                    end_alias = dynamic_cast<ExpandAll *>(leaf_op)->neighbor_->Alias();
                }
                ExpandAll *expand = dynamic_cast<ExpandAll *>(leaf_op);
                expands.insert(expands.begin(),
                               std::make_tuple(expand->start_, expand->neighbor_, expand->relp_,
                                               expand->expand_direction_, false));
            }
        }
        geax::frontend::Expr* filter = nullptr;
        if (leaf_op->type == OpType::NODE_BY_LABEL_SCAN) {
            start_alias = dynamic_cast<NodeByLabelScan *>(leaf_op)->GetNode()->Alias();
            start_label = dynamic_cast<NodeByLabelScan *>(leaf_op)->GetLabel();
            if (expands.empty()) {
                end_alias = start_alias;
            }
            if (leaf_op->parent->type == OpType::FILTER) {
                OpFilter *op_filter = dynamic_cast<OpFilter *>(leaf_op->parent);

                if (op_filter->Filter()->Type() != lgraph::Filter::GEAX_EXPR_FILTER) return false;
                auto geax_filter = ((lgraph::GeaxExprFilter *)
                                        op_filter->Filter().get())->GetArithExpr();
                filter = geax_filter.expr_;

                // only process RANGE_FILTER, it can be recursive
                if (!_JudgeRangeFilter(filter)) {
                    return false;
                }
            }
        } else {
            return false;  // the leaf node must be a node_by_label_scan in the plan
        }
        if (start_alias.empty() || end_alias.empty()) {
            return false;
        }

        ArithExprNode agg_expr;
        ArithExprNode noneagg_expr;
        if (aggregate->GetNoneAggregatedExpressions().size() == 1) {  // has key
            agg_expr = aggregate->GetAggregatedExpressions()[0];
            if (agg_expr.type != ArithExprNode::ArithExprType::AR_AST_EXP) {
                return false;
            }
            if (agg_expr.evaluator->GetExpression()->type() !=
                geax::frontend::AstNodeType::kAggFunc) {
                return false;
            }
            auto agg = (geax::frontend::AggFunc *)(agg_expr.evaluator->GetExpression());
            if (agg->expr()->type() != geax::frontend::AstNodeType::kRef) {
                return false;
            }
            auto param_name = ((geax::frontend::Ref *)agg->expr())->name();
            noneagg_expr = aggregate->GetNoneAggregatedExpressions()[0];
            if (noneagg_expr.type != ArithExprNode::ArithExprType::AR_AST_EXP) {
                return false;
            }
            if (noneagg_expr.evaluator->GetExpression()->type() !=
                geax::frontend::AstNodeType::kGetField) {
                return false;
            }

            auto field = (geax::frontend::GetField *)(noneagg_expr.evaluator->GetExpression());
            auto field_name = ((geax::frontend::Ref *)field->expr())->name();
            if (field_name != end_alias ||
                agg->funcName() != geax::frontend::GeneralSetFunction::kCount ||
                param_name != start_alias) {  // judge if head+end+count
                return false;
            }
        } else {  // has no key
            agg_expr = aggregate->GetAggregatedExpressions()[0];
            if (agg_expr.type != ArithExprNode::ArithExprType::AR_AST_EXP) {
                return false;
            }
            if (agg_expr.evaluator->GetExpression()->type() !=
                geax::frontend::AstNodeType::kAggFunc) {
                return false;
            }
            auto agg = (geax::frontend::AggFunc *)(agg_expr.evaluator->GetExpression());
            if (agg->expr()->type() != geax::frontend::AstNodeType::kRef) {
                return false;
            }
            auto param_name = ((geax::frontend::Ref *)agg->expr())->name();
            if (agg->funcName() != geax::frontend::GeneralSetFunction::kCount ||
                param_name != start_alias) {
                return false;
            }
        }
        OpGqlTraversal *traversal_op =
            new OpGqlTraversal(start_label, start_alias, aggregate->GetResultSetHeader(), agg_expr,
                             noneagg_expr, expands);
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
    ParallelTraversalV2() : OptPass(typeid(ParallelTraversalV2).name()) {}

    ~ParallelTraversalV2() {}

    bool Gate() override { return true; }

    int Execute(OpBase *root) override {
        _AdjustTraversal(root);
        return 0;
    }
};
}  // namespace cypher
