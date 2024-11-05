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
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#ifndef GEAXFRONTEND_AST_STMT_BINDTABLEINNERQUERY_H_
#define GEAXFRONTEND_AST_STMT_BINDTABLEINNERQUERY_H_

#include "geax-front-end/ast/stmt/BindingTableExpr.h"
#include "geax-front-end/ast/stmt/ProcedureBody.h"

namespace geax {
namespace frontend {

class BindingTableInnerQuery : public BindingTableExpr {
public:
    BindingTableInnerQuery() : BindingTableExpr(AstNodeType::kBindingTableInnerQuery) {}
    ~BindingTableInnerQuery() = default;

    void setBody(ProcedureBody* body) { body_ = body; }
    ProcedureBody* body() { return body_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    ProcedureBody* body_;
};  // class BindingTableInnerQuery

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_BINDTABLEINNERQUERY_H_
