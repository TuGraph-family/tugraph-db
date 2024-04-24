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
// Created by wt on 22-4-1.
//
#pragma once

#include "execution_plan/ops/op_argument.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class ImmediateArgument : public OpBase {
    const Record *input_record_ = nullptr;
    PatternGraph *pattern_graph_ = nullptr;
    std::vector<ArgIndex> args_;

 public:
    explicit ImmediateArgument(PatternGraph *pattern)
        : OpBase(OpType::ARGUMENT, "Immediate Argument"), pattern_graph_(pattern) {
        auto sym_tab = &pattern_graph_->symbol_table;
        for (auto &s : sym_tab->symbols) {
            if (s.second.scope == SymbolNode::ARGUMENT) {
                // <alias, rec_idx, type>
                args_.emplace_back(s.first, s.second.id, s.second.type);
            }
        }
    }

    void Receive(const Record *input_record) { input_record_ = input_record; }

    OpResult Initialize(RTContext *ctx) override {
        auto sym_tab = &pattern_graph_->symbol_table;
        // allocate a new record
        record = std::make_shared<Record>(sym_tab->symbols.size(), sym_tab, ctx->param_tab_);
        for (auto &arg : args_) {
            if (arg.type == SymbolNode::NODE) {
                auto &node = pattern_graph_->GetNode(arg.alias);
                CYPHER_THROW_ASSERT(!node.Empty());
                record->values[arg.rec_idx].type = Entry::NODE;
                record->values[arg.rec_idx].node = &node;
            } else if (arg.type == SymbolNode::RELATIONSHIP) {
                auto &relp = pattern_graph_->GetRelationship(arg.alias);
                CYPHER_THROW_ASSERT(!relp.Empty());
                record->values[arg.rec_idx].type = Entry::RELATIONSHIP;
                record->values[arg.rec_idx].relationship = &relp;
            } else {
                CYPHER_TODO();
            }
        }
        state = StreamUnInitialized;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        for (auto &a : args_) {
            auto it = input_record_->symbol_table->symbols.find(a.alias);
            if (it == input_record_->symbol_table->symbols.end()) {
                throw lgraph::CypherException("Unknown node variable: " + a.alias);
            }
            auto &input = input_record_->values[it->second.id];
            if (input.type != Entry::NODE) CYPHER_TODO();
            /* Note: The input may be invalid, such as nodes produced by OPTIONAL MATCH.
             * Set the record invalid in that cases. */
            record->values[a.rec_idx].node->PushVid(input.node->PullVid());
        }
        state = StreamDepleted;
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        state = StreamUnInitialized;
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};

}  // namespace cypher
