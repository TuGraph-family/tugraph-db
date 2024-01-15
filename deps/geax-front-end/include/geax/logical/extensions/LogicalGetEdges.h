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

#include "geax-front-end/ast/Ast.h"
#include "geax/logical/LogicalOperator.h"

namespace geax::logical {
class LogicalGetEdges : public LogicalOperator {
 public:
    LogicalGetEdges(const std::string& src_ref, const std::string& binding_edge,
                    frontend::EdgeDirection dir, frontend::LabelTree* labels,
                    const std::vector<frontend::Expr*>& filters)
        : LogicalOperator(LogicalOperatorType::GetEdges),
          src_ref_(src_ref),
          binding_edge_(binding_edge),
          dir_(dir),
          labels_(labels),
          filters_(filters) {}
    ~LogicalGetEdges() = default;
    std::any accept(LogicalOperatorVisitor* visitor) override {
        return visitor->visit(std::static_pointer_cast<
                             std::remove_reference<decltype(*this)>::type>(
            shared_from_this()));
    }

 private:
    std::string src_ref_;
    std::string binding_edge_;
    frontend::EdgeDirection dir_;
    frontend::LabelTree* labels_;
    std::vector<frontend::Expr*> filters_;
};
}  // namespace geax::logical
