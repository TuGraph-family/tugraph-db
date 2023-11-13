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

#ifndef GEAXFRONTEND_AST_STMT_SETSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_SETSTATEMENT_H_

#include "geax-front-end/ast/clause/SetItem.h"
#include "geax-front-end/ast/stmt/PrimitiveDataModifyStatement.h"

namespace geax {
namespace frontend {

class SetStatement : public PrimitiveDataModifyStatement {
public:
    SetStatement() : PrimitiveDataModifyStatement(AstNodeType::kSetStatement) {}
    ~SetStatement() = default;

    void appendItem(SetItem* item) { items_.emplace_back(item); }
    void setItems(std::vector<SetItem*>&& items) { items_ = std::move(items); }
    const std::vector<SetItem*>& items() const { return items_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<SetItem*> items_;
};  // class SetStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_SETSTATEMENT_H_
