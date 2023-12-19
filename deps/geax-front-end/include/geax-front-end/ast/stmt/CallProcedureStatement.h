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

#ifndef GEAXFRONTEND_AST_STMT_CALLPROCEDURESTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_CALLPROCEDURESTATEMENT_H_

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/stmt/ProcedureCall.h"

namespace geax {
namespace frontend {

class CallProcedureStatement : public AstNode {
public:
    CallProcedureStatement() : AstNode(AstNodeType::kCallProcedureStatement) {}
    ~CallProcedureStatement() = default;

    void setIsOption(bool isOption) { isOption_ = isOption; }
    bool isOption() const { return isOption_; }

    void setProcedureCall(ProcedureCall* procedureCall) { procedureCall_ = procedureCall; }
    ProcedureCall* procedureCall() const { return procedureCall_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool isOption_{false};
    ProcedureCall* procedureCall_;
};  // class CallProcedureStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_CALLPROCEDURESTATEMENT_H_
