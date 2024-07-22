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
// Created by wt on 19-10-22.
//

#pragma once

#include "cypher/execution_plan/ops/op.h"
#include "cypher/execution_plan/ops/op_argument.h"

namespace cypher {

class Apply : public OpBase {
    OpBase *lhs = nullptr;
    OpBase *rhs = nullptr;
    Argument *argument = nullptr;
    PatternGraph *pattern_graph = nullptr;

    void ResetStream(OpBase *root) {
        root->Reset();
        for (auto c : root->children) ResetStream(c);
    }

    OpResult PullFromLhs(RTContext *ctx) {
        auto res = lhs->Consume(ctx);
        if (res != OP_OK) return res;
        /* Reset the rhs op stream. Note that it is up to the operators to decide
         * whether to invalidate its entities (iterators).
         * Example 1.
         *   MATCH (a {id:'A'}),(b {id:'B'}) WITH a, b
         *   MATCH (c:Film) RETURN a,b,c
         *   (execution plan:)
         *   Produce Results
         *       Project [b,c,a]
         *           Apply
         *               Cartesian Product
         *                   Argument [a,b]
         *                   Node By Label Scan [c:Film]
         *               Project [b,a]
         *                   Cartesian Product
         *                       Node VertexIndex Seek [a]
         *                       Node VertexIndex Seek [b]
         *   The entity 'c' in rhs should keep valid after Reset().
         * Example 2.
         *   UNWIND ['Zhongshan', 'Shanghai'] AS x WITH x
         *   MATCH (a {name:x}) RETURN a,a.name
         *   (execution plan:)
         *   Produce Results
         *       Project [a.name,a]
         *           Apply
         *               Node VertexIndex Seek (Dynamic) [a]
         *                   Argument [x]
         *               Project [x]
         *                   Unwind [[Zhongshan,Shanghai],x]
         *   The entity 'a' in rhs should be invalidated after Reset(), since it should
         *   be re-initialized with a new 'x'.  */
        ResetStream(rhs);
        return OP_OK;
    }

 public:
    Apply(Argument *arg, PatternGraph *pattern)
        : OpBase(OpType::APPLY, "Apply"), argument(arg), pattern_graph(pattern) {}

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(children.size() == 2);
        lhs = children[1];
        rhs = children[0];
        argument->Receive(&lhs->record, pattern_graph);
        for (auto child : children) {
            auto res = child->Initialize(ctx);
            if (res != OP_OK) return res;
        }
        /* The Apply operator (i.e. the standard version) takes the row produced by
         * the right-hand side -- which at this point contains data from both the
         * left-hand and right-hand sides -- and yields it.  */
        record = rhs->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        /* perform a nested loop by taking a single row from the left-hand side,
         * and using the Argument operator on the right-hand side, execute the
         * operator tree on the right-hand side.  */
        if (state == StreamDepleted) return OP_DEPLETED;
        if (state == StreamUnInitialized) {
            if (PullFromLhs(ctx) != OP_OK) {
                // starting with lhs first
                state = StreamDepleted;
                return OP_DEPLETED;
            }
            state = StreamConsuming;
        }
        auto res = rhs->Consume(ctx);
        // then process rhs
        while (res != OP_OK) {
            if (PullFromLhs(ctx) != OP_OK) {
                state = StreamDepleted;
                return OP_DEPLETED;
            }
            res = rhs->Consume(ctx);
        }
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
