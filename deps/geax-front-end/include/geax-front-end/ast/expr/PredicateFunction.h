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
*         lili <lipanpan.lpp@antgroup.com>
*/

#ifndef GEAXFRONTEND_AST_EXPR_PREDICATE_FUNCTION_
#define GEAXFRONTEND_AST_EXPR_PREDICATE_FUNCTION_

#include "geax-front-end/ast/expr/Expr.h"
#include <vector>

namespace geax {
namespace frontend {

class PredicateFunction : public Expr {
public:
   PredicateFunction() : Expr(AstNodeType::kPredicateFunction) {}
   ~PredicateFunction() = default;
   void setVariable(Expr* expr) {variable_ = expr; }
   void setInExpression(Expr* expr) {in_expression_ = expr; }
   void setWhereExpression(Expr* expr) {where_expression_ = expr; }
   void setPredicateType(int type) {predicate_type_ = type; }
   Expr* getVariable() {return variable_; }
   Expr* getInExpression() {return in_expression_; }
   Expr* getWhereExpression() {return where_expression_; }
   int getPredicateType() {return predicate_type_; }

   std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
   bool equals(const Expr& other) const override;

   // now (variable, in_expression, where_expression), scalable
   Expr* variable_;
   Expr* in_expression_;
   Expr* where_expression_;
   int predicate_type_;
};  // class ListComprehension

inline bool PredicateFunction::equals(const Expr& other) const {
   const auto& expr = dynamic_cast<const PredicateFunction&>(other);
   return variable_ != nullptr && expr.variable_ != nullptr &&
          variable_ == expr.variable_ && in_expression_ != nullptr &&
          expr.in_expression_ != nullptr && in_expression_ == expr.in_expression_ &&
          where_expression_ != nullptr && expr.where_expression_ != nullptr &&
          where_expression_ == expr.where_expression_ &&
          type_ == expr.type_;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_LIST_COMPREHENSION_
