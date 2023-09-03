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

#ifndef GEAXFRONTEND_AST_CLAUSE_PATHCHAIN_H_
#define GEAXFRONTEND_AST_CLAUSE_PATHCHAIN_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/EdgeLike.h"
#include "geax-front-end/ast/clause/Node.h"


namespace geax {
namespace frontend {

/**
 * An PathChain node describes a path chain in a GPML.
 */
class PathChain : public AstNode {
public:
    PathChain() : AstNode(AstNodeType::kPathChain), head_(nullptr) {}
    ~PathChain() = default;

    void setHead(Node* head) { head_ = head; }
    Node* head() const { return head_; }

    void appendTail(EdgeLike* edge, Node* node) { tails_.emplace_back(edge, node); }
    void setTails(std::vector<std::tuple<EdgeLike*, Node*>>&& tails) {
        tails_ = std::move(tails);
    }
    const std::vector<std::tuple<EdgeLike*, Node*>>& tails() const { return tails_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    Node* head_;
    std::vector<std::tuple<EdgeLike*, Node*>> tails_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_PATHCHAIN_H_
