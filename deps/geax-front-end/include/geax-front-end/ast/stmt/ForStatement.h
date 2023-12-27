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

#ifndef GEAXFRONTEND_AST_STMT_FORSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_FORSTATEMENT_H_

#include "geax-front-end/ast/expr/Expr.h"
#include "geax-front-end/ast/stmt/SimpleQueryStatement.h"

namespace geax {
namespace frontend {

class ForStatement : public SimpleQueryStatement {
public:
    ForStatement() : SimpleQueryStatement(AstNodeType::kForStatement), expr_(nullptr) {}
    ~ForStatement() = default;

    void appendItem(std::string&& item) { itemsAlias_.emplace_back(std::move(item)); }
    void setItems(std::vector<std::string>&& itemsAlias) { itemsAlias_ = std::move(itemsAlias); }
    const std::vector<std::string>& itemsAlias() const { return itemsAlias_; }

    void setExpr(Expr* expr) { expr_ = expr; }
    Expr* expr() const { return expr_; }

    void setOrdinalityOrOffset(bool isOrdinality, std::string&& alias) {
        ordinalityOrOffset_ = std::make_tuple(isOrdinality, std::move(alias));
    }
    const std::optional<std::tuple<bool, std::string>>& ordinalityOrOffset() const {
        return ordinalityOrOffset_;
    }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<std::string> itemsAlias_;
    Expr* expr_;
    std::optional<std::tuple<bool, std::string>> ordinalityOrOffset_;
    // (isOrdinality, offset)  ordianlity: (true, aliasname) or offset (false, aliasname)
};  // ForStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_FORSTATEMENT_H_
