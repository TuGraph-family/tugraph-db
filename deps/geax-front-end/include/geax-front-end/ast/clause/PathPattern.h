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

#ifndef GEAXFRONTEND_AST_CLAUSE_PATHPATTERN_H_
#define GEAXFRONTEND_AST_CLAUSE_PATHPATTERN_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/PathChain.h"
#include "geax-front-end/ast/clause/PathPrefix.h"

namespace geax {
namespace frontend {

/**
 * An PathPattern node describes a path pattern in a GPML.
 */

enum class ExprConjunctionType : uint8_t {
    kMultiSet,
    kUnion,
    kMax,
};

inline const char* ToString(const ExprConjunctionType type) {
    static const StrArray<enumNum(ExprConjunctionType::kMax)> kArray = {
        "multiSet",
        "union",
    };
    const auto idx = static_cast<size_t>(type);
    return idx < kArray.size() ? kArray[idx] : geax::frontend::kUnknown;
}

inline bool ToEnum(std::string_view sv, ExprConjunctionType& mode) {
    static const std::unordered_map<std::string_view, ExprConjunctionType> kMap = {
        {"multiSet", ExprConjunctionType::kMultiSet},
        {"union", ExprConjunctionType::kUnion},
    };
    auto it = kMap.find(sv);
    return it == kMap.end() ? false : (mode = it->second, true);
}

class PathPattern : public AstNode {
public:
    PathPattern() : AstNode(AstNodeType::kPathPattern) {}
    ~PathPattern() = default;

    void appendChain(PathChain* chains) { chains_.emplace_back(chains); }
    void setChains(std::vector<PathChain*>&& chains) { chains_ = std::move(chains); }
    const std::vector<PathChain*>& chains() const { return chains_; }

    void setOpType(ExprConjunctionType type) { type_ = type; }
    const std::optional<ExprConjunctionType> opType() const { return type_; }

    void setAlias(std::string&& alias) { alias_ = std::move(alias); }
    const std::optional<std::string>& alias() { return alias_; }

    void setPrefix(PathPrefix* prefix) { prefix_ = prefix; }
    const std::optional<PathPrefix*>& prefix() const { return prefix_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::optional<std::string> alias_;
    std::optional<PathPrefix*> prefix_;
    std::vector<PathChain*> chains_;
    std::optional<ExprConjunctionType> type_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_PATHPATTERN_H_
