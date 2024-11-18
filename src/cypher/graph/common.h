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
// Created by wt on 6/12/18.
//
#pragma once
#include <string>
#include <unordered_set>
#include "cypher/cypher_types.h"
#include "parser/expression.h"
namespace cypher {

typedef int64_t NodeID;
typedef int64_t RelpID;

struct Property {
    bool hasMapFieldName;
    std::string field;
    std::string value_alias;
    std::string map_field_name;
    lgraph::FieldData value;
    enum {
        NUL,        // empty
        PARAMETER,  // {name:$name}
        VALUE,      // {name:'Tom Hanks'}
        VARIABLE,   // UNWIND [1,2] AS mid MATCH (n {id:mid}) || WITH {a: 1, b: 2} as pair
    } type;

    Property() : type(NUL) {}
};

/* See also traversal::Path */
struct Path {
    std::vector<bool> dirs_;
    std::vector<uint16_t> lids_;
    std::vector<int64_t> ids_;

    Path() = default;

    bool Empty() const { return ids_.empty(); }

    size_t Length() const { return dirs_.size(); }

    void SetStart(int64_t start_id) {
        if (Length() != 0) throw std::runtime_error("Cannot set start for path.");
        ids_.clear();
        ids_.emplace_back(start_id);
    }

    void Append(const lgraph::EdgeUid &edge) {
        if (ids_.back() == edge.src) {
            // forward
            dirs_.push_back(true);
            lids_.push_back(edge.lid);
            ids_.push_back(edge.eid);
            ids_.push_back(edge.dst);
        } else if (ids_.back() == edge.dst) {
            // backward
            dirs_.push_back(false);
            lids_.push_back(edge.lid);
            ids_.push_back(edge.eid);
            ids_.push_back(edge.src);
        } else {
            throw std::runtime_error("The edge doesn't match the path's end.");
        }
    }

    void Reverse() {
        std::reverse(ids_.begin(), ids_.end());
        std::reverse(lids_.begin(), lids_.end());
        std::reverse(dirs_.begin(), dirs_.end());
        for (size_t i = 0; i < dirs_.size(); i++) dirs_[i] = !dirs_[i];
    }

    void PopBack() {
        dirs_.pop_back();
        lids_.pop_back();
        ids_.pop_back();
        ids_.pop_back();
    }

    void Clear() {
        dirs_.clear();
        lids_.clear();
        ids_.clear();
    }

    lgraph::EdgeUid GetNthEdge(size_t n) const {
        size_t length = dirs_.size();
        if (n >= length) {
            throw std::runtime_error("Access out of range.");
        }
        return dirs_[n]
                   ? lgraph::EdgeUid(ids_[0 + n * 2], ids_[2 + n * 2], lids_[n], 0, ids_[1 + n * 2])
                   : lgraph::EdgeUid(ids_[2 + n * 2], ids_[0 + n * 2], lids_[n], 0,
                                     ids_[1 + n * 2]);  // TODO(heng)
    }

    std::string ToString() const {
        std::string str = "[";
        int64_t src, dst;
        for (size_t i = 0; i < Length(); i++) {
            auto euid = GetNthEdge(i);
            if (dirs_[i]) {
                src = euid.src;
                dst = euid.dst;
            } else {
                src = euid.dst;
                dst = euid.src;
            }
            if (i == 0) str.append("V[").append(std::to_string(src)).append("],");
            str.append("E[").append(euid.ToString()).append("],");
            str.append("V[").append(std::to_string(dst)).append("]");
            if (i != Length() - 1) str.append(",");
        }
        str.append("]");
        return str;
    }
};
}  // namespace cypher
