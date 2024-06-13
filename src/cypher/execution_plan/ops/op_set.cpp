﻿/**
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
// Created by dcy on 19-8-22.
//

#include "cypher/execution_plan/ops/op_set.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {

void OpSet::ExtractProperties(RTContext *ctx, const parser::Expression &p,
                       std::vector<lgraph::FieldData> &values) {
    ArithExprNode ae(p, *record->symbol_table);
    auto val = ae.Evaluate(ctx, *record);
    if (!val.IsScalar()) CYPHER_TODO();
    values.emplace_back(val.constant.scalar);
}

}