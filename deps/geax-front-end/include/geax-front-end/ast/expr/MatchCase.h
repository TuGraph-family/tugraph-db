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

#ifndef GEAXFRONTEND_AST_EXPR_MATCHCASE_H_
#define GEAXFRONTEND_AST_EXPR_MATCHCASE_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class MatchCase : public Expr {
public:
    MatchCase() : Expr(AstNodeType::kMatchCase), input_(nullptr) {}
    ~MatchCase() = default;

    void setInput(Expr* input) { input_ = input; }
    Expr* input() const { return input_; }

    void appendCase(Expr* cond, Expr* result) { cases_.emplace_back(cond, result); }
    void setCases(std::vector<std::tuple<Expr*, Expr*>>&& cases) { cases_ = std::move(cases); }
    const std::vector<std::tuple<Expr*, Expr*>>& cases() const { return cases_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    Expr* input_;
    std::vector<std::tuple<Expr*, Expr*>> cases_;
};  // class MatchCase

inline bool MatchCase::equals(const Expr& other) const {
    const auto& expr = static_cast<const MatchCase&>(other);
    bool ret = (nullptr != input_) && (nullptr != expr.input_) && *input_ == *expr.input_;
    ret = ret && cases_.size() == expr.cases_.size();
    for (auto i = 0u; i < cases_.size() && ret; ++i) {
        ret = (nullptr != std::get<0>(cases_[i])) && (nullptr != std::get<0>(expr.cases_[i])) &&
              (nullptr != std::get<1>(cases_[i])) && (nullptr != std::get<1>(expr.cases_[i]));
        ret = ret && *std::get<0>(cases_[i]) == *std::get<0>(expr.cases_[i]);
        ret = ret && (*std::get<1>(cases_[i]) == *std::get<1>(expr.cases_[i]));
    }
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_MATCHCASE_H_
