/**
 * Copyright 2022 AntGroup CO., Ltd.
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

//
// Created by wt on 18-7-27.
//

#include "cypher/execution_plan/ops/op_create.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {
void OpCreate::ExtractProperties(RTContext *ctx, const parser::TUP_PROPERTIES &properties,
                       VEC_STR &fields, std::vector<lgraph::FieldData> &values) {
    using namespace parser;
    Expression map_literal = std::get<0>(properties);
    CYPHER_THROW_ASSERT(map_literal.type == Expression::NA ||
                        map_literal.type == Expression::MAP);
    if (map_literal.type != Expression::MAP) return;
    for (auto &prop : map_literal.Map()) {
        fields.emplace_back(prop.first);
        Expression p;
        if (prop.second.type == Expression::LIST) {
            p = prop.second.List().at(0);
        } else if (prop.second.type == Expression::MAP) {
            CYPHER_TODO();
        } else {
            p = prop.second;
        }
        ArithExprNode ae(p, *record->symbol_table);
        auto value = ae.Evaluate(ctx, *record);
        CYPHER_THROW_ASSERT(value.IsScalar());
        values.emplace_back(value.constant.scalar);
    }
}
}  // namespace cypher
