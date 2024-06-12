/**
 * Copyright 2023 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") {}
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

#include "cypher/utils/ast_node_visitor_impl.h"

namespace cypher {

class AstAggExprDetector : public cypher::AstNodeVisitorImpl {
 public:
    AstAggExprDetector() = delete;

    explicit AstAggExprDetector(geax::frontend::Expr* expr) : expr_(expr) {}

    virtual ~AstAggExprDetector() = default;

    std::vector<geax::frontend::Expr*>& AggExprs();

    /**
     *  Determine if there are any invalid aggregate functions.
     *  Return true if there are none.
     */
    bool Validate();

    /**
     *  Return whether there are valid aggregate functions
     *  under the condition that Validate returns true.
     */
    bool HasValidAggFunc();

    static std::vector<geax::frontend::Expr*> GetAggExprs(geax::frontend::Expr* expr);

 private:
    std::any visit(geax::frontend::AggFunc* node) override;
    std::any visit(geax::frontend::BAggFunc* node) override;
    std::any visit(geax::frontend::GetField* node) override;
    bool nested_agg_func_ = false;
    size_t in_agg_func_ = 0;
    bool outside_var_ = false;
    geax::frontend::Expr* expr_;
    std::vector<geax::frontend::Expr*> agg_exprs_;
};

}  // namespace cypher
