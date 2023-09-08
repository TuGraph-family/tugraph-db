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

#ifndef GEAXFRONTEND_AST_EXPR_VSTRING_H_
#define GEAXFRONTEND_AST_EXPR_VSTRING_H_

#include "geax-front-end/ast/expr/Literal.h"

namespace geax {
namespace frontend {

class VString : public Literal {
public:
    VString() : Literal(AstNodeType::kVString) {}
    ~VString() = default;

    void setVal(std::string&& val) { val_ = std::move(val); }
    const std::string& val() const { return val_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::string val_;
};  // class VString

inline bool VString::equals(const Expr& other) const {
    const auto& expr = static_cast<const VString&>(other);
    return val_ == expr.val_;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_VSTRING_H_
