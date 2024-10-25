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
#include "cypher/execution_plan/ops/op_config.h"

namespace cypher {

class LimitCol : public OpBase {
    friend class LazyProjectTopN;
    size_t limit_ = 0;     // Max number of records to consume.
    size_t consumed_ = 0;  // Number of records consumed so far.

 public:
    explicit LimitCol(size_t limit) : OpBase(OpType::LIMIT, "Limit"), limit_(limit) {}

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        columnar_ = std::make_shared<DataChunk>();
        record = child->record;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (consumed_ >= limit_) return OP_DEPLETED;
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        auto res = child->Consume(ctx);
        columnar_ = child->columnar_;
        int usable_r = std::min(static_cast<size_t>(FLAGS_BATCH_SIZE),
                                limit_ - consumed_);
        columnar_->TruncateData(usable_r);
        consumed_ += usable_r;
        return res;
    }

    OpResult ResetImpl(bool complete) override {
        consumed_ = 0;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(std::to_string(limit_)).append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
