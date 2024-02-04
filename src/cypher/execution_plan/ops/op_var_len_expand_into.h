/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "cypher/execution_plan/ops/op.h"

namespace cypher {

/* Variable Length Expand Into */
class VarLenExpandInto : public OpBase {
 public:
    cypher::PatternGraph *pattern_graph_ = nullptr;
    cypher::Node *start_ = nullptr;         // start node to expand
    cypher::Node *neighbor_ = nullptr;      // neighbor of start node
    cypher::Relationship *relp_ = nullptr;  // relationship to expand
    lgraph::VIter *start_it_ = nullptr;
    lgraph::VIter *nbr_it_ = nullptr;
    lgraph::EIter *eit_ = nullptr;
    int start_rec_idx_;
    int nbr_rec_idx_;
    int relp_rec_idx_;
    int min_hop_;
    int max_hop_;
    int hop_;  // current hop working on
    ExpandTowards expand_direction_;
    std::unordered_multimap<lgraph::VertexId, Path> fwd_partial;
    std::unordered_multimap<lgraph::VertexId, Path> bwd_partial;
    std::vector<lgraph::VertexId> fwd_q;
    std::vector<lgraph::VertexId> bwd_q;

    enum State {
        Uninitialized, /* Expand wasn't initialized it. */
        Resetted,      /* Expand was just restarted. */
        Consuming,     /* Expand consuming data. */
    } state_;

    VarLenExpandInto(PatternGraph *pattern_graph, Node *start, Node *neighbor, Relationship *relp)
        : OpBase(OpType::VAR_LEN_EXPAND_INTO, "Variable Length Expand Into"),
          pattern_graph_(pattern_graph),
          start_(start),
          neighbor_(neighbor),
          relp_(relp),
          min_hop_(relp->MinHop()),
          max_hop_(relp->MaxHop()),
          hop_(0) {
        start_it_ = start->ItRef();
        nbr_it_ = neighbor->ItRef();
        eit_ = relp->ItRef();
        modifies.emplace_back(neighbor_->Alias());
        modifies.emplace_back(relp_->Alias());
        auto &sym_tab = pattern_graph->symbol_table;
        auto sit = sym_tab.symbols.find(start_->Alias());
        auto dit = sym_tab.symbols.find(neighbor_->Alias());
        auto rit = sym_tab.symbols.find(relp_->Alias());
        CYPHER_THROW_ASSERT(sit != sym_tab.symbols.end() && dit != sym_tab.symbols.end() &&
                            rit != sym_tab.symbols.end());
        expand_direction_ = relp_->Undirected()            ? BIDIRECTIONAL
                            : relp_->Src() == start_->ID() ? FORWARD
                                                           : REVERSED;
        start_rec_idx_ = sit->second.id;
        nbr_rec_idx_ = dit->second.id;
        relp_rec_idx_ = rit->second.id;
        state_ = Uninitialized;
    }

    OpResult Initialize(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto child = children[0];
        auto res = child->Initialize(ctx);
        if (res != OP_OK) return res;
        record = child->record;
        record->values[start_rec_idx_].type = Entry::NODE;
        record->values[start_rec_idx_].node = start_;
        record->values[nbr_rec_idx_].type = Entry::NODE;
        record->values[nbr_rec_idx_].node = neighbor_;
        record->values[relp_rec_idx_].type = Entry::VAR_LEN_RELP;
        record->values[relp_rec_idx_].relationship = relp_;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override { return OP_OK; }

    OpResult ResetImpl(bool complete) override {
        state_ = Uninitialized;
        hop_ = 0;
        return OP_OK;
    }

    std::string ToString() const override {
        auto towards = expand_direction_ == FORWARD    ? "-->"
                       : expand_direction_ == REVERSED ? "<--"
                                                       : "--";
        return fma_common::StringFormatter::Format(
            "{}({}) [{} {}*{}..{} {}]", name, "Into", start_->Alias(), towards,
            std::to_string(min_hop_), std::to_string(max_hop_), neighbor_->Alias());
    }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
