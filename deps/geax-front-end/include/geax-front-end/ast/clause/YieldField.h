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

#ifndef GEAXFRONTEND_AST_CLAUSE_YIELDFIELD_H_
#define GEAXFRONTEND_AST_CLAUSE_YIELDFIELD_H_

#include "geax-front-end/ast/AstNode.h"
#include <vector>
#include <tuple>

namespace geax {
namespace frontend {

class YieldField : public AstNode {
public:
    YieldField() : AstNode(AstNodeType::kYieldField) {}
    ~YieldField() = default;

    void appendItem(std::string&& name, std::string&& alias) {
        items_.emplace_back(std::move(name), std::move(alias));
    }
    void setItems(std::vector<std::tuple<std::string, std::string>>&& items) {
        items_ = std::move(items);
    }
    const std::vector<std::tuple<std::string, std::string>>& items() const { return items_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::vector<std::tuple<std::string, std::string>> items_;
};  // class YieldField

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_CLAUSE_YIELDFIELD_H_
