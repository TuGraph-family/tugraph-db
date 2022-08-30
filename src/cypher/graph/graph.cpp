/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 6/12/18.
//

#include <iostream>
#include <stack>
#include "fma-common/utils.h"
#include "parser/clause.h"
#include "graph.h"

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
            // todo: think about this again
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

NodeID PatternGraph::AddNode(const std::string &label, const std::string &alias,
                             const Property &prop_filter, Node::Derivation derivation) {
    NodeID nid = _next_nid;
    _nodes.emplace_back(nid, label, alias, prop_filter, derivation);
    _node_map.emplace(alias, nid);
    _next_nid++;
    return nid;
}

RelpID PatternGraph::AddRelationship(const std::set<std::string> &types, NodeID lhs, NodeID rhs,
                                     parser::LinkDirection direction, const std::string &alias,
                                     Relationship::Derivation derivation) {
    return AddRelationship(types, lhs, rhs, direction, alias, -1, -1, derivation);
}

RelpID PatternGraph::AddRelationship(const std::set<std::string> &types, NodeID lhs, NodeID rhs,
                                     parser::LinkDirection direction, const std::string &alias,
                                     int min_hop, int max_hop,
                                     Relationship::Derivation derivation) {
    RelpID rid = _next_rid;
    _relationships.emplace_back(rid, types, lhs, rhs, direction, alias, min_hop, max_hop,
                                derivation);
    _relp_map.emplace(alias, rid);
    _next_rid++;
    auto r = GetNode(lhs).AddRelp(rid, true);
    CYPHER_THROW_ASSERT(r);
    r = GetNode(rhs).AddRelp(rid, false);
    CYPHER_THROW_ASSERT(r);
    return rid;
}

static void ExtractNodePattern(const parser::TUP_NODE_PATTERN &node_pattern, std::string &label) {
    auto &node_labels = std::get<1>(node_pattern);
    if (!node_labels.empty()) label = node_labels[0];
}

static void ExtractNodePattern(const parser::TUP_NODE_PATTERN &node_pattern, std::string &label,
                               cypher::Property &prop) {
    ExtractNodePattern(node_pattern, label);
    auto &properties = std::get<2>(node_pattern);
    auto &expr_prop = std::get<0>(properties);
    if (expr_prop.type == parser::Expression::MAP) {
        if (expr_prop.Map().size() > 1) CYPHER_TODO();
        for (auto &m : expr_prop.Map()) {
            prop.field = m.first;
            if (m.second.type == parser::Expression::PARAMETER) {
                prop.type = Property::PARAMETER;
                prop.value_alias = m.second.String();
            } else if (m.second.type == parser::Expression::VARIABLE) {
                prop.type = Property::VARIABLE;
                prop.value_alias = m.second.String();
            } else {
                prop.type = Property::VALUE;
                prop.value = MakeFieldData(m.second);
                if (prop.value.is_null()) {
                    throw lgraph::InputError("Invalid expression: " + m.second.ToString());
                }
            }
        }
    }
}

NodeID PatternGraph::BuildNode(const parser::TUP_NODE_PATTERN &node_pattern,
                               Node::Derivation derivation) {
    const std::string &node_variable = std::get<0>(node_pattern);
    auto &node = GetNode(node_variable);
    std::string label;
    Property prop;
    if (node.Empty()) {
        if (derivation == Node::CREATED) {
            ExtractNodePattern(node_pattern, label);
            return AddNode(label, node_variable, derivation);
        } else {
            ExtractNodePattern(node_pattern, label, prop);
            return AddNode(label, node_variable, prop, derivation);
        }
    } else {
        // if the node already exists, update the node
        ExtractNodePattern(node_pattern, label, prop);
        node.Set(label, prop);
        return node.ID();
    }
}

RelpID PatternGraph::BuildRelationship(const parser::TUP_RELATIONSHIP_PATTERN &relp_pattern,
                                       NodeID lhs, NodeID rhs,
                                       Relationship::Derivation derivation) {
    auto direction = std::get<0>(relp_pattern);
    auto &relp_var = std::get<0>(std::get<1>(relp_pattern));
    auto &relp_detail = std::get<1>(relp_pattern);
    auto &relp_types = std::get<1>(relp_detail);
    auto &range = std::get<2>(relp_detail);
    // convert vector to set
    std::set<std::string> r_types(relp_types.begin(), relp_types.end());
    auto &relp = GetRelationship(relp_var);
    if (!relp.Empty()) CYPHER_TODO();
    return AddRelationship(r_types, lhs, rhs, direction, relp_var, range[0], range[1], derivation);
}

std::string PatternGraph::DumpGraph() const {
    std::string line = "Current Pattern Graph:\n";
    for (auto &n : _nodes) {
        auto derivation = n.derivation_ == Node::CREATED    ? "(C)"
                          : n.derivation_ == Node::MERGED   ? "(U)"
                          : n.derivation_ == Node::ARGUMENT ? "(A)"
                                                            : "(M)";
        line.append(fma_common::StringFormatter::Format("N[{}] {}:{} {}\n", std::to_string(n.ID()),
                                                        n.Alias(), n.Label(), derivation));
    }
    for (auto &r : _relationships) {
        auto direction = r.direction_ == parser::LEFT_TO_RIGHT   ? "-->"
                         : r.direction_ == parser::RIGHT_TO_LEFT ? "<--"
                                                                 : "--";
        auto types = fma_common::ToString(r.Types());
        auto derivation = r.derivation_ == Relationship::CREATED    ? "(C)"
                          : r.derivation_ == Relationship::MERGED   ? "(U)"
                          : r.derivation_ == Relationship::ARGUMENT ? "(A)"
                                                                    : "(M)";
        line.append(fma_common::StringFormatter::Format(
            "R[{} {} {}] {}:{} {}\n", std::to_string(r.Lhs()), direction, std::to_string(r.Rhs()),
            r.Alias(), types, derivation));
    }
    if (_nodes.empty() && _relationships.empty()) line.append("(EMPTY GRAPH)");
    return line;
}

}  // namespace cypher
