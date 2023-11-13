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

#ifndef GEAXFRONTEND_AST_EXPR_WINDOWING_H_
#define GEAXFRONTEND_AST_EXPR_WINDOWING_H_

#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class Windowing : public Expr {
public:
    Windowing() : Expr(AstNodeType::kWindowing), expr_(nullptr) {}
    ~Windowing() = default;

    void appendPartitionBy(Expr* expr) { partitionBy_.emplace_back(expr); }
    void setPartitionBy(std::vector<Expr*>&& partitionBy) { partitionBy_ = std::move(partitionBy); }
    const std::vector<Expr*>& partitionBy() const { return partitionBy_; }

    void appendOrderByClause(Expr* expr, bool flag) { orderByClause_.emplace_back(expr, flag); }
    void setOrderByClause(std::vector<std::tuple<Expr*, bool>>&& orderByClause) {
        orderByClause_ = std::move(orderByClause);
    }
    const std::vector<std::tuple<Expr*, bool>>& orderByClause() const { return orderByClause_; }

    void setExpr(Expr* expr) { expr_ = expr; }
    Expr* expr() const { return expr_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    std::vector<Expr*> partitionBy_;
    std::vector<std::tuple<Expr*, bool>> orderByClause_;
    Expr* expr_;
};  // class Windowing

inline bool Windowing::equals(const Expr& other) const {
    const auto& expr = static_cast<const Windowing&>(other);
    bool ret = (nullptr != expr_) && (nullptr != expr.expr_) && *expr_ == *expr.expr_;
    for (auto i = 0u; i < partitionBy_.size() && ret; ++i) {
        ret = (nullptr != partitionBy_[i]) && (nullptr != expr.partitionBy_[i]);
        ret = ret && *partitionBy_[i] == *expr.partitionBy_[i];
    }
    for (auto i = 0u; i < orderByClause_.size() && ret; ++i) {
        ret = (nullptr != std::get<0>(orderByClause_[i])) &&
              (nullptr != std::get<0>(expr.orderByClause_[i]));
        ret = ret && *std::get<0>(orderByClause_[i]) == *std::get<0>(expr.orderByClause_[i]);
        ret = ret && (std::get<1>(orderByClause_[i]) == std::get<1>(expr.orderByClause_[i]));
    }

    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_WINDOWING_H_
