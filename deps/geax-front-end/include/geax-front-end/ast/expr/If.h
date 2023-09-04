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
 *         Lili <liangjingru.ljr@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_EXPR_IF_H_
#define GEAXFRONTEND_AST_EXPR_IF_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class If : public Expr {
public:
    If()
        : Expr(AstNodeType::kIf), condition_(nullptr), trueBody_(nullptr), falseBody_(nullptr) {}
    ~If() = default;

    void setCondition(Expr* condition) { condition_ = condition; }
    Expr* condition() const { return condition_; }

    void setTrueBody(Expr* trueBody) { trueBody_ = trueBody; }
    Expr* trueBody() const { return trueBody_; }

    void setFalseBody(Expr* falseBody) { falseBody_ = falseBody; }
    Expr* falseBody() const { return falseBody_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    Expr* condition_;
    Expr* trueBody_;
    Expr* falseBody_;
};  // class If

inline bool If::equals(const Expr& other) const {
    const auto& expr = static_cast<const If&>(other);
    bool ret = (nullptr != condition_) && (nullptr != expr.condition_) && (nullptr != trueBody_) &&
               (nullptr != expr.trueBody_) && (nullptr != falseBody_) &&
               (nullptr != expr.falseBody_);
    ret = ret && *condition_ == *expr.condition_ && *trueBody_ == *expr.trueBody_ &&
          *falseBody_ == *expr.falseBody_;
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_IF_H_
