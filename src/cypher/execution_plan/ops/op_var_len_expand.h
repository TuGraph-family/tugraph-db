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

#include <spdlog/spdlog.h>
#include "cypher/execution_plan/ops/op.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/graph/graph.h"

#ifndef NDEBUG
#define VAR_LEN_EXP_DUMP_FOR_DEBUG()                                          \
    do {                                                                      \
        SPDLOG_DEBUG(                                                         \
            __func__ + __LINE__ + ": hop=" + hop_ + ", edge=" +               \
            (hop_ == 0 ? "na"                                                 \
                       : eits_[hop_ - 1]->GetEdge().GetId())); \
    } while (0)
#else
#define VAR_LEN_EXP_DUMP_FOR_DEBUG()
#endif

namespace cypher {

/* Variable Length Expand */
class VarLenExpand : public OpBase {
    void _InitializeEdgeIter(RTContext *ctx, int64_t vid,
                             std::unique_ptr<graphdb::EdgeIterator> &eit) {
        auto types = std::unordered_set<std::string>{relp_->Types().begin(), relp_->Types().end()};
        std::unordered_set<uint32_t> type_set;
        for (auto &type : types) {
            auto tid = ctx->txn_->db()->id_generator().GetTid(type);
            if (tid) {
                type_set.insert(tid.value());
            }
        }
        switch (expand_direction_) {
            case ExpandTowards::FORWARD:
                eit = std::make_unique<graphdb::ScanEdgeByVidDirectionTypes>(
                    ctx->txn_, vid, graphdb::EdgeDirection::OUTGOING,
                    std::move(type_set));
                break;
            case ExpandTowards::REVERSED:
                eit = std::make_unique<graphdb::ScanEdgeByVidDirectionTypes>(
                    ctx->txn_, vid, graphdb::EdgeDirection::INCOMING,
                    std::move(type_set));
                break;
            case ExpandTowards::BIDIRECTIONAL:
                eit = std::make_unique<graphdb::ScanEdgeByVidDirectionTypes>(
                    ctx->txn_, vid, graphdb::EdgeDirection::BOTH,
                    std::move(type_set));
                break;
        }
    }

    std::optional<graphdb::Vertex> GetFirstFromKthHop(RTContext *ctx, std::optional<graphdb::Vertex> vertex,
                                                      int k) {
        if (k == 1) {
            relp_->path_.Clear();
            relp_->path_.SetStart(start_->vertex_.value());
        }
        if (k == hop_ + 1) {
            return vertex;
        }
        _InitializeEdgeIter(ctx, vertex->GetId(), eits_[k - 1]);
        while (eits_[k - 1]->Valid()) {
            if (!ctx->path_unique_ || (ctx->path_unique_ &&
                !pattern_graph_->VisitedEdges().count(eits_[k - 1]->GetEdge().GetId()))) {
                auto neighbor_vertex = eits_[k - 1]->GetEdge().GetOtherEnd(vertex->GetId());
                relp_->path_.Append(eits_[k - 1]->GetEdge(), neighbor_vertex);
                if (ctx->path_unique_) {
                    pattern_graph_->VisitedEdges().insert(eits_[k - 1]->GetEdge().GetId());
                }
                auto nxt = GetFirstFromKthHop(ctx, neighbor_vertex, k + 1);
                if (nxt) return nxt;
                relp_->path_.PopBack();
                if (ctx->path_unique_) {
                    pattern_graph_->VisitedEdges().erase(eits_[k - 1]->GetEdge().GetId());
                }
            }
            eits_[k - 1]->Next();
        }
        return {};
    }


    std::optional<graphdb::Vertex> GetNextFromKthHop(RTContext *ctx, size_t k) {
        if (k == 0) return {};
        relp_->path_.PopBack();
        if (ctx->path_unique_) {
            pattern_graph_->VisitedEdges().erase(eits_[k - 1]->GetEdge().GetId());
        }
        do {
            if (eits_[k - 1]->Valid()) {
                eits_[k - 1]->Next();
            } else {
                auto vertex = GetNextFromKthHop(ctx, k - 1);
                if (!vertex) return vertex;
                _InitializeEdgeIter(ctx, vertex->GetId(), eits_[k - 1]);
            }
            if (eits_[k - 1]->Valid() &&
                (!ctx->path_unique_ || (ctx->path_unique_ &&
                !pattern_graph_->VisitedEdges().count(eits_[k - 1]->GetEdge().GetId())))) {
                auto last = relp_->path_.vertexes.back();
                auto neighbor_vertex = eits_[k - 1]->GetEdge().GetOtherEnd(last.GetId());
                relp_->path_.Append(eits_[k - 1]->GetEdge(), neighbor_vertex);
                if (ctx->path_unique_) {
                    pattern_graph_->VisitedEdges().insert(eits_[k - 1]->GetEdge().GetId());
                }
                return neighbor_vertex;
            }
        } while (true);
    }

    OpResult NextWithoutLabelFilter(RTContext *ctx) {
        if (state_ == Uninitialized) return OP_REFRESH;
        /* Start node iterator may be invalid, such as when the start is an
         * argument produced by OPTIONAL MATCH.  */
        if (!start_->vertex_) return OP_REFRESH;
        if (state_ == Resetted) {
            // go to min_hop
            hop_ = min_hop_;
            auto nbr = GetFirstFromKthHop(ctx, start_->vertex_, 1);
            if (!nbr) return OP_REFRESH;
            neighbor_->vertex_ = nbr;
            VAR_LEN_EXP_DUMP_FOR_DEBUG();
            state_ = Consuming;
            return OP_OK;
        }
        auto vertex = GetNextFromKthHop(ctx, hop_);
        if (vertex) {
            neighbor_->vertex_ = vertex;
            VAR_LEN_EXP_DUMP_FOR_DEBUG();
            return OP_OK;
        } else {
            // need expand to next hop
            if (hop_ == max_hop_) return OP_REFRESH;
            hop_++;
            vertex = GetFirstFromKthHop(ctx, start_->vertex_, 1);
            if (!vertex) {
                return OP_REFRESH;
            } else {
                neighbor_->vertex_ = vertex;
                VAR_LEN_EXP_DUMP_FOR_DEBUG();
                return OP_OK;
            }
        }
    }

    OpResult Next(RTContext *ctx) {
        do {
            if (NextWithoutLabelFilter(ctx) != OP_OK) return OP_REFRESH;
        } while (!neighbor_->Label().empty() && neighbor_->vertex_ &&
                 !neighbor_->vertex_->GetLabels().count(neighbor_->Label()));
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
    ExpandTowards expand_direction_;
    std::vector<std::unique_ptr<graphdb::EdgeIterator>> eits_;
    enum State {
        Uninitialized, /* ExpandAll wasn't initialized it. */
        Resetted,      /* ExpandAll was just restarted. */
        Consuming,     /* ExpandAll consuming data. */
    } state_;

    VarLenExpand(PatternGraph *pattern_graph, Node *start, Node *neighbor,
                 Relationship *relp)
        : OpBase(OpType::VAR_LEN_EXPAND, "Variable Length Expand"),
          pattern_graph_(pattern_graph),
          start_(start),
          neighbor_(neighbor),
          relp_(relp),
          min_hop_(relp->MinHop()),
          max_hop_(relp->MaxHop()),
          hop_(0),
          eits_(std::vector<std::unique_ptr<graphdb::EdgeIterator>>(
              max_hop_ < 0 ? 0 : max_hop_)) {
        modifies.emplace_back(neighbor_->Alias());
        modifies.emplace_back(relp_->Alias());
        auto &sym_tab = pattern_graph->symbol_table;
        auto sit = sym_tab.symbols.find(start_->Alias());
        auto dit = sym_tab.symbols.find(neighbor_->Alias());
        auto rit = sym_tab.symbols.find(relp_->Alias());
        CYPHER_THROW_ASSERT(sit != sym_tab.symbols.end() &&
                            dit != sym_tab.symbols.end() &&
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
            /* Most of the time, the start_it is definitely valid after child's
             * Consume returns OK, except when the child is an OPTIONAL
             * operation.  */
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
        return fmt::format(
            "{}({}) [{} {}*{}..{} {}]", name, "All", start_->Alias(), towards,
            std::to_string(min_hop_), std::to_string(max_hop_),
            neighbor_->Alias());
    }

    [[nodiscard]] Node *GetStartNode() const { return start_; }
    [[nodiscard]] Node *GetNeighborNode() const { return neighbor_; }
    [[nodiscard]] Relationship *GetRelationship() const { return relp_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
