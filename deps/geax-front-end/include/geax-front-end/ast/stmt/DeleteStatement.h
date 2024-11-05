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

#ifndef GEAXFRONTEND_AST_STMT_DELETESTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_DELETESTATEMENT_H_

#include "geax-front-end/ast/stmt/PrimitiveDataModifyStatement.h"

namespace geax {
namespace frontend {

class DeleteStatement : public PrimitiveDataModifyStatement {
public:
    DeleteStatement() : PrimitiveDataModifyStatement(AstNodeType::kDeleteStatement) {}
    ~DeleteStatement() = default;

    void appendItem(std::string&& item) { items_.emplace_back(std::move(item)); }
    void setItems(std::vector<std::string>&& items) { items_ = std::move(items); }
    const std::vector<std::string>& items() const { return items_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<std::string> items_;
};  // class DeleteStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_DELETESTATEMENT_H_
