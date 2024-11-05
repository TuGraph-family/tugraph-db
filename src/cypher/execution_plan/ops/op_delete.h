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

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class OpGqlDelete : public OpBase {
    const std::vector<std::string>& items_;
    bool summary_ = false;

    void DoDeleteVE(RTContext *ctx);
    void ResultSummary(RTContext *ctx);

 public:
    explicit OpGqlDelete(const std::vector<std::string>& items)
        : OpBase(OpType::GQL_DELETE, "Gql Delete")
        , items_(items) {
        state = StreamUnInitialized;
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(parent && children.size() < 2);
        summary_ = !parent->parent;
        for (auto child : children) {
            child->Initialize(ctx);
        }
        record = children[0]->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        if (children.size() != 1) CYPHER_INTL_ERR();
        auto child = children[0];
        /* Avoid reading after writing (RAW) by split reading-while-writing
         * into 2 steps:
         * 1. collect vertices&edges to delete, this op store the
         *    vids/edge_uids while child op move iterator to next;
         * 2. do the delete action;  */
        while (child->Consume(ctx) == OP_OK) DoDeleteVE(ctx);
        ResultSummary(ctx);
        state = StreamDepleted;
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
