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

#ifndef GEAXFRONTEND_AST_STMT_SELECTSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_SELECTSTATEMENT_H_

#include "geax-front-end/ast/expr/Expr.h"
#include "geax-front-end/ast/stmt/LinearQueryStatement.h"
#include "geax-front-end/ast/stmt/SimpleQueryStatement.h"

namespace geax {
namespace frontend {

class SelectStatement : public LinearQueryStatement {
public:
    SelectStatement()
        : LinearQueryStatement(AstNodeType::kSelectStatement), resultStatement_(nullptr) {}
    ~SelectStatement() = default;

    void setResultStatement(PrimitiveResultStatement* resultStatement) {
        resultStatement_ = resultStatement;
    }
    PrimitiveResultStatement* resultStatement() const { return resultStatement_; }

    void appendFromClause(std::string&& graphRef, SimpleQueryStatement* query) {
        fromClauses_.emplace_back(std::move(graphRef), query);
    }
    void setFromClauses(std::vector<std::tuple<std::string, SimpleQueryStatement*>>&& fromClauses) {
        fromClauses_ = std::move(fromClauses);
    }
    const std::vector<std::tuple<std::string, SimpleQueryStatement*>>& fromClauses() const {
        return fromClauses_;
    }

    void setWhere(Expr* where) { where_ = where; }
    const std::optional<Expr*>& where() const { return where_; }

    void setHaving(Expr* having) { having_ = having; }
    const std::optional<Expr*>& having() const { return having_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    PrimitiveResultStatement* resultStatement_;
    std::vector<std::tuple<std::string, SimpleQueryStatement*>> fromClauses_;
    std::optional<Expr*> where_;
    std::optional<Expr*> having_;
};  // class SelectStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_SELECTSTATEMENT_H_
