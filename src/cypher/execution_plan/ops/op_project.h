/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 6/14/18.
//
#pragma once

#include "parser/clause.h"
#include "arithmetic/arithmetic_expression.h"
#include "op.h"

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
    } state_;  // TODO: use OpBase state // NOLINT

    /* Construct arithmetic expressions from return clause. */
    void _BuildArithmeticExpressions(const parser::QueryPart *stmt) {
        const auto &return_body = stmt->return_clause ? std::get<1>(*stmt->return_clause)
                                                      : std::get<1>(*stmt->with_clause);
        const auto &return_items = std::get<0>(return_body);
        for (auto &item : return_items) {
            auto &expr = std::get<0>(item);
            auto &var = std::get<1>(item);
            ArithExprNode ae(expr, sym_tab_);
            return_elements_.emplace_back(ae);
            return_alias_.emplace_back(var.empty() ? expr.ToString(false) : var);
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
            r = std::make_shared<Record>();  // fake empty record.
        }
        if (res != OP_OK) return res;
        int re_idx = 0;
        for (auto &re : return_elements_) {
            auto v = re.Evaluate(ctx, *r);
            record->values[re_idx++] = v;
            // todo: handle alias
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
