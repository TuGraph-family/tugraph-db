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

//
// Created by wt on 21-11-10.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class Union : public OpBase {
    int stream_idx_;

    OpResult PullFromStreams(RTContext *ctx) {
        CYPHER_THROW_ASSERT(children.size() > 1);
        for (stream_idx_++; stream_idx_ < (int)children.size(); stream_idx_++) {
            auto child = children[stream_idx_];
            if (child->Consume(ctx) == OP_OK) {
                record = child->record;
                // Ready to continue.
                return OP_OK;
            }
        }
        /* If we're here, then we didn't manged to get new data.
         * Last stream depleted. */
        return OP_DEPLETED;
    }

 public:
    Union() : OpBase(OpType::UNION, "Union") {}

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        for (auto child : children) {
            auto res = child->Initialize(ctx);
            if (res != OP_OK) return res;
        }
        stream_idx_ = 0;
        record = children[0]->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        auto child = children[stream_idx_];
        if (child->Consume(ctx) == OP_OK) {
            return OP_OK;
        } else {
            // Failed to get data from current stream,
            // try pulling next stream for data.
            auto res = PullFromStreams(ctx);
            return res;
        }
    }

    OpResult ResetImpl(bool complete) override {
        stream_idx_ = 0;
        record = children[0]->record;
        return OP_OK;
    }

    std::string ToString() const override { return name; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
