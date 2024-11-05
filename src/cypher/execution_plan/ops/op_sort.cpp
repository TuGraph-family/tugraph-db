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
#include <queue>
#include "cypher/execution_plan/ops/op_sort.h"
#include "common/logger.h"

namespace cypher {

OpBase::OpResult Sort::Initialize(RTContext *ctx) {
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

OpBase::OpResult Sort::RealConsume(RTContext *ctx) {
    if (HandOff(record) == OP_OK) return OP_OK;
    // If we're here, we don't have any records to return
    // try to get records.
    std::priority_queue<Record, std::vector<Record>, decltype(compare_)> pq(compare_);
    CYPHER_THROW_ASSERT(!children.empty());
    auto &child = children[0];
    auto remakeRecord = [](Record &r) {
        for (auto &v : r.values) {
            if (v.IsNode()) {
                v.node = new cypher::Node(*v.node);
            } else if (v.IsRelationship()) {
                v.relationship = new cypher::Relationship(*v.relationship);
            }
        }
    };
    while (child->Consume(ctx) == OP_OK) {
        /* take snapshot of the record, otherwise we may lose result entries
             * in the following query:
             * MATCH (n) RETURN n,n.name AS name ORDER BY name  */
        //child->record->Snapshot();
        if (limit_ == 0) {
            buffer_.emplace_back(*child->record);
            remakeRecord(buffer_.back());
            continue;
        }
        if (pq.size() < limit_) {
            auto tmp = child->record;
            remakeRecord(*tmp);
            pq.push(*tmp);
        } else if (compare_(*child->record, pq.top())) {
            pq.pop();
            auto tmp = child->record;
            remakeRecord(*tmp);
            pq.push(*tmp);
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

std::string Sort::ToString() const {
    std::string str(name);
    str.append(" [").append(fmt::format("{}", sort_items_)).append("]");
    return str;
}

Sort::~Sort() {
    for (const auto& b : buffer_) {
        for (const auto& v : b.values) {
            if (v.IsNode()) {
                delete v.node;
            } else if (v.IsRelationship()) {
                delete v.relationship;
            }
        }
    }
}

}