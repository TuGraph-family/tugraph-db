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
// Created by wt on 20-2-18.
//
#pragma once

#include "parser/clause.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class OpRemove : public OpBase {
    const SymbolTable &sym_tab_;
    parser::Clause::TYPE_REMOVE remove_data_;
    PatternGraph *pattern_graph_ = nullptr;
    bool summary_ = false;

 public:
    OpRemove(const parser::QueryPart *stmt, PatternGraph *pattern_graph)
        : OpBase(OpType::REMOVE, "Remove"),
          sym_tab_(pattern_graph->symbol_table),
          pattern_graph_(pattern_graph) {
        CYPHER_THROW_ASSERT(stmt->remove_clause);
        remove_data_ = *stmt->remove_clause;
        for (auto &item : remove_data_) {
            if (item.type != parser::Expression::PROPERTY) CYPHER_TODO();
            modifies.emplace_back(item.Property().first.String());
        }
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(children.size() == 1);
        auto res = children[0]->Initialize(ctx);
        record = children[0]->record;
        state = StreamUnInitialized;
        return res;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (children[0]->Consume(ctx) != OP_OK) return OP_DEPLETED;
        for (auto &item : remove_data_) {
            if (item.type == parser::Expression::PROPERTY) {
                auto &var = item.Property().first.String();
                auto &key = item.Property().second;
                auto it = record->symbol_table->symbols.find(var);
                if (it == record->symbol_table->symbols.end()) CYPHER_TODO();
                auto &entry = record->values[it->second.id];
                if (entry.type != Entry::NODE) CYPHER_TODO();
                ctx->txn_->GetTxn()->SetVertexProperty(
                    entry.node->PullVid(), std::vector<std::string>{key},
                    std::vector<lgraph::FieldData>{lgraph::FieldData()});
            } else {
                CYPHER_TODO();
            }
        }
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [");
        for (auto &m : modifies) str.append(m).append(",");
        if (!modifies.empty()) str.pop_back();
        str.append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
