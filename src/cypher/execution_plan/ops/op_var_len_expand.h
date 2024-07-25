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
// Created by wt on 18-8-30.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"

#ifndef NDEBUG
#define VAR_LEN_EXP_DUMP_FOR_DEBUG()                                                         \
    do {                                                                                       \
        LOG_DEBUG() << __func__ << __LINE__ << ": hop=" << hop_ << ", edge="                   \
                  << (hop_ == 0 ? "na" : _detail::EdgeUid2String(eits_[hop_ - 1].GetUid())); \
        LOG_DEBUG() << pattern_graph_->VisitedEdges().Dump();                                  \
    } while (0)
#else
#define VAR_LEN_EXP_DUMP_FOR_DEBUG()
#endif

namespace cypher {

/* Variable Length Expand */
class VarLenExpand : public OpBase {
    void _InitializeEdgeIter(RTContext *ctx, int64_t vid, lgraph::EIter &eit) {
        auto &types = relp_->Types();
        auto iter_type = lgraph::EIter::NA;
        switch (expand_direction_) {
        case ExpandTowards::FORWARD:
            iter_type = types.empty() ? lgraph::EIter::OUT_EDGE : lgraph::EIter::TYPE_OUT_EDGE;
            break;
        case ExpandTowards::REVERSED:
            iter_type = types.empty() ? lgraph::EIter::IN_EDGE : lgraph::EIter::TYPE_IN_EDGE;
            break;
        case ExpandTowards::BIDIRECTIONAL:
            iter_type = types.empty() ? lgraph::EIter::BI_EDGE : lgraph::EIter::BI_TYPE_EDGE;
            break;
        }
        eit.Initialize(ctx->txn_->GetTxn().get(), iter_type, vid, types, {});
    }

#if 0  // 20210704
        void _CollectFrontierByDFS(RTContext *ctx, int64_t vid, const std::set<std::string> &types, int min_hop, int max_hop) { // NOLINT
            if (hop_ >= min_hop) {
                if (neighbor_->Label().empty()
                    || ctx->txn_->GetVertexLabel(ctx->txn_->GetVertexIterator(vid)) == neighbor_->Label()) { // NOLINT
                    frontier_buffer_.emplace(vid);
                    path_buffer_.emplace(relp_->path_);
                }
#ifndef NDEBUG
                LOG_DEBUG() << __func__ << ": hop=" << hop_ << ",vid=" << vid;
                LOG_DEBUG() << pattern_graph_->VisitedEdges().Dump();
#endif
            }
            if (hop_ == max_hop) return;
            hop_++;
            lgraph::EIter eit;
            _InitializeEdgeIter(ctx, vid, eit);
            while (eit.IsValid()) {
                if (!pattern_graph_->VisitedEdges().Contains(eit)) {
                    auto r = pattern_graph_->VisitedEdges().Add(eit);
                    if (!r.second) CYPHER_INTL_ERR();
                    relp_->path_.Append(eit.GetUid());
                    _CollectFrontierByDFS(ctx, eit.GetNbr(expand_direction_), types, min_hop, max_hop); // NOLINT
                    relp_->path_.PopBack();
                    pattern_graph_->VisitedEdges().Erase(r.first);
                }
                eit.Next();
            }
            hop_--;
        }

        void _CollectFrontierByDFS(RTContext *ctx, int64_t vid, const std::set<std::string> &types, int min_hop) { // NOLINT
            if (hop_ == min_hop) {
                if (neighbor_->Label().empty()
                    || ctx->txn_->GetVertexLabel(ctx->txn_->GetVertexIterator(vid)) == neighbor_->Label()) { // NOLINT
                    frontier_buffer_.emplace(vid);
                    path_buffer_.emplace(relp_->path_);
                }
#ifndef NDEBUG
                LOG_INFO() << __func__ << ": hop=" << hop_ << ",vid=" << vid;
                LOG_INFO() << pattern_graph_->VisitedEdges().Dump();
#endif
                return;
            }
            hop_++;
            lgraph::EIter eit;
            _InitializeEdgeIter(ctx, vid, eit);
            while (eit.IsValid()) {
                if (!pattern_graph_->VisitedEdges().Contains(eit)) {
                    auto r = pattern_graph_->VisitedEdges().Add(eit);
                    if (!r.second) CYPHER_INTL_ERR();
                    relp_->path_.Append(eit.GetUid());
                    _CollectFrontierByDFS(ctx, eit.GetNbr(expand_direction_), types, min_hop);
                    relp_->path_.PopBack();
                    pattern_graph_->VisitedEdges().Erase(r.first);
                }
                eit.Next();
            }
            hop_--;
        }

        OpResult Next(RTContext *ctx) {
            if (state_ == Uninitialized) return OP_REFRESH;
            /* Start node iterator may be invalid, such as when the start is an argument
             * produced by OPTIONAL MATCH.  */
            if (!start_it_->IsValid()) return OP_REFRESH;
            auto &types = relp_->Types();
            if (collect_all_ || min_hop_ == 0) {  // we didnot handle 0hop in other branch
                if (state_ == Resetted) {
                    relp_->path_.SetStart(start_it_->GetId());
                    /* collect all the vertex, save them into result_buffer_ */
                    _CollectFrontierByDFS(ctx, start_it_->GetId(), types,
                                          min_hop_, max_hop_);
                    state_ = Consuming;
                }
                if (frontier_buffer_.empty()) return OP_REFRESH;
                nbr_it_->Initialize(ctx->txn_.get(), lgraph::VIter::VERTEX_ITER,
                                    frontier_buffer_.front());
                frontier_buffer_.pop();
                relp_->path_ = path_buffer_.front();
                path_buffer_.pop();
            } else {
                // produce one by one
                if (state_ == Resetted) {
                    relp_->path_.SetStart(start_it_->GetId());
                    hop_ = 0;
                    _CollectFrontierByDFS(ctx, start_it_->GetId(), types, min_hop_);
                    state_ = Consuming;
                }
                if (frontier_buffer_.empty()) return OP_REFRESH;
                auto vid = frontier_buffer_.front();
                frontier_buffer_.pop();
                nbr_it_->Initialize(ctx->txn_.get(), lgraph::VIter::VERTEX_ITER, vid);
                relp_->path_ = path_buffer_.front();
                path_buffer_.pop();
                if (relp_->path_.Length() < max_hop_) {
                    lgraph::EIter eit;
                    _InitializeEdgeIter(ctx, vid, eit);
                    // construct visitedEdges from relp_->path_
                    pattern_graph_->VisitedEdges().euid_hash_set.clear();
                    for (size_t i = 0; i < relp_->path_.Length(); i++) {
                        pattern_graph_->VisitedEdges().euid_hash_set.emplace(relp_->path_.GetNthEdge(i)); // NOLINT
                    }
                    while (eit.IsValid()) {
                        if (!pattern_graph_->VisitedEdges().Contains(eit)) {
                            if (neighbor_->Label().empty() ||
                                ctx->txn_->GetVertexLabel(
                                    ctx->txn_->GetVertexIterator(vid)) ==
                                    neighbor_->Label()) {
                                frontier_buffer_.emplace(eit.GetNbr(expand_direction_));
                                relp_->path_.Append(eit.GetUid());
                                path_buffer_.emplace(relp_->path_);
                                relp_->path_.PopBack();
                            }
                        }
                        eit.Next();
                    }
                }
            }  // if collect all
#ifndef NDEBUG
            LOG_DEBUG() << "[" << __FILE__ << "] neighbor:" << nbr_it_->GetId();
#endif
            return OP_OK;
        }
#endif

    int64_t GetFirstFromKthHop(RTContext *ctx, size_t k) {
        auto start_id = start_->PullVid();
        relp_->path_.Clear();
        relp_->path_.SetStart(start_id);
        if (k == 0) return start_id;
        _InitializeEdgeIter(ctx, start_id, eits_[0]);
        if (!eits_[0].IsValid()) return -1;
        if (k == 1) {
            relp_->path_.Append(eits_[0].GetUid());
            if (ctx->path_unique_) pattern_graph_->VisitedEdges().Add(eits_[0]);
            return eits_[0].GetNbr(expand_direction_);
        }
        // k >= 2
        for (size_t i = 0; i < k; i++) {
            lgraph::EdgeUid dummy(start_id, start_id, -1, 0, -1);
            relp_->path_.Append(dummy);
        }
        return GetNextFromKthHop(ctx, k, true);
    }

    // curr_hop start from 1,2,3..
    int64_t GetNextFromKthHop(RTContext *ctx, size_t k, bool get_first) {
        if (k == 0) return -1;
        if (ctx->path_unique_) pattern_graph_->VisitedEdges().Erase(eits_[k - 1]);
        relp_->path_.PopBack();
        /* If get the first node, the 1st edge(eits[0]) is the only iterator
         * that is initialized and should not go next.
         **/
        if (!get_first || k != 1 ||
            (ctx->path_unique_ && pattern_graph_->VisitedEdges().Contains(eits_[k - 1]))) {
            do {
                eits_[k - 1].Next();
            } while (eits_[k - 1].IsValid() && ctx->path_unique_ &&
                     pattern_graph_->VisitedEdges().Contains(eits_[k - 1]));
        }
        do {
            if (!eits_[k - 1].IsValid()) {
                auto id = GetNextFromKthHop(ctx, k - 1, get_first);
                if (id < 0) return id;
                _InitializeEdgeIter(ctx, id, eits_[k - 1]);
                /* We have called get_next previously, mark get_first as
                 * false. */
                get_first = false;
            }
            while (ctx->path_unique_ && pattern_graph_->VisitedEdges().Contains(eits_[k - 1])) {
                eits_[k - 1].Next();
            }
        } while (!eits_[k - 1].IsValid());
        if (!eits_[k - 1].IsValid()) return -1;
        relp_->path_.Append(eits_[k - 1].GetUid());
        if (ctx->path_unique_) pattern_graph_->VisitedEdges().Add(eits_[k - 1]);
        return eits_[k - 1].GetNbr(expand_direction_);
    }

    OpResult NextWithoutLabelFilter(RTContext *ctx) {
        if (state_ == Uninitialized) return OP_REFRESH;
        /* Start node iterator may be invalid, such as when the start is an argument
         * produced by OPTIONAL MATCH.  */
        if (start_->PullVid() < 0) return OP_REFRESH;
        if (state_ == Resetted) {
            // go to min_hop
            hop_ = min_hop_;
            int64_t nbr_id = GetFirstFromKthHop(ctx, hop_);
            if (nbr_id < 0) return OP_REFRESH;
            neighbor_->PushVid(nbr_id);
            VAR_LEN_EXP_DUMP_FOR_DEBUG();
            state_ = Consuming;
            return OP_OK;
        }
        auto vid = GetNextFromKthHop(ctx, hop_, false);
        if (vid >= 0) {
            neighbor_->PushVid(vid);
            VAR_LEN_EXP_DUMP_FOR_DEBUG();
            return OP_OK;
        } else {
            // need expand to next hop
            if (hop_ == max_hop_) return OP_REFRESH;
            hop_++;
            auto vid = GetFirstFromKthHop(ctx, hop_ - 1);
            if (vid < 0) return OP_REFRESH;
            if (hop_ > 1 && !eits_[hop_ - 2].IsValid()) CYPHER_INTL_ERR();
            _InitializeEdgeIter(ctx, vid, eits_[hop_ - 1]);
            // TODO(anyone) merge these code similiar to GetNextFromKthHop
            do {
                if (!eits_[hop_ - 1].IsValid()) {
                    auto v = GetNextFromKthHop(ctx, hop_ - 1, false);
                    if (v < 0) return OP_REFRESH;
                    _InitializeEdgeIter(ctx, v, eits_[hop_ - 1]);
                }
                while (ctx->path_unique_ &&
                       pattern_graph_->VisitedEdges().Contains(eits_[hop_ - 1])) {
                    eits_[hop_ - 1].Next();
                }
            } while (!eits_[hop_ - 1].IsValid());
            neighbor_->PushVid(eits_[hop_ - 1].GetNbr(expand_direction_));
            relp_->path_.Append(eits_[hop_ - 1].GetUid());
            // TODO(anyone) remove in last hop
            if (ctx->path_unique_) pattern_graph_->VisitedEdges().Add(eits_[hop_ - 1]);
            VAR_LEN_EXP_DUMP_FOR_DEBUG();
            return OP_OK;
        }
    }

    OpResult Next(RTContext *ctx) {
        do {
            if (NextWithoutLabelFilter(ctx) != OP_OK) return OP_REFRESH;
        } while (!neighbor_->Label().empty() && neighbor_->IsValidAfterMaterialize(ctx) &&
                 neighbor_->ItRef()->GetLabel() != neighbor_->Label());
        return OP_OK;
    }

 public:
    cypher::PatternGraph *pattern_graph_ = nullptr;
    cypher::Node *start_ = nullptr;         // start node to expand
    cypher::Node *neighbor_ = nullptr;      // neighbor of start node
    cypher::Relationship *relp_ = nullptr;  // relationship to expand
    int start_rec_idx_;
    int nbr_rec_idx_;
    int relp_rec_idx_;
    int min_hop_;
    int max_hop_;
    int hop_;  // current hop working on
    bool collect_all_;
    ExpandTowards expand_direction_;
    std::vector<lgraph::EIter> &eits_;
    enum State {
        Uninitialized, /* ExpandAll wasn't initialized it. */
        Resetted,      /* ExpandAll was just restarted. */
        Consuming,     /* ExpandAll consuming data. */
    } state_;

    VarLenExpand(PatternGraph *pattern_graph, Node *start, Node *neighbor, Relationship *relp)
        : OpBase(OpType::VAR_LEN_EXPAND, "Variable Length Expand"),
          pattern_graph_(pattern_graph),
          start_(start),
          neighbor_(neighbor),
          relp_(relp),
          min_hop_(relp->MinHop()),
          max_hop_(relp->MaxHop()),
          hop_(0),
          collect_all_(false),
          eits_(relp_->ItsRef()) {
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
        eits_.resize(max_hop_);
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override {
        CYPHER_THROW_ASSERT(!children.empty());
        auto child = children[0];
        while (state_ == Uninitialized || Next(ctx) == OP_REFRESH) {
            auto res = child->Consume(ctx);
            relp_->path_.Clear();
            state_ = Resetted;
            if (res != OP_OK) {
                /* When consume after the stream is DEPLETED, make sure
                 * the result always be DEPLETED.  */
                state_ = Uninitialized;
                return res;
            }
            /* Most of the time, the start_it is definitely valid after child's Consume
             * returns OK, except when the child is an OPTIONAL operation.  */
        }
        return OP_OK;
    }

    OpResult ResetImpl(bool complete) override {
        state_ = Uninitialized;
        // std::queue<lgraph::VertexId>().swap(frontier_buffer_);
        // std::queue<Path>().swap(path_buffer_);
        hop_ = 0;
        // TODO(anyone) reset modifies
        return OP_OK;
    }

    std::string ToString() const override {
        auto towards = expand_direction_ == FORWARD    ? "-->"
                       : expand_direction_ == REVERSED ? "<--"
                                                       : "--";
        return fma_common::StringFormatter::Format(
            "{}({}) [{} {}*{}..{} {}]", name, "All", start_->Alias(), towards,
            std::to_string(min_hop_), std::to_string(max_hop_), neighbor_->Alias());
    }

    Node *GetStartNode() const { return start_; }
    Node *GetNeighborNode() const { return neighbor_; }
    Relationship *GetRelationship() const { return relp_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
