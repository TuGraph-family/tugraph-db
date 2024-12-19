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
// Created by wt on 6/13/18.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

class NodeByLabelScan : public OpBase {
    friend class LocateNodeByVid;
    friend class LocateNodeByVidV2;
    friend class LocateNodeByIndexedProp;
    friend class LocateNodeByIndexedPropV2;
    friend class LocateNodeByPropRangeFilter;

    Node *node_ = nullptr;
    lgraph::VIter *it_ = nullptr;           // also cab be derived from node
    std::string alias_;                     // also can be derived from node
    std::string label_;                     // also can be derived from node
    int node_rec_idx_;                      // index of node in record
    int rec_length_;                        // number of entries in a record.
    const SymbolTable *sym_tab_ = nullptr;  // build time context
    bool consuming_ = false;                // whether begin consuming
    std::unordered_map<std::string, std::pair<lgraph::FieldData, lgraph::FieldData>> field_bounds_;

 public:
    NodeByLabelScan(Node *node, const SymbolTable *sym_tab,
                    std::unordered_map<std::string, std::pair<lgraph::FieldData, lgraph::FieldData>>
                        field_bounds = {})
        : OpBase(OpType::NODE_BY_LABEL_SCAN, "Node By Label Scan"), node_(node), sym_tab_(sym_tab) {
        CYPHER_THROW_ASSERT(node);
        it_ = node->ItRef();
        alias_ = node->Alias();
        label_ = node->Label();
        modifies.emplace_back(alias_);
        auto it = sym_tab->symbols.find(alias_);
        CYPHER_THROW_ASSERT(it != sym_tab->symbols.end());
        node_rec_idx_ = it->second.id;
        rec_length_ = sym_tab->symbols.size();
        consuming_ = false;
        field_bounds_ = std::move(field_bounds);
    }

    OpResult Initialize(RTContext *ctx) override {
        // allocate a new record
        record = std::make_shared<Record>(rec_length_, sym_tab_, ctx->param_tab_);
        record->values[node_rec_idx_].type = Entry::NODE;
        record->values[node_rec_idx_].node = node_;
        // transaction allocated before in plan:execute
        auto primary_field = ctx->txn_->GetVertexPrimaryField(label_);
        for (auto &[field, bounds] : field_bounds_) {
            if (field == primary_field) {
                break;
            }
            if (ctx->txn_->GetTxn()->IsIndexed(label_, field)) {
                it_->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::INDEX_ITER, label_, field,
                                bounds.first, bounds.second);
                return OP_OK;
            }
        }

        // fallback to primary index
        it_->Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::INDEX_ITER, label_, primary_field,
                        lgraph::FieldData(), lgraph::FieldData());

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

    OpResult ResetImpl(bool complete) override {
#ifndef NDEBUG
        LOG_DEBUG() << alias_ << " reset";
#endif
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
        bool flag = false;
        str.append(" [").append(alias_).append(":").append(label_).append("]");

        for (auto &[field, bounds] : field_bounds_) {
            if (flag) {
                str.append(",");
            }
            str.append(" ").append(alias_).append(".").append(field).append("[");
            str.append(bounds.first.ToString()).append(",");
            str.append(bounds.second.ToString());
            str.append(")");
            flag = true;
        }
        return str;
    }

    Node *GetNode() const { return node_; }

    const std::string& GetLabel() { return label_; }

    const SymbolTable * GetSymtab() {return sym_tab_;}

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
