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

#ifndef GEAXFRONTEND_AST_CLAUSE_LABELAND_H_
#define GEAXFRONTEND_AST_CLAUSE_LABELAND_H_

#include "geax-front-end/ast/clause/LabelTree.h"

namespace geax {
namespace frontend {

class LabelAnd : public LabelTree {
public:
    LabelAnd() : LabelTree(AstNodeType::kLabelAnd), left_(nullptr), right_(nullptr) {}
    ~LabelAnd() = default;

    void setLeft(LabelTree* left) { left_ = left; }
    LabelTree* left() const { return left_; }

    void setRight(LabelTree* right) { right_ = right; }
    LabelTree* right() const { return right_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool equals(const LabelTree& other) const override;

    LabelTree* left_;
    LabelTree* right_;
};

inline bool LabelAnd::equals(const LabelTree& other) const {
    const auto& LabelTree = static_cast<const LabelAnd&>(other);
    bool ret = (nullptr != left_) && (nullptr != LabelTree.left_) && (nullptr != right_) &&
               (nullptr != LabelTree.right_);
    ret = ret && *left_ == *LabelTree.left_ && *right_ == *LabelTree.right_;
    return ret;
}

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_LABELAND_H_
