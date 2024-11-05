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

#ifndef GEAXFRONTEND_AST_CLAUSE_ORDERBYFIELD_H_
#define GEAXFRONTEND_AST_CLAUSE_ORDERBYFIELD_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class OrderByField : public AstNode {
public:
    OrderByField() : AstNode(AstNodeType::kOrderByField), field_(nullptr), order_(false) {}
    ~OrderByField() = default;

    void setOrder(bool order) { order_ = order; }
    bool order() const { return order_; }

    void setNullOrder(bool order) { nullOrder_ = order; }
    const std::optional<bool>& nullOrder() const { return nullOrder_; }

    void setField(Expr* field) { field_ = field; }
    Expr* field() const { return field_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    Expr* field_;
    // false -> ASC, true -> DESC
    bool order_;
    // false -> NullFirst, true -> NullLast, None -> depend on order
    std::optional<bool> nullOrder_;
};  // class OrderByField

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_ORDERBYFIELD_H_
