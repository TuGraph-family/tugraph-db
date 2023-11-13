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

#ifndef GEAXFRONTEND_AST_STMT_PROCEDUREBODY_H_
#define GEAXFRONTEND_AST_STMT_PROCEDUREBODY_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/BindingVar.h"
#include "geax-front-end/ast/clause/SchemaRef.h"
#include "geax-front-end/ast/stmt/StatementWithYield.h"

namespace geax {
namespace frontend {

/**
 * This is the root of an AST.
 */
class ProcedureBody : public AstNode {
public:
    ProcedureBody() : AstNode(AstNodeType::kProcedureBody) {}
    ~ProcedureBody() = default;

    void appendStatement(StatementWithYield* statement) { statements_.emplace_back(statement); }
    void setStatements(std::vector<StatementWithYield*>&& statements) {
        statements_ = std::move(statements);
    }
    const std::vector<StatementWithYield*>& statements() const { return statements_; }

    void setSchemaRef(SchemaRef* schemaRef) { schemaRef_ = schemaRef; }
    const std::optional<SchemaRef*>& schemaRef() const { return schemaRef_; }

    void appendBindingVar(BindingVar* binding) { bindingVars_.emplace_back(binding); }
    void setBindingVars(std::vector<BindingVar*>&& bindingVars) {
        bindingVars_ = std::move(bindingVars);
    }
    const std::vector<BindingVar*>& bindingVars() const { return bindingVars_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::optional<SchemaRef*> schemaRef_;
    std::vector<BindingVar*> bindingVars_;
    std::vector<StatementWithYield*> statements_;
};  // class ProcedureBody

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_PROCEDUREBODY_H_
