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

#ifndef GEAXFRONTEND_AST_STMT_KILLSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_KILLSTATEMENT_H_

#include "geax-front-end/ast/stmt/Statement.h"
namespace geax {
namespace frontend {
class KillStatement : public Statement {
public:
    KillStatement() : Statement(AstNodeType::kKillStatement) {}
    virtual ~KillStatement() = default;

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

    void setId(uint64_t id) { id_ = id; }

    bool id() const { return id_; }

    void setQuery(bool flag) { query_ = flag; }

    bool isQuery() const { return query_; }

private:
    uint64_t id_;
    bool query_{true};
};  // class KillStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_KILLSTATEMENT_H_
