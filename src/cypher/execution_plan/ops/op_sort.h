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
// Created by wt on 19-7-5.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class Sort : public OpBase {
    friend class LazyProjectTopN;
    std::vector<Record> buffer_;
    std::vector<std::pair<int, bool>> sort_items_;
    size_t limit_ = 0;
    std::function<bool(const Record &, const Record &)> compare_;
    // TODO(anyone) handle skip clause

    OpResult HandOff(std::shared_ptr<Record> &r) {
        if (buffer_.empty()) return OP_DEPLETED;
        *r = buffer_.back();
        buffer_.pop_back();
        return OP_OK;
    }

 public:
    Sort(const std::vector<std::pair<int, bool>> &sort_items, int64_t skip, int64_t limit)
        : OpBase(OpType::SORT, "Sort"),
          sort_items_(sort_items),
          limit_(limit < 0 ? 0 : (limit + (skip < 0 ? 0 : skip))) {}

    OpResult Initialize(RTContext *ctx) override;

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool complete) override { return OP_OK; }

    std::string ToString() const override;

    CYPHER_DEFINE_VISITABLE()
    CYPHER_DEFINE_CONST_VISITABLE()

    ~Sort() override;
};
}  // namespace cypher
