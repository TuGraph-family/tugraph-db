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
#include <optional>
#include "cypher/graph/common.h"
#include "graphdb/graph_entity.h"
#include "geax-front-end/ast/clause/Node.h"
namespace cypher {
class RTContext;

class Node {
    NodeID id_;
    std::string label_;
    std::string alias_;
    std::vector<RelpID> rhs_relps_;  // rhs relationship ids
    std::vector<RelpID> lhs_relps_;  // lhs relationship ids
    bool visited_ = false;

 public:
    geax::frontend::Node* ast_node_;
    std::optional<graphdb::Vertex> vertex_;
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
    Node(const Node &) = default;

    NodeID ID() const;

    const std::string &Label() const;

    void SetLabel(const std::string &label) { label_ = label; }

    const std::string &Alias() const;

    bool &Visited() { return visited_; }

    const std::vector<RelpID> &RhsRelps() const;

    const std::vector<RelpID> &LhsRelps() const;

    bool Empty() const;

    bool AddRelp(RelpID rid, bool is_rhs_relp);

    void SetAlias(const std::string &alias) {
        alias_ = alias;
    }
};
}  // namespace cypher
