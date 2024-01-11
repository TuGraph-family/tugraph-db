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

#include <string>

#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/ast/utils/AstUtil.h"
#include "geax/logical/LogicalOperator.h"
#include "geax/utils/ToStringUtil.h"

namespace geax::logical {

class LogicalNodeSeek : public LogicalOperator {
 public:
    LogicalNodeSeek(const std::string& binding_var, const std::string& label,
                    const std::string& field, frontend::Expr* value)
        : LogicalOperator(LogicalOperatorType::NodeSeek),
          binding_var_(binding_var),
          label_(label),
          field_(field),
          value_(value) {}
    ~LogicalNodeSeek() = default;
    std::any accept(LogicalOperatorVisitor* visitor) override {
        return visitor->visit(std::static_pointer_cast<
                             std::remove_reference<decltype(*this)>::type>(
            shared_from_this()));
    }
    std::string toString() const override {
        return "LogicalNodeSeek(binding_var:" + binding_var_ +
               ", label:" + label_ + ", field:" + field_ +
               ", value:" + ToString(value_) + ")";
    }

 private:
    std::string binding_var_;
    std::string label_;
    std::string field_;
    frontend::Expr* value_;
};

}  // namespace geax::logical
