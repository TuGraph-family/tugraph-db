﻿/**
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

#include "op.h"

namespace cypher {

class AllNodeScan : public OpBase {
    /* NOTE: Nodes in pattern graph are stored in std::vector, whose reference
     * will become INVALID after reallocation.
     * TODO: Make sure not add nodes to the pattern graph, otherwise use NodeId instead.  */
    friend class LocateNodeByVid;
    friend class LocateNodeByIndexedProp;

    Node *node_ = nullptr;
    lgraph::VIter *it_ = nullptr;           // also can be derived from node
    std::string alias_;                     // also can be derived from node
    std::string label_;                     // also can be derived from node
    int node_rec_idx_;                      // index of node in record
    int rec_length_;                        // number of entries in a record.
    const SymbolTable *sym_tab_ = nullptr;  // build time context
    bool consuming_ = false;                // whether begin consuming

 public:
    AllNodeScan(Node *node, const SymbolTable *sym_tab)
        : OpBase(OpType::ALL_NODE_SCAN, "All Node Scan"), node_(node), sym_tab_(sym_tab) {
        if (node) {
            it_ = node->ItRef();
            alias_ = node->Alias();
            modifies.emplace_back(alias_);
        }
        auto it = sym_tab->symbols.find(alias_);
        CYPHER_THROW_ASSERT(node && it != sym_tab->symbols.end());
        if (it != sym_tab->symbols.end()) node_rec_idx_ = it->second.id;
        rec_length_ = sym_tab->symbols.size();
        consuming_ = false;
    }

    OpResult Initialize(RTContext *ctx) override {
        // allocate a new record
        record = std::make_shared<Record>(rec_length_, sym_tab_);
        record->values[node_rec_idx_].type = Entry::NODE;
        record->values[node_rec_idx_].node = node_;
        record->SetParameter(ctx->param_tab_);
        // transaction allocated before in plan:execute
        // todo: remove patternGraph's state (ctx)
        node_->ItRef()->Initialize(ctx->txn_.get(), lgraph::VIter::VERTEX_ITER);
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
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        consuming_ = false;
        if (complete) {
            // undo method initialize()
            record = nullptr;
            // todo: cleaned in ExecutionPlan::Execute
            if (it_ && it_->Initialized()) it_->FreeIter();
        } else {
            if (it_ && it_->Initialized()) it_->Reset();
        }
        return OP_OK;
    }

    std::string ToString() const override {
        std::string str(name);
        str.append(" [").append(alias_).append("]");
        return str;
    }

    Node *GetNode() const { return node_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()

};
}  // namespace cypher
