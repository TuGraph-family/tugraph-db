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

#ifndef GEAXFRONTEND_AST_CLAUSE_PROPSTRUCT_H_
#define GEAXFRONTEND_AST_CLAUSE_PROPSTRUCT_H_

#include <vector>

#include "geax-front-end/ast/clause/ElementPredicate.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace geax {
namespace frontend {

class PropStruct : public ElementPredicate {
public:
    PropStruct() : ElementPredicate(AstNodeType::kPropStruct) {}
    ~PropStruct() = default;

    void appendProperty(std::string&& key, Expr* value) {
        properties_.emplace_back(std::move(key), value);
    }
    void setProperties(std::vector<std::tuple<std::string, Expr*>>&& properties) {
        properties_ = std::move(properties);
    }
    const std::vector<std::tuple<std::string, Expr*>>& properties() const { return properties_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<std::tuple<std::string, Expr*>> properties_;
};  // class PropStruct

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_PROPSTRUCT_H_
