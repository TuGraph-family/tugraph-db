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

#ifndef GEAXFRONTEND_AST_EXPR_BINARYOP_H_
#define GEAXFRONTEND_AST_EXPR_BINARYOP_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class BinaryOp : public Expr {
public:
    explicit BinaryOp(AstNodeType type) : Expr(type), left_(nullptr), right_(nullptr) {}
    virtual ~BinaryOp() = default;

    void setLeft(Expr* left) { left_ = left; }
    Expr* left() const { return left_; }

    void setRight(Expr* right) { right_ = right; }
    Expr* right() const { return right_; }

protected:
    bool equals(const Expr& other) const override;

private:
    Expr* left_;
    Expr* right_;
};  // class BinaryOp

inline bool BinaryOp::equals(const Expr& other) const {
    const auto& expr = static_cast<const BinaryOp&>(other);
    bool ret = (nullptr != left_) && (nullptr != expr.left_) && (nullptr != right_) &&
               (nullptr != expr.right_);
    ret = ret && *left_ == *expr.left_ && *right_ == *expr.right_;
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_BINARYOP_H_
