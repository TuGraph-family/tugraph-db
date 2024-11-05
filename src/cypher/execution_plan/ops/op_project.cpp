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
// Created by wt on 6/18/19.
//

#include "cypher/execution_plan/ops/op_project.h"

namespace cypher {

Project::Project(const std::vector<std::tuple<ArithExprNode, std::string>> &items,
        const SymbolTable *sym_tab)
        : OpBase(OpType::PROJECT, "Project"), sym_tab_(*sym_tab) {
    single_response_ = false;
    state_ = Uninitialized;
    for (const auto &[expr, var] : items) {
        return_elements_.emplace_back(expr);
        return_alias_.emplace_back(var.empty() ? expr.ToString() : var);
        if (!var.empty()) modifies.emplace_back(var);
    }
}

OpBase::OpResult Project::RealConsume(RTContext *ctx) {
    OpBase::OpResult res = OP_OK;
    std::shared_ptr<Record> r;
    if (!children.empty()) {
        CYPHER_THROW_ASSERT(children.size() == 1);
        res = children[0]->Consume(ctx);
        r = children[0]->record;
    } else {
        // QUERY: RETURN 1+2
        // Return a single record followed by NULL
        // on the second call.
        if (single_response_) return OP_DEPLETED;
        single_response_ = true;
        // QUERY: RETURN [x IN range(0,10) | x^3] AS result
        // List comprehension is divided into three parts:
        // x (reference), range(0,10) (range), and x^3 (expression).
        // When calculating x^3, we need to get the value of x.
        // The symbol of x is stored in the symbol table,
        // and its value is stored in the Project op.
        // Therefore, we need to change r from an empty record to a record
        // of the same size as the symbol table.
        r = std::make_shared<Record>(sym_tab_.symbols.size());
    }
    if (res != OP_OK) return res;
    int re_idx = 0;
    for (auto &re : return_elements_) {
        auto v = re.Evaluate(ctx, *r);
        record->values[re_idx++] = v;
        // TODO(anyone) handle alias
    }
    return OP_OK;
}

}