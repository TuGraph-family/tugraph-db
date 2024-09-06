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

#include "fma-common/type_traits.h"

#include "cypher/graph/node.h"
#include "cypher/graph/relationship.h"
#include "parser/data_typedef.h"
#include "parser/symbol_table.h"

namespace cypher {

// triple: <start_node, relationship, neighbor_node>
typedef std::vector<std::tuple<NodeID, RelpID, NodeID>> EXPAND_STEPS;

class PatternGraph {
    std::vector<Node> _nodes;
    std::vector<Relationship> _relationships;
    NodeID _next_nid;
    RelpID _next_rid;
    std::unordered_map<std::string, NodeID> _node_map;
    std::unordered_map<std::string, RelpID> _relp_map;
    // for relationship uniqueness
    lgraph::EuidHashSet _visited_edges;

    bool _IsHanging(NodeID id, bool ignore_created) const {
        const auto &node = GetNode(id);
        for (auto rr : node.RhsRelps()) {
            auto &r = GetRelationship(rr);
            // TODO(anyone) think about this again
            if (!ignore_created || (r.derivation_ != Relationship::CREATED &&
                                    GetNode(r.Rhs()).derivation_ != Node::CREATED &&
                                    r.derivation_ != Relationship::MERGED &&
                                    GetNode(r.Rhs()).derivation_ != Node::MERGED)) {
                return false;
            }
        }
        for (auto lr : node.LhsRelps()) {
            auto &r = GetRelationship(lr);
            if (!ignore_created || (r.derivation_ != Relationship::CREATED &&
                                    GetNode(r.Lhs()).derivation_ != Node::CREATED &&
                                    r.derivation_ != Relationship::MERGED &&
                                    GetNode(r.Rhs()).derivation_ != Node::MERGED)) {
                return false;
            }
        }
        return true;
    }

    void _CollectExpandStepsByDFS(NodeID start, bool ignore_created, EXPAND_STEPS &expand_steps);

    DISABLE_COPY(PatternGraph);

 public:
    SymbolTable symbol_table;

 public:
    PatternGraph();

    PatternGraph(PatternGraph &&rhs)
        : _nodes(std::move(rhs._nodes)),
          _relationships(std::move(rhs._relationships)),
          _next_nid(rhs._next_nid),
          _next_rid(rhs._next_rid),
          _node_map(std::move(rhs._node_map)),
          _relp_map(std::move(rhs._relp_map)),
          _visited_edges(std::move(rhs._visited_edges)),
          symbol_table(std::move(rhs.symbol_table)) {}

    Node &GetNode(NodeID id);

    const Node &GetNode(NodeID id) const {
        if ((size_t)id >= _nodes.size()) return EmptyNode();
        return _nodes[id];
    }

    Node &GetNode(const std::string &alias);

    const Node &GetNode(const std::string &alias) const;

    std::vector<Node> &GetNodes();

    const std::vector<Node> &GetNodes() const { return _nodes; }

    Relationship &GetRelationship(RelpID id);

    const Relationship &GetRelationship(RelpID id) const;

    Relationship &GetRelationship(const std::string &alias);

    const Relationship &GetRelationship(const std::string &alias) const;

    std::vector<Relationship> &GetRelationships() { return _relationships; }

    const std::vector<Relationship> &GetRelationships() const { return _relationships; }

    lgraph::EuidHashSet &VisitedEdges() { return _visited_edges; }

    NodeID AddNode(const std::string &label, const std::string &alias, Node::Derivation derivation);

    NodeID AddNode(const std::string &label, const std::string &alias, const Property &prop_filter,
                   Node::Derivation derivation);

    NodeID AddNode(Node *node) {
        NodeID nid = _next_nid;
        _nodes.emplace_back(nid, node->Label(), node->Alias(), node->Prop(), node->derivation_);
        _node_map.emplace(node->Alias(), nid);
        _next_nid++;
        return nid;
    }

    RelpID AddRelationship(const std::set<std::string> &types, NodeID lhs, NodeID rhs,
                           parser::LinkDirection direction, const std::string &alias,
                           Relationship::Derivation derivation, parser::Expression properties,
                           std::vector<std::tuple<std::string, geax::frontend::Expr*>>
                           geax_properties = {});

    RelpID AddRelationship(const std::set<std::string> &types, NodeID lhs, NodeID rhs,
                           parser::LinkDirection direction, const std::string &alias, int min_hop,
                           int max_hop, Relationship::Derivation derivation,
                           parser::Expression properties,
                           std::vector<std::tuple<std::string, geax::frontend::Expr*>>
                           geax_properties = {});

    RelpID AddRelationship(Relationship *relp) {
        return AddRelationship(relp->Types(), relp->Lhs(), relp->Rhs(), relp->direction_,
                               relp->Alias(), relp->MinHop(), relp->MaxHop(), relp->derivation_,
                               relp->Properties(), relp->GeaxProperties());
    }

    NodeID BuildNode(const parser::TUP_NODE_PATTERN &node_pattern, Node::Derivation derivation);

    RelpID BuildRelationship(const parser::TUP_RELATIONSHIP_PATTERN &relp_pattern, NodeID lhs,
                             NodeID rhs, Relationship::Derivation derivation);

    std::vector<EXPAND_STEPS> CollectExpandStreams(const std::vector<NodeID> &start_nodes,
                                                   bool ignore_created) {
        std::vector<EXPAND_STEPS> expand_streams;
        for (auto s : start_nodes) {
            auto &node = GetNode(s);
            if (!node.Visited() && node.derivation_ != Node::CREATED &&
                node.derivation_ != Node::MERGED) {
                EXPAND_STEPS expand_stream;
                _CollectExpandStepsByDFS(s, ignore_created, expand_stream);
                if (!expand_stream.empty()) expand_streams.emplace_back(expand_stream);
            }
        }
        return expand_streams;
    }

    std::string DumpGraph() const;

    static inline Node &EmptyNode() {
        static Node empty;
        return empty;
    }

    static inline Relationship &EmptyRelationship() {
        static Relationship empty;
        return empty;
    }
};
}  // namespace cypher
