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

#ifndef GEAXFRONTEND_AST_CLAUSE_ELEMENTFILLER_H_
#define GEAXFRONTEND_AST_CLAUSE_ELEMENTFILLER_H_

#include <optional>
#include <vector>

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/ElementPredicate.h"
#include "geax-front-end/ast/clause/LabelTree.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

/**
 * A ElementFiller represents a common part of vertex and edge patterns.
 */
class ElementFiller : public AstNode {
public:
    ElementFiller() : AstNode(AstNodeType::kElementFiller) {}
    ~ElementFiller() = default;

    void setV(std::string&& v) { v_ = std::move(v); }
    const std::optional<std::string>& v() const { return v_; }

    void setLabel(LabelTree* label) { label_ = label; }
    const std::optional<LabelTree*>& label() const { return label_; }

    void appendPredicate(ElementPredicate* predicate) {
        elementPredicates_.emplace_back(predicate);
    }
    void setPredicates(std::vector<ElementPredicate*>&& elementPredicates) {
        elementPredicates_ = std::move(elementPredicates);
    }
    const std::vector<ElementPredicate*>& predicates() const { return elementPredicates_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::optional<std::string> v_;  // also called element variable
    std::optional<LabelTree*> label_;
    std::vector<ElementPredicate*> elementPredicates_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_ELEMENTFILLER_H_
