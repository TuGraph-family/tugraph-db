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

#ifndef GEAXFRONTEND_AST_STMT_EXPLAINACTIVITY_H_
#define GEAXFRONTEND_AST_STMT_EXPLAINACTIVITY_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/stmt/ProcedureBody.h"

namespace geax {
namespace frontend {

/**
 * This is explain or profile a statement.
 */
class ExplainActivity : public AstNode {
public:
    ExplainActivity() : AstNode(AstNodeType::kExplainActivity), procedureBody_(nullptr) {}
    ~ExplainActivity() = default;

    void setIsProfile(bool isProfile) { isProfile_ = isProfile; }
    bool isProfile() const { return isProfile_; }

    void setFormat(std::string&& format) { format_ = std::move(format); }
    const std::optional<std::string> format() const { return format_; }

    void setProcedureBody(ProcedureBody* procedureBody) { procedureBody_ = procedureBody; }
    ProcedureBody* procedureBody() const { return procedureBody_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool isProfile_;
    std::optional<std::string> format_;
    ProcedureBody* procedureBody_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_EXPLAINACTIVITY_H_
