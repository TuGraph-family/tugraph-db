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

#ifndef GEAXFRONTEND_AST_EXPR_MULTICOUNT_H_
#define GEAXFRONTEND_AST_EXPR_MULTICOUNT_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class MultiCount : public Expr {
public:
    MultiCount() : Expr(AstNodeType::kMultiCount) {}
    ~MultiCount() = default;

    void appendArg(Expr* arg) { args_.emplace_back(arg); }
    void setArgs(std::vector<Expr*>&& args) { args_ = std::move(args); }
    const std::vector<Expr*>& args() const { return args_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::vector<Expr*> args_;
};  // class MultiCount

inline bool MultiCount::equals(const Expr& other) const {
    const auto& expr = static_cast<const MultiCount&>(other);
    bool ret = expr.args_.size() == args_.size();
    for (auto i = 0u; i < args_.size() && ret; ++i) {
        ret = (nullptr != args_[i]) && (nullptr != expr.args_[i]);
        ret = ret && *args_[i] == *expr.args_[i];
    }
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_MULTICOUNT_H_
