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
 */

#ifndef GEAXFRONTEND_AST_STMT_UnwindStatement_H_
#define GEAXFRONTEND_AST_STMT_UnwindStatement_H_

#include <string>
#include "geax-front-end/ast/expr/Expr.h"
#include "geax-front-end/ast/stmt/SimpleQueryStatement.h"

namespace geax {
namespace frontend {

class UnwindStatement : public SimpleQueryStatement {
public:
    explicit UnwindStatement() 
        : SimpleQueryStatement(AstNodeType::kUnwindStatement) {}
    ~UnwindStatement() = default;

    void setVariable(std::string&& v) {variable_ = std::move(v);}

    const std::string& variable() {return variable_;}

    void setList(Expr* expr) {list_ = expr;}

    Expr* list() const { return list_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }
private:
    std::string variable_;
    Expr* list_;
};  // class SchemaRef

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_UnwindStatement_H_
