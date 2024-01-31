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

//
// Created by wt on 6/12/18.
//

#include "cypher/graph/node.h"
#include "execution_plan/runtime_context.h"

namespace cypher {

Node::Node() : id_(-1), derivation_(UNKNOWN) {}

Node::Node(cypher::NodeID id, const std::string &label, const std::string &alias,
           Derivation derivation)
    : id_(id), label_(label), alias_(alias), derivation_(derivation) {}

Node::Node(cypher::NodeID id, const std::string &label, const std::string &alias,
           const Property &prop, Derivation derivation)
    : Node(id, label, alias, derivation) {
    property_ = prop;
}

NodeID Node::ID() const { return id_; }

const std::string &Node::Label() const { return label_; }

const std::string &Node::Alias() const { return alias_; }

size_t Node::RhsDegree() const { return rhs_relps_.size(); }

size_t Node::LhsDegree() const { return lhs_relps_.size(); }

const std::vector<RelpID> &Node::RhsRelps() const { return rhs_relps_; }

const std::vector<RelpID> &Node::LhsRelps() const { return lhs_relps_; }

const Property &Node::Prop() const { return property_; }

lgraph::VIter *Node::ItRef() { return &it_; }

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

void Node::Set(const std::string &label, const cypher::Property &property) {
    if (!label.empty()) label_ = label;
    // TODO(anyone) multi pair
    if (property.type != Property::NUL) property_ = property;
}

bool Node::IsValidAfterMaterialize(RTContext *ctx) {
    if (!it_.IsValid() && vid_ >= 0) {
        it_.Initialize(ctx->txn_->GetTxn().get(), lgraph::VIter::VERTEX_ITER, vid_);
    }
    return it_.IsValid();
}
}  // namespace cypher
