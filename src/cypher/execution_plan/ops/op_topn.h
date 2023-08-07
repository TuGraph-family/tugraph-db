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
// Created on 7/1/21.
//
#pragma once

#include <map>
#include "cypher/execution_plan/ops/op.h"
namespace cypher {

class TopN : public OpBase {
    friend class LazyProjectTopN;
    std::vector<Record, MemoryMonitorAllocator<Record>> buffer_;
    std::vector<std::pair<int, bool>> sort_items_;
    std::map<int, int> corresponding_order_;
    std::vector<ArithExprNode> sort_elements_;
    std::vector<ArithExprNode> return_elements_;
    size_t limit_ = 0;
    std::function<bool(const Record &, const Record &)> compare_;

    void _GetSortElements() {
        // get corresponding order
        // TODO(anyone): process sort_elements_alias_
        int id = 0;
        for (auto &item : sort_items_) {
            int idx = item.first;
            sort_elements_.emplace_back(return_elements_[idx]);
            corresponding_order_.emplace(idx, id++);
        }
    }

    OpResult HandOff(std::shared_ptr<Record> &r) {
        if (buffer_.empty()) return OP_DEPLETED;
        *r = buffer_.back();
        buffer_.pop_back();
        return OP_OK;
    }

 public:
    TopN(const std::vector<std::pair<int, bool>> &sort_items, size_t limit,
         const std::vector<ArithExprNode> return_elements)
        : OpBase(OpType::TOPN, "TopN"),
          sort_items_(sort_items),
          return_elements_(return_elements),
          limit_(limit) {
        _GetSortElements();
    }

    OpResult Initialize(RTContext *ctx) override {
        if (!children.empty()) {
            auto &child = children[0];
            auto res = child->Initialize(ctx);
            if (res != OP_OK) return res;
        }
        record = std::make_shared<Record>(return_elements_.size());
        compare_ = [&](const Record &lhs, const Record &rhs) -> bool {
            int lidx = 0;
            int ridx = 0;
            bool ascending = true;
            for (auto &item : sort_items_) {
                lidx = lhs.values.size() == sort_items_.size() ? corresponding_order_.at(item.first)
                                                               : item.first;
                ridx = item.first;
                ascending = item.second;
                if (lhs.values[lidx] != rhs.values[ridx]) {
                    break;
                }
            }
            return ascending ? lhs.values[lidx] < rhs.values[ridx]
                             : lhs.values[lidx] > rhs.values[ridx];
        };
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        auto postre = std::make_shared<Record>(return_elements_.size());
        auto rec = std::make_shared<Record>(sort_elements_.size());
        std::priority_queue<Record, std::vector<Record>, decltype(compare_)> pq(compare_);
        CYPHER_THROW_ASSERT(!children.empty());
        auto &child = children[0];
        while (child->Consume(ctx) == OP_OK) {
            std::shared_ptr<Record> r;
            r = child->record;
            // preproject part
            int re_idx = 0;
            for (auto &re : sort_elements_) {
                auto v = re.Evaluate(ctx, *r);
                rec->values[re_idx++] = v;
                // TODO(anyone) handle alias
            }
            // sort and limit part
            int changed = 0;
            if (pq.size() < limit_)
                changed = 1;
            else if (compare_(*rec.get(), pq.top()))
                changed = 2;
            // postproject part
            if (changed > 0) {
                re_idx = 0;
                for (auto &re : return_elements_) {
                    auto v = re.Evaluate(ctx, *r);
                    v.Snapshot();  // Take snapshot, otherwise the underlying iter be invalid
                    postre->values[re_idx++] = v;
                }
                if (changed == 2) {
                    pq.pop();
                }
                pq.push(*postre.get());
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
        return HandOff(record);
    }

    OpResult ResetImpl(bool complete) override { return OP_OK; }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(fma_common::ToString(sort_items_));
        str.append(" ,{").append(std::to_string(limit_)).append("}]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
