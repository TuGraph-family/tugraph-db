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

#include <iostream>
#include <stack>
#include "cypher/graph/graph.h"
#include "cypher/cypher_exception.h"
#include <spdlog/fmt/fmt.h>

namespace cypher {

PatternGraph::PatternGraph() : _next_nid(0), _next_rid(0) {}

void PatternGraph::_CollectExpandStepsByDFS(NodeID start, bool ignore_created,
                                            EXPAND_STEPS &expand_steps) {
    GetNode(start).Visited() = true;
    if (_IsHanging(start, ignore_created)) {
        expand_steps.emplace_back(start, -1, -1);
        return;
    }
    std::stack<NodeID> sn;
    sn.push(start);
    while (!sn.empty()) {
        auto &curr = GetNode(sn.top());
        // find the first unvisited relationship
        RelpID relp = -1;
        NodeID neighbor;
        for (auto rr : curr.RhsRelps()) {
            auto &r = GetRelationship(rr);
            auto &nbr = GetNode(r.Rhs());
            if (nbr.ID() == curr.ID()) CYPHER_TODO();  // self-loop!
            // TODO(anyone) think about this again
            if (!nbr.Visited() && (!ignore_created || (nbr.derivation_ != Node::CREATED &&
                                                       nbr.derivation_ != Node::MERGED))) {
                relp = r.ID();
                neighbor = nbr.ID();
                break;
            }
        }
        for (auto lr : curr.LhsRelps()) {
            auto &r = GetRelationship(lr);
            auto &nbr = GetNode(r.Lhs());
            if (nbr.ID() == curr.ID()) CYPHER_TODO();  // self-loop!
            if (!nbr.Visited() &&
                (!ignore_created ||
                 (nbr.derivation_ != Node::CREATED && nbr.derivation_ != Node::MERGED)) &&
                (relp < 0 || (relp >= 0 && r.ID() < relp))) {
                relp = r.ID();
                neighbor = nbr.ID();
                break;
            }
        }
        if (relp < 0) {
            sn.pop();
            continue;
        }
        expand_steps.emplace_back(curr.ID(), relp, neighbor);
        sn.push(neighbor);
        GetNode(neighbor).Visited() = true;
    }
}

Node &PatternGraph::GetNode(NodeID id) {
    if ((size_t)id >= _nodes.size()) return EmptyNode();
    return _nodes[id];
}

Node &PatternGraph::GetNode(const std::string &alias) {
    auto it = _node_map.find(alias);
    return it == _node_map.end() ? EmptyNode() : _nodes[it->second];
}

const Node &PatternGraph::GetNode(const std::string &alias) const {
    auto it = _node_map.find(alias);
    return it == _node_map.end() ? EmptyNode() : _nodes[it->second];
}

std::vector<Node> &PatternGraph::GetNodes() { return _nodes; }

Relationship &PatternGraph::GetRelationship(RelpID id) {
    if ((size_t)id >= _relationships.size()) return EmptyRelationship();
    return _relationships[id];
}

const Relationship &PatternGraph::GetRelationship(RelpID id) const {
    if ((size_t)id >= _relationships.size()) return EmptyRelationship();
    return _relationships[id];
}

Relationship &PatternGraph::GetRelationship(const std::string &alias) {
    auto it = _relp_map.find(alias);
    return it == _relp_map.end() ? EmptyRelationship() : _relationships[it->second];
}

const Relationship &PatternGraph::GetRelationship(const std::string &alias) const {
    auto it = _relp_map.find(alias);
    return it == _relp_map.end() ? EmptyRelationship() : _relationships[it->second];
}

NodeID PatternGraph::AddNode(const std::string &label, const std::string &alias,
                             Node::Derivation derivation) {
    NodeID nid = _next_nid;
    _nodes.emplace_back(nid, label, alias, derivation);
    _node_map.emplace(alias, nid);
    _next_nid++;
    return nid;
}

RelpID PatternGraph::AddRelationship(const std::set<std::string> &types, NodeID lhs, NodeID rhs,
                                     parser::LinkDirection direction, const std::string &alias,
                                     Relationship::Derivation derivation) {
    return AddRelationship(types, {}, lhs, rhs, direction, alias, -1, -1,
                           derivation);
}

RelpID PatternGraph::AddRelationship(const std::set<std::string> &types, geax::frontend::Edge* ast_node_, NodeID lhs, NodeID rhs,
                                     parser::LinkDirection direction, const std::string &alias,
                                     int min_hop, int max_hop,
                                     Relationship::Derivation derivation) {
    RelpID rid = _next_rid;
    _relationships.emplace_back(rid, types, ast_node_, lhs, rhs, direction, alias, min_hop, max_hop,
                                derivation);
    _relp_map.emplace(alias, rid);
    _next_rid++;
    auto r = GetNode(lhs).AddRelp(rid, true);
    CYPHER_THROW_ASSERT(r);
    r = GetNode(rhs).AddRelp(rid, false);
    CYPHER_THROW_ASSERT(r);
    return rid;
}

std::string PatternGraph::DumpGraph() const {
    std::string line = "Current Pattern Graph:\n";
    for (auto &n : _nodes) {
        auto derivation =
                            n.derivation_ == Node::CREATED    ? "(CREATED)"
                          : n.derivation_ == Node::MATCHED   ? "(MATCHED)"
                          : n.derivation_ == Node::MERGED   ? "(MERGED)"
                          : n.derivation_ == Node::ARGUMENT ? "(ARGUMENT)"
                          : n.derivation_ == Node::YIELD ? "(YIELD)"
                                                            : "(UNKNOWN)";
        line.append(fmt::format("N[{}] {}:{} {}\n", std::to_string(n.ID()),
                                                        n.Alias(), n.Label(), derivation));
    }
    for (auto &r : _relationships) {
        auto direction = r.direction_ == parser::LEFT_TO_RIGHT   ? "-->"
                         : r.direction_ == parser::RIGHT_TO_LEFT ? "<--"
                                                                 : "--";
        std::string types = fmt::format("{}", fmt::join(r.Types(), ","));
        auto derivation = r.derivation_ == Relationship::CREATED    ? "(CREATED)"
                          : r.derivation_ == Relationship::MATCHED   ? "(MATCHED)"
                          : r.derivation_ == Relationship::MERGED   ? "(MERGED)"
                          : r.derivation_ == Relationship::ARGUMENT ? "(ARGUMENT)"
                                                                    : "(UNKNOWN)";
        line.append(fmt::format(
            "R[{} {} {}] {}:{} {}\n", std::to_string(r.Lhs()), direction, std::to_string(r.Rhs()),
            r.Alias(), types, derivation));
    }
    for (auto &[symbol, symbol_node] : symbol_table.symbols) {
        line.append(fmt::format(
            "Symbol: [{}] type({}), scope({}), symbol_id({})\n", symbol,
            cypher::SymbolNode::to_string(symbol_node.type),
            cypher::SymbolNode::to_string(symbol_node.scope), symbol_node.id));
    }
    if (_nodes.empty() && _relationships.empty()) line.append("(EMPTY GRAPH)\n");
    return line;
}

}  // namespace cypher
