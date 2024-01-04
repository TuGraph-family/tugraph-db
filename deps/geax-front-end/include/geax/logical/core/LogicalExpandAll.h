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
#include <vector>

#include "geax-front-end/ast/Ast.h"
#include "geax-front-end/ast/utils/AstUtil.h"
#include "geax/logical/LogicalOperator.h"
#include "geax/utils/ToStringUtil.h"

namespace geax::logical {
class LogicalExpandAll : public LogicalOperator {
 public:
    LogicalExpandAll(const std::string& src_ref, const std::string& binding_dst,
                     const std::string& binding_edge,
                     frontend::EdgeDirection dir, frontend::LabelTree* labels,
                     std::vector<frontend::Expr*> filters)
        : LogicalOperator(LogicalOperatorType::ExpandAll),
          src_ref_(src_ref),
          binding_dst_(binding_dst),
          binding_edge_(binding_edge),
          dir_(dir),
          labels_(labels),
          filters_(filters) {}
    ~LogicalExpandAll() = default;
    std::any accept(LogicalOperatorVisitor* visitor) override {
        return visitor->visit(std::static_pointer_cast<
                             std::remove_reference<decltype(*this)>::type>(
            shared_from_this()));
    }
    std::string toString() const override {
        std::string str = "LogicalExpandAll(src_ref:" + src_ref_ +
                          ", binding_dst:" + binding_dst_ +
                          ", binding_edge:" + binding_edge_ +
                          ", dir:" + ToString(dir_) +
                          ", labels:" + ToString(labels_) +
                          ", filters:" + utils::ToString(filters_) + ")";
        return str;
    }

 private:
    std::string src_ref_;
    std::string binding_dst_;
    std::string binding_edge_;
    frontend::EdgeDirection dir_;
    frontend::LabelTree* labels_;
    std::vector<frontend::Expr*> filters_;
};
}  // namespace geax::logical
