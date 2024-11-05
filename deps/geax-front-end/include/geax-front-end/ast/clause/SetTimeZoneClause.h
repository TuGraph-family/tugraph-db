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

#ifndef GEAXFRONTEND_AST_CLAUSE_SETTIMEZONECLAUSE_H_
#define GEAXFRONTEND_AST_CLAUSE_SETTIMEZONECLAUSE_H_

#include "geax-front-end/ast/clause/SessionSetCommand.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class SetTimeZoneClause : public SessionSetCommand {
public:
    SetTimeZoneClause() : SessionSetCommand(AstNodeType::kSetTimeZone), initExpr_(nullptr) {}
    ~SetTimeZoneClause() = default;

    void setInitExpr(Expr* initExpr) { initExpr_ = initExpr; }
    Expr* initExpr() const { return initExpr_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    Expr* initExpr_;
};

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_SETTIMEZONECLAUSE_H_
