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
#include "geax/logical/LogicalOperator.h"

namespace geax::logical {
class LogicalExtractEdge : public LogicalOperator {
 public:
    LogicalExtractEdge(const std::string& edge_ref,
                       const std::string& binding_dst)
        : LogicalOperator(LogicalOperatorType::ExtractEdge),
          edge_ref_(edge_ref),
          binding_dst_(binding_dst) {}
    ~LogicalExtractEdge() = default;
    std::any accept(LogicalOperatorVisitor* visitor) override {
        return visitor->visit(std::static_pointer_cast<
                             std::remove_reference<decltype(*this)>::type>(
            shared_from_this()));
    }

 private:
    std::string edge_ref_;
    std::string binding_dst_;
};

}  // namespace geax::logical
