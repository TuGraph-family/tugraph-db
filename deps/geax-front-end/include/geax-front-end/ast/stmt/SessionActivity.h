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

#ifndef GEAXFRONTEND_AST_STMT_SESSIONACTIVITY_H_
#define GEAXFRONTEND_AST_STMT_SESSIONACTIVITY_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/stmt/Session.h"

namespace geax {
namespace frontend {

/**
 * This is one of the roots(session or transaction) of an AST.
 */
class SessionActivity : public AstNode {
public:
    SessionActivity() : AstNode(AstNodeType::kSessionActivity) {}
    ~SessionActivity() = default;

    void appendSession(Session* session) { sessions_.emplace_back(session); }
    void setSessions(std::vector<Session*>&& sessions) { sessions_ = std::move(sessions); }
    const std::vector<Session*>& sessions() const { return sessions_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<Session*> sessions_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_SESSIONACTIVITY_H_
