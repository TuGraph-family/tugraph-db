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

#ifndef GEAXFRONTEND_AST_CLAUSE_GRAPHPATTERN_H_
#define GEAXFRONTEND_AST_CLAUSE_GRAPHPATTERN_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/PathPattern.h"
#include "geax-front-end/ast/clause/PathPrefix.h"
#include "geax-front-end/ast/clause/YieldField.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

enum class MatchMode {
    kRepeatable,
    kDifferent,
    kMax,
};

inline const char* ToString(MatchMode mode) {
    static const StrArray<enumNum(MatchMode::kMax)> kNameArray = {
        "Repeatable",
        "Different",
    };
    const auto idx = static_cast<size_t>(mode);
    return idx < kNameArray.size() ? kNameArray[idx] : geax::frontend::kUnknown;
}

inline bool ToEnum(std::string_view sv, MatchMode& mode) {
    static const std::unordered_map<std::string_view, MatchMode> kModeMap = {
        {"Repeatable", MatchMode::kRepeatable},
        {"Different", MatchMode::kDifferent},
    };
    auto it = kModeMap.find(sv);
    return it == kModeMap.end() ? false : (mode = it->second, true);
}

/**
 * An GraphPattern node descripes the Graph Pattern Matching Langunage(GPML)
 * derived from the ISO/GQL standard.
 *
 * We can transform exsting graph query languages of any kind into this structured
 * GPML, including ISO/GQL, SQL/PGQ, ORACLE/PGQL, cypher and gremlin,
 * which fullfills the core of a graph query language.
 *
 */
class GraphPattern : public AstNode {
public:
    GraphPattern() : AstNode(AstNodeType::kGraphPattern) {}
    ~GraphPattern() = default;

    void appendPathPattern(PathPattern* pathPattern) { pathPatterns_.emplace_back(pathPattern); }
    void setPathPatterns(std::vector<PathPattern*>&& pathPatterns) {
        pathPatterns_ = std::move(pathPatterns);
    }
    const std::vector<PathPattern*>& pathPatterns() const { return pathPatterns_; }

    void setMatchMode(MatchMode mode) { mode_ = mode; }
    const std::optional<MatchMode>& matchMode() const { return mode_; }

    void setKeep(PathPrefix* keep) { keep_ = keep; }
    const std::optional<PathPrefix*>& keep() const { return keep_; }

    void setWhere(Expr* where) { where_ = where; }
    const std::optional<Expr*>& where() const { return where_; }
    void resetWhere() { where_.reset(); }

    // NOTE(yaochi): this could be confict with system function yield()
    void setYield(YieldField* yield) { yield_ = yield; }
    const std::optional<YieldField*>& yield() const { return yield_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<PathPattern*> pathPatterns_;
    std::optional<MatchMode> mode_;
    std::optional<PathPrefix*> keep_;
    std::optional<Expr*> where_;
    std::optional<YieldField*> yield_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_GRAPHPATTERN_H_
