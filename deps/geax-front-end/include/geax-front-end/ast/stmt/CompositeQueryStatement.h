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

#ifndef GEAXFRONTEND_AST_STMT_COMPOSITEQUERYSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_COMPOSITEQUERYSTATEMENT_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/QueryConjunctionType.h"
#include "geax-front-end/ast/stmt/LinearQueryStatement.h"

namespace geax {
namespace frontend {

/**
 * NOTE(yaochi): I think `composite` is more precise than `set`, since `otherwise` is also
 * included here.
 */
class CompositeQueryStatement : public AstNode {
public:
    CompositeQueryStatement() : AstNode(AstNodeType::kCompositeStatement), head_(nullptr) {}
    ~CompositeQueryStatement() = default;

    void setHead(LinearQueryStatement* head) { head_ = head; }
    LinearQueryStatement* head() const { return head_; }

    void appendBody(QueryConjunctionType* type, LinearQueryStatement* statement) {
        body_.emplace_back(type, statement);
    }
    void setBody(std::vector<std::tuple<QueryConjunctionType*, LinearQueryStatement*>>&& body) {
        body_ = std::move(body);
    }
    const auto& body() const { return body_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    LinearQueryStatement* head_;
    std::vector<std::tuple<QueryConjunctionType*, LinearQueryStatement*>> body_;
};  // class CompositeQueryStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_COMPOSITEQUERYSTATEMENT_H_
