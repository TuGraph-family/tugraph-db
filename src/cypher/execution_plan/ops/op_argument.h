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
// Created by wt on 19-10-11.
//

#pragma once

#include "op.h"

namespace cypher {

struct ArgIndex {
    std::string alias;
    // ASSERT the indexes in THIS record and INPUT record are the same.
    size_t rec_idx;
    SymbolNode::Type type;

    ArgIndex(const std::string &a, size_t idx, SymbolNode::Type t)
        : alias(a), rec_idx(idx), type(t) {}
};

class Argument : public OpBase {
    std::vector<ArgIndex> args_;
    std::shared_ptr<Record> *input_record_ = nullptr;
    const SymbolTable *sym_tab_ = nullptr;  // build time context
    PatternGraph *pattern_graph_ = nullptr;

 public:
    Argument(const SymbolTable *sym_tab) : OpBase(OpType::ARGUMENT, "Argument"), sym_tab_(sym_tab) {
        std::map<size_t, std::pair<std::string, SymbolNode::Type>> ordered_alias;
        for (auto &a : sym_tab->symbols) {
            if (a.second.scope == SymbolNode::ARGUMENT) {
                ordered_alias.emplace(a.second.id, std::make_pair(a.first, a.second.type));
            }
        }
        for (auto &a : ordered_alias) {
            // <alias, rec_idx, type>
            args_.emplace_back(a.second.first, a.first, a.second.second);
            modifies.emplace_back(a.second.first);
        }
    }

    void Receive(std::shared_ptr<Record> *input_record, PatternGraph *pattern_graph) {
        input_record_ = input_record;
        pattern_graph_ = pattern_graph;
    }

    OpResult Initialize(RTContext *ctx) override {
        // allocate a new record
        record = std::make_shared<Record>(sym_tab_->symbols.size(), sym_tab_);
        record->SetParameter(ctx->param_tab_);
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
            } else if (arg.type == SymbolNode::CONSTANT) {
                record->values[arg.rec_idx].type = Entry::CONSTANT;
            } else {
                CYPHER_TODO();
            }
        }
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        if (state == StreamDepleted) return OP_DEPLETED;
        for (auto &a : args_) {
            auto &input = (*input_record_)->values[a.rec_idx];
            auto &value = record->values[a.rec_idx];
            int64_t vid = -1;
            switch (input.type) {
            case Entry::CONSTANT:
                record->values[a.rec_idx].constant = input.constant;
                break;
            case Entry::NODE:
                /* Note: The input may be invalid, such as nodes produced by OPTIONAL MATCH.
                 * Set the record invalid in that cases. */
                record->values[a.rec_idx].node->PushVid(input.node->PullVid());
                break;
            case Entry::NODE_SNAPSHOT:
                if (vid < 0) {
                    // TODO: use integer directly // NOLINT
                    // extract vid from snapshot, "V[2020]"
                    CYPHER_THROW_ASSERT(input.constant.IsString());
                    const auto &str = input.constant.scalar.string();
                    vid = std::stoi(str.substr(2, str.size() - 3));
                }
                if (vid < 0) CYPHER_TODO();
                record->values[a.rec_idx].node->PushVid(vid);
                break;
            case Entry::RELATIONSHIP:
                if (!input.relationship->ItRef()->IsValid()) CYPHER_TODO();
                record->values[a.rec_idx].relationship->ItRef()->Initialize(
                    ctx->txn_.get(), input.relationship->ItRef()->GetUid());
                break;
            default:
                CYPHER_TODO();
            }
        }
        state = StreamDepleted;
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        if (complete) {
            record = nullptr;
            state = StreamUnInitialized;
        } else {
            state = StreamUnInitialized;
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [");
        for (auto &a : args_) str.append(a.alias).append(",");
        str.pop_back();
        str.append("]");
        return str;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()

};
}  // namespace cypher
