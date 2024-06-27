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
 */

#ifndef GEAXFRONTEND_AST_STMT_INQUERYPROCEDURECALL_H_
#define GEAXFRONTEND_AST_STMT_INQUERYPROCEDURECALL_H_

#include "geax-front-end/ast/stmt/ProcedureCall.h"
#include "geax-front-end/ast/expr/Param.h"
#include "geax-front-end/ast/expr/Expr.h"
#include "geax-front-end/ast/clause/YieldField.h"

namespace geax {
namespace frontend {

class InQueryProcedureCall : public ProcedureCall {
public:
    InQueryProcedureCall() : ProcedureCall(AstNodeType::kInQueryProcedureCall) {}
    ~InQueryProcedureCall() = default;

    void setName(StringParam&& name) { name_ = std::move(name); }
    const StringParam name() const { return name_; }

    void appendArg(Expr* arg) { args_.emplace_back(arg); }
    void setArgs(std::vector<Expr*>&& args) { args_ = std::move(args); }
    const std::vector<Expr*>& args() const { return args_; }

    void setYield(YieldField* yield) { yield_ = yield; }
    const std::optional<YieldField*>& yield() const { return yield_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    StringParam name_;
    std::vector<Expr*> args_;
    std::optional<YieldField*> yield_;
};  // class NamedProcedureCall

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_INQUERYPROCEDURECALL_H_
