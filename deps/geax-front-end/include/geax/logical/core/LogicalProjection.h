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
 */

#pragma once

#include <vector>
#include <string>
#include <tuple>

#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/ast/utils/AstUtil.h"
#include "geax/logical/LogicalOperator.h"

namespace geax::logical {
class LogicalProjection : public LogicalOperator {
 public:
    LogicalProjection(
        const std::vector<std::tuple<frontend::Expr*, std::string>>& items)
        : LogicalOperator(LogicalOperatorType::Projection), items_(items) {}
    ~LogicalProjection() = default;
    std::any accept(LogicalOperatorVisitor* visitor) override {
        return visitor->visit(std::static_pointer_cast<
                             std::remove_reference<decltype(*this)>::type>(
            shared_from_this()));
    }
    std::string toString() const override {
        std::string str = "LogicalProjection(items:[";
        for (auto& item : items_) {
            str += "(" + ToString(std::get<0>(item)) + ", " +
                   std::get<1>(item) + "), ";
        }
        if (!items_.empty()) {
            str.resize(str.size() - 2);
        }
        str += "])";
        return str;
    }

 private:
    std::vector<std::tuple<frontend::Expr*, std::string>> items_;
};
}  // namespace geax::logical
