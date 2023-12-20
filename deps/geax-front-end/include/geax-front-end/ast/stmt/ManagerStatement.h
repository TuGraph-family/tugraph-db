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
 *         fangji.hcm <fangji.hcm@antgroup.com>
 */

#ifndef GEAXFRONTEND_AST_STMT_MANAGERSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_MANAGERSTATEMENT_H_

#include "geax-front-end/ast/stmt/Statement.h"

namespace geax {
namespace frontend {
class ManagerStatement : public Statement {
public:
    ManagerStatement() : Statement(AstNodeType::kManagerStatement) {}

    virtual ~ManagerStatement() = default;

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }
};  // class ManagerStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_MANAGERSTATEMENT_H_
