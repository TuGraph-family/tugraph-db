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

#ifndef GEAXFRONTEND_AST_STMT_FOCUSEDRESULTSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_FOCUSEDRESULTSTATEMENT_H_

#include "geax-front-end/ast/stmt/LinearQueryStatement.h"
#include "geax-front-end/ast/stmt/PrimitiveResultStatement.h"

namespace geax {
namespace frontend {

class FocusedResultStatement : public LinearQueryStatement {
public:
    FocusedResultStatement()
        : LinearQueryStatement(AstNodeType::kFocusedResultStatement),
          resultStatement_(nullptr) {}
    ~FocusedResultStatement() = default;

    void setGraphRef(std::string&& graphRef) { graphRef_ = std::move(graphRef); }
    const std::string& graphRef() const { return graphRef_; }

    void setResultStatement(PrimitiveResultStatement* resultStatement) {
        resultStatement_ = resultStatement;
    }
    PrimitiveResultStatement* resultStatement() const { return resultStatement_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::string graphRef_;
    PrimitiveResultStatement* resultStatement_;
};  // class FocusedResultStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_FOCUSEDRESULTSTATEMENT_H_
