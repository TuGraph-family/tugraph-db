/**
 * Copyright 2023 AntGroup CO., Ltd.
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
 *
 *  Author:
 *         lili <liangjingru.ljr@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_CLAUSE_Edge_H_
#define GEAXFRONTEND_AST_CLAUSE_Edge_H_

#include <tuple>

#include "geax-front-end/ast/clause/EdgeLike.h"
#include "geax-front-end/ast/clause/ElementFiller.h"
#include "geax-front-end/ast/expr/Param.h"

namespace geax {
namespace frontend {

enum class EdgeDirection : uint8_t {
    kUndirected,         // ~[]~, ~
    kPointRight,         // -[]->, ->
    kPointLeft,          // <-[]-, <-
    kLeftOrUndirected,   // <~[]~, <~
    kRightOrUndirected,  // ~[]~>, ~>
    kLeftOrRight,        // <-[]->, <->
    kAnyDirected,        // -[]-, -
    kMax,
};
inline const char* ToString(EdgeDirection dir) {
    static const StrArray<enumNum(EdgeDirection::kMax)> kNameMap = {
        "Undirected",        "PointRight",  "PointLeft",   "LeftOrUndirected",
        "RightOrUndirected", "LeftOrRight", "AnyDirected",
    };
    const auto idx = static_cast<size_t>(dir);
    return idx < kNameMap.size() ? kNameMap[idx] : geax::frontend::kUnknown;
}
inline bool ToEnum(std::string_view sv, EdgeDirection& dir) {
    static const std::unordered_map<std::string_view, EdgeDirection> kDirMap = {
        {"Undirected", EdgeDirection::kUndirected},
        {"PointRight", EdgeDirection::kPointRight},
        {"PointLeft", EdgeDirection::kPointLeft},
        {"LeftOrUndirected", EdgeDirection::kLeftOrUndirected},
        {"RightOrUndirected", EdgeDirection::kRightOrUndirected},
        {"LeftOrRight", EdgeDirection::kLeftOrRight},
        {"AnyDirected", EdgeDirection::kAnyDirected},
    };
    auto it = kDirMap.find(sv);
    return it == kDirMap.end() ? false : (dir = it->second, true);
}

/**
 * An Edge represents an edge pattern in the traversal pattern.
 *
 * -[]-
 */
class Edge : public EdgeLike {
public:
    Edge() : EdgeLike(AstNodeType::kEdge), filler_(nullptr) {}
    ~Edge() = default;

    void setDirection(EdgeDirection direction) { direction_ = direction; }
    EdgeDirection direction() const { return direction_; }

    void setHopRange(IntParam&& low, std::optional<IntParam>&& high) {
        hopRange_ = std::make_tuple(std::move(low), std::move(high));
    }
    const std::optional<std::tuple<IntParam, std::optional<IntParam>>>& hopRange() const {
        return hopRange_;
    }

    void setFiller(ElementFiller* filler) { filler_ = filler; }
    ElementFiller* filler() const { return filler_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    EdgeDirection direction_;
    ElementFiller* filler_;
    std::optional<std::tuple<IntParam, std::optional<IntParam>>> hopRange_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_Edge_H_
