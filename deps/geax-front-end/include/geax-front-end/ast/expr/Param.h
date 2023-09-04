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

#ifndef GEAXFRONTEND_AST_EXPR_PARAM_H_
#define GEAXFRONTEND_AST_EXPR_PARAM_H_

#include <variant>

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

using IntParam = std::variant<int64_t, Param*>;
using BoolParam = std::variant<bool, Param*>;

class Param : public Expr {
public:
    Param() : Expr(AstNodeType::kParam) {}
    ~Param() = default;

    void setName(std::string&& name) { name_ = std::move(name); }
    const std::string& name() const { return name_; }
    std::string& name() { return name_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::string name_;
};  // class Param

inline bool Param::equals(const Expr& other) const {
    const auto& expr = static_cast<const Param&>(other);
    return name_ == expr.name_;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_PARAM_H_
