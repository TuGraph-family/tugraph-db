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

#include "cypher/graph/node.h"

namespace cypher {

Node::Node() : id_(-1), derivation_(UNKNOWN) {}

Node::Node(cypher::NodeID id, const std::string &label, const std::string &alias,
           Derivation derivation)
    : id_(id), label_(label), alias_(alias), derivation_(derivation) {}

NodeID Node::ID() const { return id_; }

const std::string &Node::Label() const { return label_; }

const std::string &Node::Alias() const { return alias_; }

const std::vector<RelpID> &Node::RhsRelps() const { return rhs_relps_; }

const std::vector<RelpID> &Node::LhsRelps() const { return lhs_relps_; }

bool Node::Empty() const { return (id_ < 0); }

bool Node::AddRelp(cypher::RelpID rid, bool is_rhs_relp) {
    if (is_rhs_relp) {
        for (auto r : rhs_relps_) {
            if (r == rid) return false;
        }
        rhs_relps_.emplace_back(rid);
    } else {
        for (auto r : lhs_relps_) {
            if (r == rid) return false;
        }
        lhs_relps_.emplace_back(rid);
    }
    return true;
}

}  // namespace cypher
