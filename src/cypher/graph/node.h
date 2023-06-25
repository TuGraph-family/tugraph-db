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
// Created by wt on 6/12/18.
//
#pragma once

#include <string>
#include <vector>
#include "common.h"

namespace cypher {
struct RTContext;

class Node {
    NodeID id_;
    std::string label_;
    std::string alias_;
    std::vector<RelpID> rhs_relps_;  // rhs relationship ids
    std::vector<RelpID> lhs_relps_;  // lhs relationship ids
    Property property_;              // e.g. {name:'Tom Hanks'}, {name:$name}
    lgraph::VIter it_;
    lgraph::VertexId vid_ = -1;      // the cache vid of iterator
    bool materialized_ = false;
    bool visited_ = false;

 public:
    enum Derivation {
        MATCHED,   // node referred in match clause
        ARGUMENT,  // unmatched node that is argument
        CREATED,   // unmatched node in create clause
        MERGED,    // unmatched node in merge clause
        YIELD,     // node yield from inquery call
        UNKNOWN,
    } derivation_;

    Node();

    Node(NodeID id, const std::string &label, const std::string &alias, Derivation derivation);

    Node(NodeID id, const std::string &label, const std::string &alias, const Property &prop,
         Derivation derivation);

    NodeID ID() const;

    const std::string &Label() const;

    void SetLabel(std::string schema_label) { label_ = schema_label; }

    const std::string &Alias() const;

    bool &Visited() { return visited_; }

    bool Visited() const { return visited_; }

    size_t RhsDegree() const;

    size_t LhsDegree() const;

    const std::vector<RelpID> &RhsRelps() const;

    const std::vector<RelpID> &LhsRelps() const;

    const Property &Prop() const;

    lgraph::VIter *ItRef();

    bool Empty() const;

    bool AddRelp(RelpID rid, bool is_rhs_relp);

    void Set(const std::string &label, const Property &property);

    lgraph::VertexId GetVid() const { return vid_; }

    void SetVid(lgraph::VertexId vid) { vid_ = vid; }

    /* Get node's vid, get vid from the iterator if necessary.
     **/
    lgraph::VertexId PullVid() {
        if (vid_ >= 0) return vid_;
        if (it_.IsValid()) vid_ = it_.GetId();
        return vid_;
    }

    /* Set node's vid, and set the iterator as well.
     **/
    void PushVid(lgraph::VertexId vid) {
        vid_ = vid;
        if (!it_.IsValid()) return;
        if (vid_ < 0) {
            it_.FreeIter();
        } else {
            it_.Goto(vid);
        }
    }

    /* If the node is valid or not, meanwhile materialize the iterator
     * if necessary.
     **/
    bool IsValidAfterMaterialize(RTContext *ctx);
};
}  // namespace cypher
