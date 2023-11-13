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

#ifndef GEAXFRONTEND_AST_STMT_CATALOGMODIFYSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_CATALOGMODIFYSTATEMENT_H_

#include "geax-front-end/ast/stmt/Statement.h"
#include "geax-front-end/ast/stmt/SingleCatalogStatement.h"

namespace geax {
namespace frontend {

class CatalogModifyStatement : public Statement {
public:
    CatalogModifyStatement() : Statement(AstNodeType::kCatalogModifyStatement) {}
    ~CatalogModifyStatement() = default;

    void appendStatement(SingleCatalogStatement* statement) { statements_.emplace_back(statement); }
    void setStatements(std::vector<SingleCatalogStatement*>&& statements) {
        statements_ = std::move(statements);
    }
    const std::vector<SingleCatalogStatement*>& statements() const { return statements_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<SingleCatalogStatement*> statements_;
};  // class CatalogModifyStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_CATALOGMODIFYSTATEMENT_H_
