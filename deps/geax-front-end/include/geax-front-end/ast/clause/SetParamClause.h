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

#ifndef GEAXFRONTEND_AST_CLAUSE_SETPARAMCLAUSE_H_
#define GEAXFRONTEND_AST_CLAUSE_SETPARAMCLAUSE_H_

#include "geax-front-end/ast/clause/SessionSetCommand.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

enum class SessionParamType : uint8_t {
    kTableType,
    kGraphType,
    kValueType,
    kMax,
};
inline const char* ToString(SessionParamType dir) {
    static const StrArray<enumNum(SessionParamType::kMax)> kNameMap = {"TableType", "GraphType",
                                                                       "ValueType"};
    const auto idx = static_cast<size_t>(dir);
    return idx < kNameMap.size() ? kNameMap[idx] : geax::frontend::kUnknown;
}
inline bool ToEnum(std::string_view sv, SessionParamType& dir) {
    static const std::unordered_map<std::string_view, SessionParamType> kDirMap = {
        {"TableType", SessionParamType::kTableType},
        {"GraphType", SessionParamType::kGraphType},
        {"ValueType", SessionParamType::kValueType},
    };
    auto it = kDirMap.find(sv);
    return it == kDirMap.end() ? false : (dir = it->second, true);
}

class SetParamClause : public SessionSetCommand {
public:
    SetParamClause() : SessionSetCommand(AstNodeType::kSetParam), expr_(nullptr) {}
    ~SetParamClause() = default;

    void setSessionParamType(SessionParamType paramType) { paramType_ = paramType; }
    SessionParamType paramType() const { return paramType_; }

    void setName(std::string&& name) { name_ = std::move(name); }
    const std::string& name() const { return name_; }

    void setExpr(Expr* expr) { expr_ = expr; }
    Expr* expr() const { return expr_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    SessionParamType paramType_;
    std::string name_;
    Expr* expr_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_SETPARAMCLAUSE_H_
