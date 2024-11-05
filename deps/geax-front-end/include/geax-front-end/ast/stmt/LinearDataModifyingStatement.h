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

#ifndef GEAXFRONTEND_AST_STMT_LINEARDATAMODIFYINGSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_LINEARDATAMODIFYINGSTATEMENT_H_

#include "geax-front-end/ast/stmt/Statement.h"
#include "geax-front-end/ast/stmt/PrimitiveDataModifyStatement.h"
#include "geax-front-end/ast/stmt/PrimitiveResultStatement.h"
#include "geax-front-end/ast/stmt/SimpleQueryStatement.h"

namespace geax {
namespace frontend {

class LinearDataModifyingStatement : public Statement {
public:
    LinearDataModifyingStatement() : Statement(AstNodeType::kDataModifyStatement) {}
    ~LinearDataModifyingStatement() = default;

    void setUseGraph(std::string&& useGraph) { useGraph_ = std::move(useGraph); }
    const std::optional<std::string>& useGraph() const { return useGraph_; }

    void appendModifyStatement(PrimitiveDataModifyStatement* modifyStatement) {
        modifyStatements_.emplace_back(modifyStatement);
    }
    void setModifyStatement(std::vector<PrimitiveDataModifyStatement*>&& modifyStatements) {
        modifyStatements_ = std::move(modifyStatements);
    }
    const std::vector<PrimitiveDataModifyStatement*>& modifyStatements() const {
        return modifyStatements_;
    }

    void appendQueryStatement(SimpleQueryStatement* queryStatement) {
        queryStatements_.emplace_back(queryStatement);
    }
    void setQueryStatements(std::vector<SimpleQueryStatement*>&& queryStatements) {
        queryStatements_ = std::move(queryStatements);
    }
    const std::vector<SimpleQueryStatement*>& queryStatements() const { return queryStatements_; }

    void setResultStatement(PrimitiveResultStatement* resultStatement) {
        resultStatement_ = resultStatement;
    }
    const std::optional<PrimitiveResultStatement*>& resultStatement() const {
        return resultStatement_;
    }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::optional<std::string> useGraph_;
    std::vector<SimpleQueryStatement*> queryStatements_;
    std::vector<PrimitiveDataModifyStatement*> modifyStatements_;
    std::optional<PrimitiveResultStatement*> resultStatement_;
};  // class LinearDataModifyingStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_LINEARDATAMODIFYINGSTATEMENT_H_
