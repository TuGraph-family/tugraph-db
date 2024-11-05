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

/*
 * written by botu.wzy
 */

#pragma once
#include <string>
#include <unordered_map>
#include <unordered_set>

#include "bolt/graph.h"
#include "bolt/path.h"
#include "value.h"

namespace common {
struct Node {
    int64_t id;
    std::unordered_set<std::string> labels;
    std::unordered_map<std::string, Value> properties;
    std::string ToString() const;
    bolt::Node ToBolt() const;
};

struct Relationship {
    int64_t id;
    int64_t src;
    int64_t dst;
    std::string type;
    std::unordered_map<std::string, Value> properties;
    std::string ToString() const;
    bolt::Relationship ToBolt() const;
    bolt::RelNode ToBoltUnbound() const;
};

struct PathElement {
    bool is_node;
    std::any data;
};

struct Path {
    std::vector<PathElement> data;
    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] bolt::InternalPath ToBolt() const;
};

enum class ResultType : char {
    Value        = 0,
    Node         = 1,
    Relationship = 2,
    Path         = 3
};

struct Result {
    std::any data;
    ResultType type;
    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] std::any ToBolt() const;
};

}

template <>
struct fmt::formatter<std::vector<common::Result>> {
    template <typename ParseContext>
    constexpr auto parse(ParseContext& ctx) { return ctx.begin(); }

    template <typename FormatContext>
    auto format(const std::vector<common::Result>& vec, FormatContext& ctx) {
        auto out = ctx.out();
        out = fmt::format_to(out, "[");
        for (auto it = vec.begin(); it != vec.end(); ++it) {
            if (it != vec.begin()) {
                out = fmt::format_to(out, ", ");
            }
            out = fmt::format_to(out, "{}", it->ToString());
        }
        out = fmt::format_to(out, "]");
        return out;
    }
};