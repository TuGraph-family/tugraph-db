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
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#ifndef GEAXFRONTEND_AST_STMT_BINDINGTABLE_H_
#define GEAXFRONTEND_AST_STMT_BINDINGTABLE_H_

#include "geax-front-end/ast/stmt/BindingDefinition.h"
#include "geax-front-end/ast/stmt/BindingTableExpr.h"

namespace geax {
namespace frontend {

class BindingTable : public BindingDefinition {
public:
    BindingTable() : BindingDefinition(AstNodeType::kBindingTable), query_(nullptr) {}
    ~BindingTable() = default;

    void setVal(std::string&& varName) { varName_ = std::move(varName); }
    const std::string& varName() const { return varName_; }

    void appendType(std::string&& fieldname, std::string&& filedtype) {
        types_.emplace_back(std::move(fieldname), std::move(filedtype));
    }
    void setTypes(std::vector<std::tuple<std::string, std::string>>&& types) {
        types_ = std::move(types);
    }
    const std::vector<std::tuple<std::string, std::string>>& types() const { return types_; }

    void setQuery(BindingTableExpr* query) { query_ = query; }
    BindingTableExpr* query() { return query_; }

    std::any accept(AstNodeVisitor& visitor) override { return visitor.visit(this); }

private:
    std::string varName_;
    std::vector<std::tuple<std::string, std::string>> types_;
    BindingTableExpr* query_;
};  // class BindingTable

}  // namespace frontend
}  // namespace geax

#endif  // GEAXFRONTEND_AST_STMT_BINDINGTABLE_H_
