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

#ifndef GEAXFRONTEND_AST_STMT_STATEMENT_H_
#define GEAXFRONTEND_AST_STMT_STATEMENT_H_

#include "geax-front-end/ast/AstNode.h"

namespace geax {
namespace frontend {

/**
 * An Statement represents a basic statement in a query.
 *
 * The layout of the query statment family should be as the follows:
 *
 * ProcedureBody
 *    |
 *    v
 * StatementWithYield --> Statement
 *
 * A Statement could be (we only have query statements currently)
 * - CatalogModifyStatement
 * - LinearDataModifyingStatement
 * - CompositeQueryStatement (QueryStatement -> JOIN -> CompositeQueryStatement)
 *      A CompositeQueryStatement includes linearQueryStatements
 */
class Statement : public AstNode {
public:
    explicit Statement(AstNodeType nodeType) : AstNode(nodeType) {}
    virtual ~Statement() = default;
};  // class Statement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_STATEMENT_H_
