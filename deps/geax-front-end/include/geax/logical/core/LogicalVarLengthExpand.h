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
#include "geax-front-end/ast/utils/AstUtil.h"
#include "geax/logical/LogicalOperator.h"
#include "geax/utils/ToStringUtil.h"

namespace geax::logical {
class LogicalVarLengthExpand : public LogicalOperator {
 public:
    LogicalVarLengthExpand(
        const std::string& src_ref, const std::string& binding_dst,
        const std::string& binding_edges, const std::string& binding_edge,
        frontend::EdgeDirection dir, frontend::LabelTree* labels,
        std::vector<frontend::Expr*> filters, int64_t min_hop, int64_t max_hop)
        : LogicalOperator(LogicalOperatorType::VarLengthExpand),
          src_ref_(src_ref),
          binding_dst_(binding_dst),
          binding_edges_(binding_edges),
          binding_edge_(binding_edge),
          dir_(dir),
          labels_(labels),
          filters_(filters),
          min_hop_(min_hop),
          max_hop_(max_hop) {}
    ~LogicalVarLengthExpand() = default;
    std::any accept(LogicalOperatorVisitor* visitor) override {
        return visitor->visit(std::static_pointer_cast<
                             std::remove_reference<decltype(*this)>::type>(
            shared_from_this()));
    }
    std::string toString() const override {
        return "LogicalVarLengthExpand(src_ref:" + src_ref_ +
               ", binding_dst:" + binding_dst_ +
               ", binding_edges:" + binding_edges_ +
               ", binding_edge:" + binding_edge_ +
               ", dir:" + frontend::ToString(dir_) +
               ", labels:" + ToString(labels_) +
               ", filters:" + utils::ToString(filters_) +
               ", min_hop:" + utils::ToString(min_hop_) +
               ", max_hop:" + utils::ToString(max_hop_) + ")";
    }

 private:
    std::string src_ref_;
    std::string binding_dst_;
    std::string binding_edges_;
    std::string binding_edge_;
    frontend::EdgeDirection dir_;
    frontend::LabelTree* labels_;
    std::vector<frontend::Expr*> filters_;
    int64_t min_hop_;
    int64_t max_hop_;
};
}  // namespace geax::logical
