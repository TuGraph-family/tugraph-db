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
// Created by wt on 19-11-19.
//
#pragma once

#include "parser/clause.h"
#include "cypher/execution_plan/ops/op.h"
#include "cypher/arithmetic/arithmetic_expression.h"

namespace cypher {

class Unwind : public OpBase {
    std::vector<cypher::FieldData> list_;  // List which the unwind operation is performed on.
    ArithExprNode exp_;                    // Arithmetic expression (evaluated as an array).
    std::string resolved_name_;
    int rec_idx_ = 0;
    unsigned int list_idx_ = 0;  // Current list index.
    const SymbolTable *sym_tab_ = nullptr;
    static const unsigned int INDEX_NOT_SET = std::numeric_limits<unsigned int>::max();

    OpResult HandOff() {
        if (list_idx_ < list_.size()) {
            record->values[rec_idx_].constant = list_[list_idx_];
            list_idx_++;
            return OP_OK;
        }
        return OP_DEPLETED;
    }

 public:
    Unwind(const ArithExprNode &exp, const std::string &resolved_name, const SymbolTable *sym_tab)
        : OpBase(OpType::UNWIND, "Unwind"),
          exp_(exp),
          resolved_name_(resolved_name),
          sym_tab_(sym_tab) {
        modifies.emplace_back(resolved_name);
    }

    OpResult Initialize(RTContext *ctx) override {
        if (children.size() > 1) CYPHER_TODO();
        if (children.empty()) {
            // No child operation, list must be static.
            record = std::make_shared<Record>(sym_tab_->symbols.size(), sym_tab_, ctx->param_tab_);
            // Set parameter, use uniform runtime context
            for (auto &p : ctx->param_tab_) {
                auto it = record->symbol_table->symbols.find(p.first);
                if (it != record->symbol_table->symbols.end()) {
                    record->values[it->second.id].type = Entry::CONSTANT;
                    record->values[it->second.id].constant = p.second;
                }
            }
            auto list = exp_.Evaluate(ctx, *record);
            if (list.IsArray()) {
                list_ = *list.constant.array;
                list_idx_ = 0;
            } else {
                /* Attempting to use UNWIND on an expression that does not return a list --
                 *  such as UNWIND 5  will cause an error. The exception to this is when the
                 * expression returns null  this will reduce the number of rows to zero,
                 * causing it to cease its execution and return no results.  */
                if (!list.IsNull()) throw lgraph::CypherException("List expected in UNWIND");
                list_idx_ = INDEX_NOT_SET;
            }
        } else {
            list_idx_ = INDEX_NOT_SET;
            // List might depend on data provided by child operation.
            auto res = children[0]->Initialize(ctx);
            if (res != OP_OK) return res;
            record = children[0]->record;
        }
        /* Set record */
        auto it = sym_tab_->symbols.find(resolved_name_);
        if (it == sym_tab_->symbols.end()) CYPHER_TODO();
        rec_idx_ = it->second.id;
        record->values[rec_idx_].type = Entry::CONSTANT;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (HandOff() == OP_OK) return OP_OK;

        // No child operation to pull data from, we're done.
        if (children.empty()) return OP_DEPLETED;

        auto child = children[0];
        // Did we managed to get new data?
        if (child->Consume(ctx) == OP_OK) {
            auto list = exp_.Evaluate(ctx, *child->record);
            if (list.IsArray()) {
                list_ = *list.constant.array;
                list_idx_ = 0;
            } else {
                if (!list.IsNull()) throw lgraph::CypherException("List expected in UNWIND");
                list_idx_ = INDEX_NOT_SET;
            }
        }
        return HandOff();
    }

    OpResult ResetImpl(bool complete) override {
        list_idx_ = children.empty() ? 0 : INDEX_NOT_SET;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(exp_.ToString()).append(",").append(resolved_name_).append("]");
        return str;
    }

    std::string ResolvedName() const { return resolved_name_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};

}  // namespace cypher
