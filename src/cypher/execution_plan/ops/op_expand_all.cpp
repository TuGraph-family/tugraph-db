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
// Created by wt on 7/3/18.
//

#include "cypher/execution_plan/ops/op_expand_all.h"

#include <spdlog/fmt/fmt.h>

#include "common/logger.h"
#include "cypher/arithmetic/arithmetic_expression.h"
#include "cypher/utils/geax_util.h"
#include "geax-front-end/ast/Ast.h"
#include "graphdb/edge_iterator.h"
using namespace graphdb;
namespace cypher {

static void ExtractLabelTree(std::vector<std::string>& labels,
                             const geax::frontend::LabelTree* root) {
    if (typeid(*root) == typeid(geax::frontend::SingleLabel)) {
        labels.emplace_back(((geax::frontend::SingleLabel*)root)->label());
    } else if (typeid(*root) == typeid(geax::frontend::LabelOr)) {
        auto* label_or = (geax::frontend::LabelOr*)root;
        ExtractLabelTree(labels, label_or->left());
        ExtractLabelTree(labels, label_or->right());
    } else {
        CYPHER_TODO();
    }
}

void ExpandAll::ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& properties,
                                        const geax::frontend::ElementFiller* filler) {
    auto& props = filler->predicates();
    //if (props.size() > 1) CYPHER_TODO();
    for (auto p : props) {
        geax::frontend::PropStruct* prop = nullptr;
        checkedCast(p, prop);
        if (!prop) CYPHER_TODO();
        for (auto& property : prop->properties()) {
            ArithExprNode ae(std::get<1>(property), *record->symbol_table);
            auto ent = ae.Evaluate(ctx, *record);
            if (!ent.IsConstant()) CYPHER_TODO();
            properties.emplace(std::get<0>(property), ent.constant);
        }
    }
}

ExpandAll::ExpandAll(PatternGraph *pattern_graph, Node *start, Node *neighbor, Relationship *relp)
        : OpBase(OpType::EXPAND_ALL, "Expand"),
          start_(start),
          neighbor_(neighbor),
          relp_(relp),
          pattern_graph_(pattern_graph) {
        CYPHER_THROW_ASSERT(start && neighbor && relp);
        modifies.emplace_back(neighbor_->Alias());
        modifies.emplace_back(relp_->Alias());
        auto &sym_tab = pattern_graph->symbol_table;
        auto sit = sym_tab.symbols.find(start_->Alias());
        auto nit = sym_tab.symbols.find(neighbor_->Alias());
        auto rit = sym_tab.symbols.find(relp_->Alias());
        CYPHER_THROW_ASSERT(sit != sym_tab.symbols.end() && nit != sym_tab.symbols.end() &&
                            rit != sym_tab.symbols.end());
        expand_into_ = nit->second.scope == SymbolNode::ARGUMENT;
        expand_direction_ = relp_->Undirected()            ? BIDIRECTIONAL
                            : relp_->Src() == start_->ID() ? FORWARD
                                                           : REVERSED;
        start_rec_idx_ = sit->second.id;
        nbr_rec_idx_ = nit->second.id;
        relp_rec_idx_ = rit->second.id;
        state_ = ExpandAllUninitialized;
}

void ExpandAll::_InitializeEdgeIter(RTContext *ctx) {
    auto &types = relp_->Types();
    auto label = neighbor_->Label();
    std::unordered_map<std::string, Value> relp_map;
    ExtractProperties(ctx, relp_map, relp_->ast_node_->filler());
    std::vector<std::string> labels;
    if (neighbor_->ast_node_->filler()->label().has_value()) {
        ExtractLabelTree(labels, neighbor_->ast_node_->filler()->label().value());
    }
    std::unordered_set<std::string> neighbor_labels = {labels.begin(), labels.end()};
    std::unordered_map<std::string, Value> neighbor_properties;
    ExtractProperties(ctx, neighbor_properties, neighbor_->ast_node_->filler());
    if (expand_into_ && !neighbor_->vertex_) {
        CYPHER_TODO();
    }
    switch (expand_direction_) {
    case ExpandTowards::FORWARD:
        if (expand_into_) {
            eit_ = start_->vertex_->NewEdgeIterator(
                EdgeDirection::OUTGOING, std::unordered_set<std::string>{types.begin(), types.end()}, relp_map,
                neighbor_->vertex_.value());
        } else {
            eit_ = start_->vertex_->NewEdgeIterator(
                EdgeDirection::OUTGOING,
                std::unordered_set<std::string>{types.begin(), types.end()},
                relp_map, neighbor_labels, neighbor_properties);
        }
        break;
    case ExpandTowards::REVERSED:
        if (expand_into_) {
            eit_ = start_->vertex_->NewEdgeIterator(
                EdgeDirection::INCOMING, std::unordered_set<std::string>{types.begin(), types.end()}, relp_map,
                neighbor_->vertex_.value());
        } else {
            eit_ = start_->vertex_->NewEdgeIterator(
                EdgeDirection::INCOMING,
                std::unordered_set<std::string>{types.begin(), types.end()},
                relp_map, neighbor_labels, neighbor_properties);
        }
        break;
    case ExpandTowards::BIDIRECTIONAL:
        if (expand_into_) {
            eit_ = start_->vertex_->NewEdgeIterator(
                EdgeDirection::BOTH, std::unordered_set<std::string>{types.begin(), types.end()}, relp_map,
                neighbor_->vertex_.value());
        } else {
            eit_ = start_->vertex_->NewEdgeIterator(
                EdgeDirection::BOTH,
                std::unordered_set<std::string>{types.begin(), types.end()},
                relp_map, neighbor_labels, neighbor_properties);
        }
        break;
    }
}

bool ExpandAll::ExpandValid() const {
    while (eit_->Valid() &&
           pattern_graph_->VisitedEdges().count(eit_->GetEdge().GetId())) {
        eit_->Next();
    }
    if (!eit_->Valid()) return false;
    relp_->edge_ = eit_->GetEdge();
    neighbor_->vertex_ = relp_->edge_->GetOtherEnd(start_->vertex_->GetId());
    pattern_graph_->VisitedEdges().insert(eit_->GetEdge().GetId());
    return true;
}

OpBase::OpResult ExpandAll::Next(RTContext *ctx) {
    // Reset iterators
    if (state_ == ExpandAllResetted) {
        /* Start node iterator may be invalid, such as when the start is an argument
         * produced by OPTIONAL MATCH.  */
        if (!start_->vertex_) return OP_REFRESH;
        _InitializeEdgeIter(ctx);
        if (!ExpandValid()) {
            return OP_REFRESH;
        }
        state_ = ExpandAllConsuming;
        return OP_OK;
    }
    pattern_graph_->VisitedEdges().erase(eit_->GetEdge().GetId());
    eit_->Next();
    return ExpandValid() ? OP_OK : OP_REFRESH;
}

OpBase::OpResult ExpandAll::RealConsume(RTContext *ctx) {
    CYPHER_THROW_ASSERT(!children.empty());
    auto child = children[0];
    while (state_ == ExpandAllUninitialized || Next(ctx) == OP_REFRESH) {
        auto res = child->Consume(ctx);
        state_ = ExpandAllResetted;
        if (res != OP_OK) {
            /* When consume after the stream is DEPLETED, make sure
             * the result always be DEPLETED.  */
            state_ = ExpandAllUninitialized;
            return res;
        }
        /* Most of the time, the start_it is definitely valid after child's Consume
         * returns OK, except when the child is an OPTIONAL operation.  */
    }
    return OP_OK;
}

OpBase::OpResult ExpandAll::ResetImpl(bool complete) {
    /* TODO(anyone) optimize, the apply operator need reset rhs stream completely,
         * while the cartesian product doesn't.
         * e.g.:
         * match (n:Person {name:'Vanessa Redgrave'})-->(m) with m as m1
         * match (n:Person {name:'Vanessa Redgrave'})<--(m) return m as m2, m1
         * */
    /* reset modifies */
    //eit_->FreeIter();
    eit_.reset();
    //if (!expand_into_) neighbor_->PushVid(-1);
    //pattern_graph_->VisitedEdges().Erase(*eit_);
    state_ = ExpandAllUninitialized;
    return OP_OK;
}

std::string ExpandAll::ToString() const {
    auto towards = expand_direction_ == FORWARD    ? "-->"
                   : expand_direction_ == REVERSED ? "<--"
                                                   : "--";
    return fmt::format(
        "{}({}) [{} {} {}]", name, expand_into_ ? "Into" : "All", start_->Alias(), towards,
        neighbor_->Alias());
}

}