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

#include "cypher/execution_plan/optimization/opt_pass.h"

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
class PassReduceCount : public OptPass {
    static int _IdentifyResultAndAggregate(OpBase *root, ProduceResults *&op_result,
                                           Aggregate *&op_aggregate) {
        auto op = root;
        if (op->type != OpType::PRODUCE_RESULTS || op->children.size() != 1) return 1;
        op_result = dynamic_cast<ProduceResults *>(op);
        op = op->children[0];
        if (op->type != OpType::AGGREGATE || op->children.size() != 1) return 1;
        // Expecting a single aggregation
        op_aggregate = dynamic_cast<Aggregate *>(op);
        if (op_aggregate->aggregated_expressions_.size() != 1 ||
            !op_aggregate->noneaggregated_expressions_.empty()) {
            return 1;
        }
        // Make sure aggregation performs counting.
        auto &exp = op_aggregate->aggregated_expressions_[0];
        ArithExprNode ae(exp, op_aggregate->sym_tab_);
        CYPHER_THROW_ASSERT(ae.type == ArithExprNode::AR_EXP_OP);
        /* consider: RETURN count(*)+1 */
        if (ae.op.type != ArithOpNode::AR_OP_AGGREGATE || ae.op.func_name != "count") return 1;
        // Make sure Count acts on an alias.
        if (ae.op.children.size() != 2) return 1;
        auto &arg = ae.op.children[1];
        if (arg.type != ArithExprNode::AR_EXP_OPERAND ||
            arg.operand.type != ArithOperandNode::AR_OPERAND_VARIADIC ||
            !arg.operand.variadic.entity_prop.empty()) {
            return 1;
        }
        return 0;
    }

    /* Checks if execution plan only performs edge count */
    static int _IdentifyEdgeCount(OpBase *root, ProduceResults *&op_result,
                                  Aggregate *&op_aggregate, ExpandAll *&op_expand,
                                  OpBase *&op_scan) {
        if (_IdentifyResultAndAggregate(root, op_result, op_aggregate) != 0) return 1;
        OpBase *op = op_aggregate->children[0];

        if (op->type != OpType::EXPAND_ALL || op->children.size() != 1) return 1;
        op_expand = dynamic_cast<ExpandAll *>(op);
        op = op->children[0];

        if (op->type != OpType::ALL_NODE_SCAN && op->type != OpType::NODE_BY_LABEL_SCAN) {
            return 1;
        }
        op_scan = op;
        CYPHER_THROW_ASSERT(op->children.empty());
        return 0;
    }

    void _DoReduceEdgeCount(ExecutionPlan &plan) {
        ProduceResults *op_result = nullptr;
        Aggregate *op_aggregate = nullptr;
        ExpandAll *op_expand = nullptr;
        OpBase *op_scan = nullptr;
        if (_IdentifyEdgeCount(plan.Root(), op_result, op_aggregate, op_expand, op_scan) != 0) {
            return;
        }
        auto op_count =
            new RelationshipCount(op_expand->start_, op_expand->neighbor_, op_expand->relp_);
        op_result->RemoveChild(op_aggregate);
        op_result->AddChild(op_count);
        FMA_DBG() << "the stream to delete:";
        std::string s;
        OpBase::DumpStream(op_aggregate, 0, false, s);
        FMA_DBG() << s;
        OpBase::FreeStream(op_aggregate);
    }

 public:
    PassReduceCount() : OptPass(typeid(PassReduceCount).name()) {}

    bool Gate() override { return true; }

    int Execute(ExecutionPlan *plan) override {
        _DoReduceEdgeCount(*plan);
        return 0;
    }
};

}  // namespace cypher
