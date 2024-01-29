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
// Created by wt on 19-7-5.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class Sort : public OpBase {
    friend class LazyProjectTopN;
    std::vector<Record, MemoryMonitorAllocator<Record>> buffer_;
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

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        record = std::make_shared<Record>();
        compare_ = [&](const Record &lhs, const Record &rhs) -> bool {
            int idx = 0;
            bool ascending = true;
            for (auto &item : sort_items_) {
                idx = item.first;
                ascending = item.second;
                if (lhs.values[idx] != rhs.values[idx]) break;
            }
            return ascending ? lhs.values[idx] < rhs.values[idx]
                             : lhs.values[idx] > rhs.values[idx];
        };
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (HandOff(record) == OP_OK) return OP_OK;
        // If we're here, we don't have any records to return
        // try to get records.
        std::priority_queue<Record, std::vector<Record>, decltype(compare_)> pq(compare_);
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        while (child->Consume(ctx) == OP_OK) {
            /* take snapshot of the record, otherwise we may lose result entries
             * in the following query:
             * MATCH (n) RETURN n,n.name AS name ORDER BY name  */
            child->record->Snapshot();
            if (limit_ == 0) {
                buffer_.emplace_back(*child->record);
                continue;
            }
            if (pq.size() < limit_) {
                pq.push(*child->record);
            } else if (compare_(*child->record, pq.top())) {
                pq.pop();
                pq.push(*child->record);
            }
        }
        if (!pq.empty()) {
            buffer_.clear();
            while (!pq.empty()) {
                buffer_.emplace_back(pq.top());
                pq.pop();
            }
        } else {
            std::sort(buffer_.begin(), buffer_.end(), compare_);
            std::reverse(buffer_.begin(), buffer_.end());
        }
        if (buffer_.empty()) return OP_DEPLETED;
        return HandOff(record);
    }

    OpResult ResetImpl(bool complete) override { return OP_OK; }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(fma_common::ToString(sort_items_)).append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
