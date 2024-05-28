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

#ifndef GEAXFRONTEND_AST_CLAUSE_UNWIND_H_
#define GEAXFRONTEND_AST_CLAUSE_UNWIND_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class Unwind : public AstNode {
public:
    Unwind() : AstNode(AstNodeType::kUnwind) {}
    ~Unwind() = default;

    void setVariable(std::string&& variable) {variable_ = std::move(variable);}

    const std::string& variable() {return variable_;}

    void setExpr(Expr* expr) {exprList_ = expr;}

    const Expr* Expr() {return exprList_;}

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::string variable_;
    Expr* exprList_;
};  // class Unwind

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_UNWIND_H_
