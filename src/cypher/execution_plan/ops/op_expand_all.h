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

#include "cypher/execution_plan/ops/op.h"
#include "cypher/graph/graph.h"

namespace cypher {

/* ExpandAll
 * Expands entire graph,
 * Each node within the graph will be set */
class ExpandAll : public OpBase {
    void _InitializeEdgeIter(RTContext *ctx);
    OpResult Next(RTContext *ctx);
    void ExtractProperties(RTContext *ctx, std::unordered_map<std::string, Value>& properties,
                                      const geax::frontend::ElementFiller* filler);
    bool ExpandValid() const;

 public:
    cypher::Node *start_;         // start node to expand
    cypher::Node *neighbor_;      // neighbor of start node
    cypher::Relationship *relp_;  // relationship to expand
    std::unique_ptr<graphdb::EdgeIterator> eit_;
    int start_rec_idx_;
    int nbr_rec_idx_;
    int relp_rec_idx_;
    cypher::PatternGraph *pattern_graph_;
    bool expand_into_;
    ExpandTowards expand_direction_;

    /* ExpandAllStates
     * Different states in which ExpandAll can be at. */
    enum ExpandAllState {
        ExpandAllUninitialized, /* ExpandAll wasn't initialized it. */
        ExpandAllResetted,      /* ExpandAll was just restarted. */
        ExpandAllConsuming,     /* ExpandAll consuming data. */
    } state_;

    // TODO(anyone) rename expandAll to expand
    ExpandAll(PatternGraph *pattern_graph, Node *start, Node *neighbor, Relationship *relp);

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
        record->values[relp_rec_idx_].type = Entry::RELATIONSHIP;
        record->values[relp_rec_idx_].relationship = relp_;
        return OP_OK;
    }

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool complete) override;

    std::string ToString() const override;

    Node* GetStartNode() const { return start_; }
    Node* GetNeighborNode() const { return neighbor_; }
    Relationship* GetRelationship() const { return relp_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
