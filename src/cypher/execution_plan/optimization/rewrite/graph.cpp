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
// Created by liyunlong2000 on 23-07-19.
//
#include "cypher/execution_plan/optimization/rewrite/graph.h"

namespace cypher::rewrite {
void Graph::AddNode(size_t id, int label_num) { m_nodes.push_back(rewrite::Node(id, label_num)); }

void Graph::AddEdge(size_t id, size_t source_id, size_t target_id, std::set<int> labels,
                    parser::LinkDirection direction) {
    // Edge edge(id,source_id,target_id,label_num);
    if (direction == parser::LinkDirection::LEFT_TO_RIGHT) {
        m_edges.push_back(Edge(id, source_id, target_id, labels, direction));
        m_nodes[source_id].m_outedges.push_back(id);
        m_nodes[target_id].m_inedges.push_back(id);
    } else if (direction == parser::LinkDirection::RIGHT_TO_LEFT) {
        m_edges.push_back(
            Edge(id, target_id, source_id, labels, parser::LinkDirection::RIGHT_TO_LEFT));
        m_nodes[source_id].m_inedges.push_back(id);
        m_nodes[target_id].m_outedges.push_back(id);
    } else {
        m_edges.push_back(Edge(id, source_id, target_id, labels, direction));
        m_nodes[source_id].m_undirectededges.push_back(id);
        m_nodes[target_id].m_undirectededges.push_back(id);
    }
}

void Graph::PrintGraph() {
    for (Node node : m_nodes) {
        LOG_INFO() << "Node id:" << node.m_id;
        LOG_INFO() << "In edges:";
        for (size_t eid : node.m_inedges) {
            Edge& edge = m_edges[eid];
            LOG_INFO() << edge.m_id << "-" << edge.m_source_id << ".";
        }
        LOG_INFO();

        LOG_INFO() << "Out edges:";
        for (size_t eid : node.m_outedges) {
            Edge& edge = m_edges[eid];
            LOG_INFO() << edge.m_id << "-" << edge.m_target_id << ".";
        }
        LOG_INFO();

        LOG_INFO() << "Undirection edges:";
        for (size_t eid : node.m_undirectededges) {
            Edge& edge = m_edges[eid];
            LOG_INFO() << edge.m_id << "-" << edge.m_direction << ".";
        }
        LOG_INFO();
    }
}
};  // namespace cypher::rewrite
