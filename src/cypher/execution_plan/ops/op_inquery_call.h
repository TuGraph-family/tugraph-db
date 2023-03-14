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
// Created by wt on 19-1-21.
//
#pragma once

#include <vector>
#include "parser/clause.h"
#include "procedure/procedure.h"
#include "op.h"

namespace cypher {
class InQueryCall : public OpBase {
    const PatternGraph *pattern_ = nullptr;
    std::string func_name_;
    Procedure *procedure_ = nullptr;
    parser::Clause::TYPE_CALL call_clause_;
    std::vector<Record> buffer_;
    std::vector<size_t> yield_idx_;

    void SetFunc(const std::string &name) {
        auto p = global_ptable.GetProcedure(name);
        if (!p) {
            throw lgraph::EvaluationException("unregistered standalone function: " + name);
        }
        procedure_ = p;
        func_name_ = name;
    }

    OpResult HandOff(std::shared_ptr<Record> &r) {
        if (buffer_.empty()) return OP_DEPLETED;
        auto &rec = buffer_.back();
        for (int i = 0; i < (int)yield_idx_.size(); i++) {
            r->values[yield_idx_[i]] = rec.values[i];
        }
        buffer_.pop_back();
        return OP_OK;
    }

 public:
    InQueryCall(const PatternGraph *pattern_graph, const parser::QueryPart *stmt)
        : OpBase(OpType::INQUERY_CALL, "In Query Call"),
          pattern_(pattern_graph),
          call_clause_(*stmt->iq_call_clause) {
        state = StreamUnInitialized;
        modifies = std::get<2>(*stmt->iq_call_clause);
    }

    OpResult Initialize(RTContext *ctx) override {
        auto &sym_tab = pattern_->symbol_table;
        if (children.empty()) {
            record = std::make_shared<Record>(sym_tab.symbols.size(), &sym_tab);
        } else {
            CYPHER_THROW_ASSERT(children.size() == 1);
            if (children[0]->Initialize(ctx) != OP_OK) return OP_ERR;
            record = children[0]->record;
        }
        auto &yield_items = std::get<2>(call_clause_);
        for (auto &y : yield_items) {
            auto it = sym_tab.symbols.find(y);
            if (it == sym_tab.symbols.end()) CYPHER_TODO();
            yield_idx_.emplace_back(it->second.id);
        }
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (children.empty()) {
            if (HandOff(record) == OP_OK) return OP_OK;
            if (state == StreamDepleted) return OP_DEPLETED;
            auto &call = call_clause_;
            auto &procedure_name = std::get<0>(call);
            auto &parameters = std::get<1>(call);
            auto &yield_items = std::get<2>(call);
            SetFunc(procedure_name);
            procedure_->function(ctx, record.get(), parameters, yield_items, &buffer_);
            std::reverse(buffer_.begin(), buffer_.end());
            state = StreamDepleted;
            return HandOff(record);
        } else {
            if (HandOff(record) == OP_OK) return OP_OK;
            auto child = children[0];
            while (child->Consume(ctx) == OP_OK) {
                auto &call = call_clause_;
                auto &procedure_name = std::get<0>(call);
                auto &parameters = std::get<1>(call);
                auto &yield_items = std::get<2>(call);
                SetFunc(procedure_name);
                /* If the procedure need a separate txn while the other operations
                 * working in the previous one, throw exception.
                 * e.g.
                 *   MATCH (n) CALL db.addLabel(n.name)  */
                if (procedure_->separate_txn) CYPHER_TODO();
                procedure_->function(ctx, record.get(), parameters, yield_items, &buffer_);
                std::reverse(buffer_.begin(), buffer_.end());
                if (HandOff(record) == OP_OK) return OP_OK;
            }
            return OP_DEPLETED;
        }
    }

    OpResult ResetImpl(bool complete = false) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(func_name_).append("]");
        return str;
    }

    const parser::Clause::TYPE_CALL& CallClause() const { return call_clause_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()

};
}  // namespace cypher
