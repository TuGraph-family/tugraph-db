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

/*
 * @Author: gelincheng
 * @Date: 2022-01-11
 * @LastEditors: gelincheng
 * @Description:
 */
#pragma once

#include <utility>

#include "arithmetic/arithmetic_expression.h"
#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class NodeByIdSeek : public OpBase {
    std::vector<lgraph::VertexId> target_vids_;
    size_t idx_ = 0;
    Node *node_ = nullptr;
    lgraph::VIter *it_ = nullptr;
    std::string alias_;
    std::string label_;
    int node_rec_idx_;
    int rec_length_;
    const SymbolTable *sym_tab_ = nullptr;

 public:
    NodeByIdSeek(std::vector<lgraph::VertexId> target_vids, Node *node, const SymbolTable *sym_tab)
        : OpBase(OpType::NODE_BY_ID_SEEK, "Node By Id Seek"),
          target_vids_(std::move(target_vids)),
          node_(node),
          sym_tab_(sym_tab) {
        if (node) {
            it_ = node->ItRef();
            alias_ = node->Alias();
            label_ = node->Label();
            modifies.emplace_back(alias_);
        }
        auto it = sym_tab->symbols.find(alias_);
        CYPHER_THROW_ASSERT(node && it != sym_tab->symbols.end());
        if (it != sym_tab->symbols.end()) node_rec_idx_ = it->second.id;
        rec_length_ = sym_tab->symbols.size();
    }

    OpResult Initialize(RTContext *ctx) override {
        record = std::make_shared<Record>(rec_length_, sym_tab_, ctx->param_tab_);
        record->values[node_rec_idx_].type = Entry::NODE;
        record->values[node_rec_idx_].node = node_;
        record->SetParameter(ctx->param_tab_);
        node_->ItRef()->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::VERTEX_ITER);
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        // remember set vid to -1 every time
        node_->SetVid(-1);

        if (!it_ || !it_->IsValid()) return OP_DEPLETED;
        while (idx_ < target_vids_.size()) {
            if (it_->Goto(target_vids_[idx_])) {
                if (label_.empty() || it_->GetLabel() == label_) {
                    idx_++;
                    return OP_OK;
                }
            }
            idx_++;
        }
        return OP_DEPLETED;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append("[");
        for (auto v : target_vids_) {
            str.append(std::to_string(v)).append(",");
        }
        str.append("]");

        return str;
    }

    OpResult ResetImpl(bool complete = false) override {
        if (complete) {
            // undo method initialize()
            record = nullptr;
            if (it_ && it_->Initialized()) it_->FreeIter();
        } else {
            if (it_ && it_->Initialized()) it_->Reset();
        }
        return OP_OK;
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
