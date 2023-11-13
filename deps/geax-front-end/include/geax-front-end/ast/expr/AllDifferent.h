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

#ifndef GEAXFRONTEND_AST_EXPR_ALLDIFFERENT_H_
#define GEAXFRONTEND_AST_EXPR_ALLDIFFERENT_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class AllDifferent : public Expr {
public:
    AllDifferent() : Expr(AstNodeType::kAllDifferent) {}
    ~AllDifferent() = default;

    void appendItem(Expr* item) { items_.emplace_back(item); }
    void setItems(std::vector<Expr*>&& items) { items_ = std::move(items); }
    const std::vector<Expr*>& items() const { return items_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::vector<Expr*> items_;
};  // class AllDifferent

inline bool AllDifferent::equals(const Expr& other) const {
    const auto& expr = static_cast<const AllDifferent&>(other);
    bool ret = expr.items_.size() == items_.size();
    for (auto i = 0u; i < items_.size() && ret; ++i) {
        ret = (nullptr != items_[i]) && (nullptr != expr.items_[i]);
        ret = ret && *items_[i] == *expr.items_[i];
    }
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_ALLDIFFERENT_H_
