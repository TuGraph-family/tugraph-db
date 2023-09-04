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

#ifndef GEAXFRONTEND_AST_STMT_STATEMENTWITHYIELD_H_
#define GEAXFRONTEND_AST_STMT_STATEMENTWITHYIELD_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/YieldField.h"
#include "geax-front-end/ast/stmt/Statement.h"

namespace geax {
namespace frontend {

class StatementWithYield : public AstNode {
public:
    StatementWithYield()
        : AstNode(AstNodeType::kStatementWithYield), yield_(nullptr), statement_(nullptr) {}
    ~StatementWithYield() = default;

    void setYield(YieldField* yield) { yield_ = yield; }
    YieldField* yield() const { return yield_; }

    void setStatement(Statement* statement) { statement_ = statement; }
    Statement* statement() const { return statement_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    YieldField* yield_;
    Statement* statement_;
};  // class StatementWithYield

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_STATEMENTWITHYIELD_H_
