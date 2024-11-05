/**
* Copyright 2024 AntGroup CO., Ltd.
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

#include "cypher/execution_plan/ops/op_inquery_call.h"

namespace cypher {

OpBase::OpResult GqlInQueryCall::HandOff(RTContext *ctx, std::shared_ptr<Record> &r) {
    if (buffer_.empty()) return OP_DEPLETED;
    auto &rec = buffer_.back();
    for (int i = 0; i < (int)yield_idx_.size(); i++) {
        if (rec[i].type == ProcedureResultType::Node) {
            r->values[yield_idx_[i]].type = Entry::NODE;
            r->values[yield_idx_[i]].node->vertex_ = rec[i].AsNode();
        } else {
            r->values[yield_idx_[i]] = cypher::Entry(rec[i].AsValue());
        }
    }
    buffer_.pop_back();
    return OP_OK;
}

OpBase::OpResult GqlInQueryCall::RealConsume(RTContext *ctx) {
    if (children.empty()) {
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
            ArithExprNode node(expr, pattern_graph_->symbol_table);
            parameters.emplace_back(node.Evaluate(ctx, *record.get()));
        }
        p->function(ctx, record.get(), parameters, _yield_items, &buffer_);
        std::reverse(buffer_.begin(), buffer_.end());
        state = StreamDepleted;
        return HandOff(ctx, record);
    } else {
        if (HandOff(ctx, record) == OP_OK) return OP_OK;
        auto child = children[0];
        while (child->Consume(ctx) == OP_OK) {
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
                ArithExprNode node(expr, pattern_graph_->symbol_table);
                parameters.emplace_back(node.Evaluate(ctx, *record.get()));
            }
            p->function(ctx, record.get(), parameters, _yield_items, &buffer_);
            std::reverse(buffer_.begin(), buffer_.end());
            if (HandOff(ctx, record) == OP_OK) return OP_OK;
        }
        return OP_DEPLETED;
    }

    return OP_DEPLETED;
}
}
