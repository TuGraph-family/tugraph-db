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
 *         Suosi.zl <suosi.zl@alibaba-inc.com>
 */

#ifndef GEAXFRONTEND_AST_EXPR_MKLIST_H_
#define GEAXFRONTEND_AST_EXPR_MKLIST_H_

#include <vector>
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class MkList : public Expr {
public:
    MkList() : Expr(AstNodeType::kMkList) {}
    ~MkList() = default;

    void appendElem(Expr* expr) { elems_.emplace_back(expr); }
    void setElems(std::vector<Expr*>&& elems) { elems_ = std::move(elems); }
    const std::vector<Expr*>& elems() const { return elems_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::vector<Expr*> elems_;
};  // class MkList

inline bool MkList::equals(const Expr& other) const {
    const auto& expr = static_cast<const MkList&>(other);
    bool ret = elems_.size() == expr.elems_.size();
    for (auto i = 0u; i < elems_.size() && ret; ++i) {
        ret = (nullptr != elems_[i]) && (nullptr != expr.elems_[i]);
        ret = ret && *elems_[i] == *expr.elems_[i];
    }
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_MKLIST_H_
