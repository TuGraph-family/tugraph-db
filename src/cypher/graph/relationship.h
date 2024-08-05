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
#include <utility>
#include "cypher/graph/common.h"
#include "parser/data_typedef.h"
#include "cypher/filter/iterator.h"
#include "cypher/parser/expression.h"
#include "geax-front-end/ast/expr/Expr.h"

namespace cypher {

class Relationship {
    RelpID id_ = -1;
    std::set<std::string> types_;
    parser::Expression properties_;
    std::vector<std::tuple<std::string, geax::frontend::Expr*>> geax_properties;
    NodeID lhs_ = -1;
    NodeID rhs_ = -1;
    std::string alias_;
    lgraph::EIter it_;
    // Store edge iterators for variable length relationship, which slightly duplicate path_.
    std::vector<lgraph::EIter> its_;

 public:
    parser::LinkDirection direction_ = parser::LinkDirection::UNKNOWN;
    enum Derivation {
        MATCHED,   // referred in match clause
        ARGUMENT,  // unmatched argument
        CREATED,   // unmatched in create clause
        MERGED,    // unmatched relationship in merge clause
        UNKNOWN,
    } derivation_ = UNKNOWN;
    int min_hop_ = -1, max_hop_ = -1;  // for variable length relationship
    Path path_;                        // for variable length relationship

    Relationship();

    Relationship(RelpID id, const std::set<std::string> &types, NodeID lhs, NodeID rhs,
                 parser::LinkDirection direction, const std::string &alias, Derivation derivation);

    Relationship(RelpID id, const std::set<std::string> &types, NodeID src, NodeID dst,
                 parser::LinkDirection direction, const std::string &alias, int min_hop,
                 int max_hop, Derivation derivation);

    Relationship(Relationship &&) = default;

    Relationship(const Relationship &) = delete;

    RelpID ID() const;

    const std::set<std::string> &Types() const;

    const parser::Expression& Properties() { return properties_; }

    const std::vector<std::tuple<std::string, geax::frontend::Expr*>>& GeaxProperties() {
        return geax_properties;
    }

    void SetProperties(parser::Expression props) {
        properties_ = std::move(props);
    }

    void SetGeaxProperties(std::vector<std::tuple<std::string, geax::frontend::Expr*>> props) {
        geax_properties = std::move(props);
    }

    void SetTypes(const std::set<std::string> &types) { types_ = types; }

    NodeID Lhs() const { return lhs_; }

    NodeID Rhs() const { return rhs_; }

    NodeID Src() const;

    NodeID Dst() const;

    const std::string &Alias() const;

    lgraph::EIter *ItRef();

    std::vector<lgraph::EIter> &ItsRef();

    bool Empty() const;

    bool Undirected() const;

    bool VarLen() const;

    int MinHop() const;

    int MaxHop() const;

    void AddType(const std::string &type) { types_.emplace(type); }

    void SetLhs(NodeID lhs) { lhs_ = lhs; }

    void SetRhs(NodeID rhs) { rhs_ = rhs; }

    void SetAlias(const std::string &alias) { alias_ = alias; }

    static bool CheckVarLen(int min_hop, int max_hop);
};
}  // namespace cypher
