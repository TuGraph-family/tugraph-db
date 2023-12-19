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

#ifndef GEAXFRONTEND_AST_STMT_CALLQUERYSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_CALLQUERYSTATEMENT_H_

#include "geax-front-end/ast/stmt/CallProcedureStatement.h"
#include "geax-front-end/ast/stmt/SimpleQueryStatement.h"

namespace geax {
namespace frontend {

class CallQueryStatement : public SimpleQueryStatement {
public:
    CallQueryStatement() : SimpleQueryStatement(AstNodeType::kCallQueryStatement) {}
    ~CallQueryStatement() = default;

    void setProcedureStatement(CallProcedureStatement* procedureStatement) {
        procedureStatement_ = procedureStatement;
    }
    CallProcedureStatement* procedureStatement() const { return procedureStatement_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    CallProcedureStatement* procedureStatement_;
};  // class CallQueryStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_CALLQUERYSTATEMENT_H_
