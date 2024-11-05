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

#ifndef GEAXFRONTEND_AST_STMT_PRIMITIVERESULTSTATEMENT_H_
#define GEAXFRONTEND_AST_STMT_PRIMITIVERESULTSTATEMENT_H_

#include <vector>

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/clause/Hint.h"
#include "geax-front-end/ast/clause/OrderByField.h"
#include "geax-front-end/ast/stmt/FilterStatement.h"
#include "geax-front-end/ast/expr/Param.h"

namespace geax {
namespace frontend {
class PrimitiveResultStatement : public AstNode {
public:
    PrimitiveResultStatement()
        : AstNode(AstNodeType::kPrimitiveResultStatement), isDistinct_(false) {}
    ~PrimitiveResultStatement() = default;

    void setDistinct(bool distinct) { isDistinct_ = distinct; }
    bool distinct() const { return isDistinct_; }

    void appendItem(std::string&& name, Expr* expr) { items_.emplace_back(std::move(name), expr, false); }
    void setItems(std::vector<std::tuple<std::string, Expr*, bool>>&& items) {
        items_ = std::move(items);
    }
    const std::vector<std::tuple<std::string, Expr*, bool>>& items() const { return items_; }
    void appendGroupKey(Expr* groupKey) { groupKeys_.emplace_back(groupKey); }
    void setGroupKeys(std::vector<Expr*>&& groupKeys) { groupKeys_ = std::move(groupKeys); }
    const std::vector<Expr*>& groupKeys() const { return groupKeys_; }

    void appendOrderBy(OrderByField* orderBy) { orderBys_.emplace_back(orderBy); }
    void setOrderBys(std::vector<OrderByField*>&& orderBys) { orderBys_ = std::move(orderBys); }
    const std::vector<OrderByField*>& orderBys() const { return orderBys_; }

    void setLimit(IntParam&& limit) { limit_ = std::move(limit); }
    const std::optional<IntParam>& limit() const { return limit_; }

    void setOffset(IntParam&& offset) { offset_ = std::move(offset); }
    const std::optional<IntParam>& offset() const { return offset_; }

    void appendHint(Hint* hint) { hints_.emplace_back(hint); }
    void setHints(std::vector<Hint*>&& hints) { hints_ = std::move(hints); }
    const std::vector<Hint*>& hints() const { return hints_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    bool isDistinct_;
    std::vector<std::tuple<std::string, Expr*, bool>> items_;
    std::vector<Expr*> groupKeys_;
    std::vector<OrderByField*> orderBys_;
    std::optional<IntParam> limit_;
    std::optional<IntParam> offset_;
    std::vector<Hint*> hints_;
};  // PrimitiveResultStatement

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_PRIMITIVERESULTSTATEMENT_H_
