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
// Created by wt on 6/14/18.
//
#pragma once

#include "parser/clause.h"
#include "arithmetic/arithmetic_expression.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class Project : public OpBase {
    friend class LazyProjectTopN;
    const SymbolTable &sym_tab_;
    std::vector<ArithExprNode> return_elements_;
    std::vector<std::string> return_alias_;
    bool single_response_;
    enum {
        Uninitialized,
        RefreshAfterPass,
        Resetted,
        Consuming,
    } state_;  // TODO(anyone) use OpBase state

    /* Construct arithmetic expressions from return clause. */
    void _BuildArithmeticExpressions(const parser::QueryPart *stmt) {
        const auto &return_body = stmt->return_clause ? std::get<1>(*stmt->return_clause)
                                                      : std::get<1>(*stmt->with_clause);
        const auto &return_items = std::get<0>(return_body);
        std::unordered_set<std::string> distinct_alias;
        for (auto &item : return_items) {
            auto &expr = std::get<0>(item);
            auto &var = std::get<1>(item);
            ArithExprNode ae(expr, sym_tab_);
            return_elements_.emplace_back(ae);
            auto alias = var.empty() ? expr.ToString(false) : var;
            if (distinct_alias.find(alias) != distinct_alias.end()) {
                throw lgraph::CypherException("Duplicate alias: " + alias);
            }
            distinct_alias.emplace(alias);
            return_alias_.emplace_back(alias);
            if (!var.empty()) modifies.emplace_back(var);
        }
    }

 public:
    Project(const parser::QueryPart *stmt, const SymbolTable *sym_tab)
        : OpBase(OpType::PROJECT, "Project"), sym_tab_(*sym_tab) {
        single_response_ = false;
        state_ = Uninitialized;
        _BuildArithmeticExpressions(stmt);
    }

    Project(const std::vector<std::tuple<ArithExprNode, std::string>> &items,
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

    OpResult Initialize(RTContext *ctx) override {
        if (!children.empty()) {
            auto &child = children[0];
            auto res = child->Initialize(ctx);
            if (res != OP_OK) return res;
        }
        /* projection */
        record = std::make_shared<Record>(return_elements_.size());
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        OpResult res = OP_OK;
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

    OpResult ResetImpl(bool complete) override {
        if (complete) {
            record = nullptr;
            single_response_ = false;
            state_ = Uninitialized;
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [");
        for (auto &i : return_alias_) {
            str.append(i).append(",");
        }
        if (!return_alias_.empty()) str.pop_back();
        str.append("]");
        return str;
    }

    const std::vector<ArithExprNode> &ReturnElements() const { return return_elements_; }

    const std::vector<std::string> &ReturnAlias() const { return return_alias_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
