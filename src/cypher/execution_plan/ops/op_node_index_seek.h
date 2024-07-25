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
// Created by wt on 7/2/18.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class NodeIndexSeek : public OpBase {
    // std::unique_ptr<lgraph::Transaction> *txn_ = nullptr;
    Node *node_ = nullptr;
    lgraph::VIter *it_ = nullptr;           // also can be derived from node
    std::string alias_;                     // also can be derived from node
    int node_rec_idx_;                      // index of node in record
    int rec_length_;                        // number of entries in a record.
    const SymbolTable *sym_tab_ = nullptr;  // build time context
    bool consuming_ = false;                // whether begin consuming
    std::string field_;
    int value_rec_idx_;
    std::vector<lgraph::FieldData> target_values_;

    OpResult HandOff() {
        if (!it_ || !it_->IsValid()) return OP_REFRESH;
        if (!consuming_) {
            consuming_ = true;
            return OP_OK;
        }
        it_->Next();
        return it_->IsValid() ? OP_OK : OP_REFRESH;
    }

 public:
    NodeIndexSeek(Node *node, const SymbolTable *sym_tab, std::string field = "",
                  std::vector<lgraph::FieldData> target_values = {})
        : OpBase(OpType::NODE_INDEX_SEEK, "Node Index Seek"), node_(node), sym_tab_(sym_tab) {
        if (node) {
            it_ = node->ItRef();
            alias_ = node->Alias();
            modifies.emplace_back(alias_);
        }
        auto it = sym_tab->symbols.find(alias_);
        CYPHER_THROW_ASSERT(node && it != sym_tab->symbols.end());
        if (it != sym_tab->symbols.end()) node_rec_idx_ = it->second.id;
        rec_length_ = sym_tab->symbols.size();
        target_values_ = std::move(target_values);
        field_ = std::move(field);
        value_rec_idx_ = 0;
    }

    OpResult Initialize(RTContext *ctx) override {
        // allocate a new record
        record = std::make_shared<Record>(rec_length_, sym_tab_, ctx->param_tab_);
        record->values[node_rec_idx_].type = Entry::NODE;
        record->values[node_rec_idx_].node = node_;

        auto &pf = node_->Prop();
        field_ = pf.type != Property::NUL ? pf.field : field_;  // use pf.field if applicable
        if (pf.type == Property::VALUE) {
            target_values_.emplace_back(pf.value);
        } else if (pf.type == Property::PARAMETER) {
            auto it = ctx->param_tab_.find(pf.value_alias);
            if (it == ctx->param_tab_.end())
                throw lgraph::CypherException("invalid parameter: " + pf.value_alias);
            if (it->second.type != cypher::FieldData::SCALAR) {
                throw lgraph::CypherException("parameter with wrong type: " + pf.value_alias);
            }
            target_values_.emplace_back(it->second.scalar);
        } else if (pf.type == Property::VARIABLE) {
            CYPHER_TODO();
        }
        CYPHER_THROW_ASSERT(!target_values_.empty());
        auto value = target_values_[0];
        if (!node_->Label().empty() && ctx->txn_->GetTxn()->IsIndexed(node_->Label(), field_)) {
            it_->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::INDEX_ITER, node_->Label(),
                            field_, value, value);
        } else {
            // Weak index iterator
            it_->Initialize(ctx->txn_->GetTxn().get(), node_->Label(), field_, value);
        }
        consuming_ = false;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        /* Always set node's vid to -1:
         * - if found a new vertex ok, pull vid when use it;
         * - otherwise, set node to -1 in case mistaken for the
         *   vertex to be valid.
         * */
        node_->SetVid(-1);

        if (HandOff() == OP_OK) return OP_OK;
        while ((size_t)value_rec_idx_ < target_values_.size() - 1) {
            value_rec_idx_++;
            auto value = target_values_[value_rec_idx_];
            if (!node_->Label().empty() && ctx->txn_->GetTxn()->IsIndexed(node_->Label(), field_)) {
                it_->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::INDEX_ITER,
                                node_->Label(), field_, value, value);
            } else {
                // Weak index iterator
                it_->Initialize(ctx->txn_->GetTxn().get(), node_->Label(), field_, value);
            }
            if (it_->IsValid()) {
                return OP_OK;
            }
        }
        return OP_DEPLETED;
    }

    OpResult ResetImpl(bool complete = false) override {
        consuming_ = false;
        if (complete) {
            // undo method initialize()
            record = nullptr;
            if (it_ && it_->Initialized()) it_->FreeIter();
        } else {
            if (it_ && it_->Initialized()) it_->Reset();
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(alias_).append("]");
        str.append(" ").append(field_).append(" IN [");
        for (const auto &v : target_values_) {
            str.append(v.ToString()).append(",");
        }
        str.append("]");
        return str;
    }

    Node *GetNode() const { return node_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
