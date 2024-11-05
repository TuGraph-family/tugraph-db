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

#include "cypher/execution_plan/ops/op_standalone_call.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/procedure/procedure.h"
#include "common/logger.h"
namespace cypher {

OpBase::OpResult GqlStandaloneCall::Initialize(RTContext *ctx) {
    record = std::make_shared<Record>(yield_.value()->items().size(), &symbol_table_);
    return OP_OK;
}

OpBase::OpResult GqlStandaloneCall::HandOff(
    RTContext *ctx, std::shared_ptr<Record> &r) {
    if (buffer_.empty()) return OP_DEPLETED;
    auto &rec = buffer_.back();
    assert(r->values.size() == rec.size());
    for (size_t i = 0; i < rec.size(); i++) {
        switch (rec[i].type) {
            case ProcedureResultType::Value:{
                record->values[i] = cypher::Entry(rec[i].AsValue());
                break;
            }
            default: {
                CYPHER_TODO();
            }
        }
    }
    buffer_.pop_back();
    return OP_OK;
}

cypher::OpBase::OpResult cypher::GqlStandaloneCall::RealConsume(RTContext *ctx) {
    if (HandOff(ctx, record) == OP_OK) return OP_OK;
    if (state == StreamDepleted) return OP_DEPLETED;
    auto p = global_ptable.GetProcedure(func_name_);
    if (!p) {
        THROW_CODE(CypherException, "unregistered function: {}", func_name_);
    }
    std::vector<std::string> _yield_items;
    if (yield_.has_value()) {
        for (auto &pair : yield_.value()->items()) {
            _yield_items.emplace_back(std::get<0>(pair));
        }
    }
    std::vector<Entry> parameters;
    parameters.reserve(args_.size());
    for (auto expr : args_) {
        ArithExprNode node(expr, symbol_table_);
        parameters.emplace_back(node.Evaluate(ctx, *record));
    }
    p->function(ctx, nullptr, parameters, _yield_items, &buffer_);
    std::reverse(buffer_.begin(), buffer_.end());
    state = StreamDepleted;
    return HandOff(ctx, record);
}

}
