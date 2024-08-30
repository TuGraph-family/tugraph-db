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

#pragma once

#include "core/data_type.h"
#include "cypher/execution_plan/ops/op.h"
#include "cypher/cypher_exception.h"
#include "geax-front-end/ast/AstNode.h"

namespace cypher {

class NodeIndexSeekByRange : public OpBase {
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
    geax::frontend::AstNodeType cmpOp_;
    lgraph::FieldData startFieldData;
    lgraph::FieldData endFieldData;
 
 public:
    NodeIndexSeekByRange(Node *node, const SymbolTable *sym_tab, std::string field = "",
                  std::vector<lgraph::FieldData> target_values = {},
                  geax::frontend::AstNodeType cmpOP = geax::frontend::AstNodeType::kBEqual)
        : OpBase(OpType::NODE_INDEX_SEEK_BYRANGE, "Node Index Seek By Range"),
                    node_(node), sym_tab_(sym_tab), cmpOp_(cmpOP) {
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
        startFieldData = lgraph::FieldData();
        endFieldData = lgraph::FieldData();
    }

    OpResult Initialize(RTContext *ctx) override {
        // allocate a new record
        record = std::make_shared<Record>(rec_length_, sym_tab_, ctx->param_tab_);
        record->values[node_rec_idx_].type = Entry::NODE;
        record->values[node_rec_idx_].node = node_;
        
        CYPHER_THROW_ASSERT(!target_values_.empty());
        CYPHER_THROW_ASSERT(cmpOp_ == geax::frontend::AstNodeType::kBSmallerThan ||
                            cmpOp_ == geax::frontend::AstNodeType::kBGreaterThan);
        auto value = target_values_[0];
        if (cmpOp_ == geax::frontend::AstNodeType::kBGreaterThan) {
            startFieldData = value;
        } else if (cmpOp_ == geax::frontend::AstNodeType::kBSmallerThan) {
            endFieldData = value;
        }
        if (!node_->Label().empty() &&
                ctx->txn_->GetTxn()->IsIndexed(node_->Label(), field_)) {
                it_->Initialize(ctx->txn_->GetTxn().get(),
                            lgraph::VIter::INDEX_ITER, node_->Label(),
                            field_,  startFieldData, endFieldData);
        } else {
            // Weak index iterator
            it_->Initialize(ctx->txn_->GetTxn().get(), node_->Label(),
                                field_, startFieldData, endFieldData);
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

        if (!it_ || !it_->IsValid()) return OP_DEPLETED;
        if (!consuming_) {
            consuming_ = true;
        } else {
            it_->Next();
            if (!it_->IsValid()) return OP_DEPLETED;
        }
#ifndef NDEBUG
        LOG_DEBUG() << alias_ << ": " << it_->GetId();
#endif
        return OP_OK;
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
        if (cmpOp_ == geax::frontend::AstNodeType::kBSmallerThan) {
            str.append("< ").append(target_values_[0].ToString());
        } else if (cmpOp_ == geax::frontend::AstNodeType::kBGreaterThan) {
            str.append("> ").append(target_values_[0].ToString());
        }
        str.append("]");
        return str;
    }

    Node *GetNode() const { return node_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
