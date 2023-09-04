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

#ifndef GEAXFRONTEND_AST_EXPR_MKMAP_H_
#define GEAXFRONTEND_AST_EXPR_MKMAP_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class MkMap : public Expr {
public:
    MkMap() : Expr(AstNodeType::kMkMap) {}
    ~MkMap() = default;

    void appendElem(Expr* k, Expr* v) { elems_.emplace_back(k, v); }
    void setElems(std::vector<std::tuple<Expr*, Expr*>>&& elems) { elems_ = std::move(elems); }
    const std::vector<std::tuple<Expr*, Expr*>>& elems() const { return elems_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::vector<std::tuple<Expr*, Expr*>> elems_;
};  // class MkMap

inline bool MkMap::equals(const Expr& other) const {
    const auto& expr = static_cast<const MkMap&>(other);
    bool ret = elems_.size() == expr.elems_.size();
    for (auto i = 0u; i < elems_.size() && ret; ++i) {
        ret = (nullptr != std::get<0>(elems_[i])) && (nullptr != std::get<0>(expr.elems_[i])) &&
              (nullptr != std::get<1>(elems_[i])) && (nullptr != std::get<1>(expr.elems_[i]));
        ret = ret && *std::get<0>(elems_[i]) == *std::get<0>(expr.elems_[i]);
        ret = ret && (*std::get<1>(elems_[i]) == *std::get<1>(expr.elems_[i]));
    }
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_MKMAP_H_
