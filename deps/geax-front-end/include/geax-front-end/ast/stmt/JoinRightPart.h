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

#ifndef GEAXFRONTEND_AST_STMT_JOINRIGHTPART_H_
#define GEAXFRONTEND_AST_STMT_JOINRIGHTPART_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/stmt/CompositeQueryStatement.h"
#include "geax-front-end/ast/stmt/PrimitiveResultStatement.h"

namespace geax {
namespace frontend {

enum class QueryJoinType : uint8_t {
    kLeftJoin,
    kRightJoin,
    kInnerJoin,
    kOuterJoin,
    kMax,
};

inline const char* ToString(QueryJoinType type) {
    static const StrArray<enumNum(QueryJoinType::kMax)> kNameMap = {
        "LeftJoin",
        "RightJoin",
        "InnerJoin",
        "OuterJoin",
    };
    const auto idx = static_cast<size_t>(type);
    return idx < kNameMap.size() ? kNameMap[idx] : geax::frontend::kUnknown;
}

inline bool ToEnum(std::string_view sv, QueryJoinType& type) {
    static const std::unordered_map<std::string_view, QueryJoinType> kTypeMap = {
        {"LeftJoin", QueryJoinType::kLeftJoin},
        {"RightJoin", QueryJoinType::kRightJoin},
        {"InnerJoin", QueryJoinType::kInnerJoin},
        {"OuterJoin", QueryJoinType::kOuterJoin},
    };
    auto it = kTypeMap.find(sv);
    return it == kTypeMap.end() ? false : (type = it->second, true);
}

class JoinRightPart : public AstNode {
public:
    JoinRightPart()
        : AstNode(AstNodeType::kJoinRight), rightBody_(nullptr), onExpr_(nullptr) {}
    ~JoinRightPart() = default;

    void setJoinType(QueryJoinType joinType) { joinType_ = joinType; }
    QueryJoinType joinType() const { return joinType_; }

    void setRightBody(CompositeQueryStatement* rightBody) { rightBody_ = rightBody; }
    CompositeQueryStatement* rightBody() const { return rightBody_; }

    void setOnExpr(Expr* onExpr) { onExpr_ = onExpr; }
    Expr* onExpr() const { return onExpr_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    QueryJoinType joinType_;
    CompositeQueryStatement* rightBody_;
    Expr* onExpr_;
};  // class JoinRightPart

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_JOINRIGHTPART_H_
