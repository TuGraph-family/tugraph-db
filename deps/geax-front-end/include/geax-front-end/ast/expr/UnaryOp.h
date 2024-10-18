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

#ifndef GEAXFRONTEND_AST_EXPR_UNARYOP_H_
#define GEAXFRONTEND_AST_EXPR_UNARYOP_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class UnaryOp : public Expr {
public:
    explicit UnaryOp(AstNodeType type) : Expr(type), expr_(nullptr) {}
    virtual ~UnaryOp() = default;

    void setExpr(Expr* expr) { expr_ = expr; }
    Expr* expr() const { return expr_; }

protected:
    bool equals(const Expr& other) const override;

private:
    Expr* expr_;
};  // class UnaryOp

inline bool UnaryOp::equals(const Expr& other) const {
    const auto& expr = static_cast<const UnaryOp&>(other);
    bool ret = (nullptr != expr_) && (nullptr != expr.expr_) && *expr_ == *expr.expr_;
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_UNARYOP_H_
