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

#ifndef GEAXFRONTEND_AST_EXPR_ISLABELED_H_
#define GEAXFRONTEND_AST_EXPR_ISLABELED_H_

#include "geax-front-end/ast/clause/LabelTree.h"
#include "geax-front-end/ast/expr/UnaryOp.h"

namespace geax {
namespace frontend {

class IsLabeled : public UnaryOp {
public:
    IsLabeled() : UnaryOp(AstNodeType::kIsLabeled), labelTree_(nullptr) {}
    ~IsLabeled() = default;

    void setLabelTree(LabelTree* labelTree) { labelTree_ = labelTree; }
    LabelTree* labelTree() const { return labelTree_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const Expr& other) const override;

    LabelTree* labelTree_;
};  // class IsLabeled

inline bool IsLabeled::equals(const Expr& other) const {
    const auto& expr = static_cast<const IsLabeled&>(other);
    bool ret = UnaryOp::equals(other) && (nullptr != labelTree_) && (nullptr != expr.labelTree_) &&
               *labelTree_ == *expr.labelTree_;
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_EXPR_ISLABELED_H_
