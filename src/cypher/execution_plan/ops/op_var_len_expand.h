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
// Modified by bxj on 24-3-30.
//
#pragma once

#include "cypher/execution_plan/ops/op.h"
#include "filter/filter.h"

namespace cypher {

struct DfsState {
    // current node id
    lgraph::VertexId currentNodeId;
    // current index for current node
    lgraph::EIter *currentEit;
    // level, or path length
    int level;
    // number of neighbors of this node
    int count;
    // whether the eiter need Next()
    bool needNext;

    DfsState(RTContext *ctx, lgraph::VertexId id, int level, cypher::Relationship *relp,
             ExpandTowards expand_direction, bool needNext, bool isMaxHop);
};

class Predicate {
 public:
    virtual bool eval(std::vector<lgraph::EIter> &eits) = 0;
    virtual ~Predicate() = default;
};

class HeadPredicate : public Predicate {
 private:
    // operator
    lgraph::CompareOp op;
    // operand, on the right
    FieldData operand;

 public:
    HeadPredicate(lgraph::CompareOp op, FieldData operand) : op(op), operand(operand) {}
    bool eval(std::vector<lgraph::EIter> &eits) override;
};

class LastPredicate : public Predicate {
 private:
    // operator
    lgraph::CompareOp op;
    // operand, on the right
    FieldData operand;

 public:
    LastPredicate(lgraph::CompareOp op, FieldData operand) : op(op), operand(operand) {}
    bool eval(std::vector<lgraph::EIter> &eits) override;
};

class IsAscPredicate : public Predicate {
 public:
    IsAscPredicate() {}
    bool eval(std::vector<lgraph::EIter> &eits) override;
};

class IsDescPredicate : public Predicate {
 public:
    IsDescPredicate() {}
    bool eval(std::vector<lgraph::EIter> &eits) override;
};

class MaxInListPredicate : public Predicate {
 private:
    lgraph::CompareOp op;
    FieldData operand;

 public:
    MaxInListPredicate(lgraph::CompareOp op, FieldData operand) : op(op), operand(operand) {}
    bool eval(std::vector<lgraph::EIter> &eits) override;
};

class MinInListPredicate : public Predicate {
 private:
    lgraph::CompareOp op;
    FieldData operand;

 public:
    MinInListPredicate(lgraph::CompareOp op, FieldData operand) : op(op), operand(operand) {}
    bool eval(std::vector<lgraph::EIter> &eits) override;
};

/* Variable Length Expand */
class VarLenExpand : public OpBase {
    bool PerNodeLimit(RTContext *ctx, size_t count);

    bool NextWithFilter(RTContext *ctx);

    void PushFilter(std::shared_ptr<lgraph::Filter> filter);

    // save 6 types of predicates
    std::vector<std::unique_ptr<Predicate>> predicates;
    // add predicate to the vector
    void addPredicate(std::unique_ptr<Predicate> p);

    // stack for DFS
    std::vector<DfsState> stack;

    // this flag decides whether need to pop relp_->Path
    bool needPop;

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
    ExpandTowards expand_direction_;

    // edge_filter_ is temp used
    std::shared_ptr<lgraph::Filter> edge_filter_ = nullptr;

    VarLenExpand(PatternGraph *pattern_graph, Node *start, Node *neighbor, Relationship *relp);

    void PushDownEdgeFilter(std::shared_ptr<lgraph::Filter> edge_filter);

    OpResult Initialize(RTContext *ctx) override;

    OpResult RealConsume(RTContext *ctx) override;

    OpResult ResetImpl(bool complete) override;

    std::string ToString() const override;

    Node *GetStartNode() const { return start_; }
    Node *GetNeighborNode() const { return neighbor_; }
    Relationship *GetRelationship() const { return relp_; }

    CYPHER_DEFINE_VISITABLE()

    CYPHER_DEFINE_CONST_VISITABLE()
};
}  // namespace cypher
