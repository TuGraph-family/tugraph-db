#include "graph.h"

namespace rewrite_cypher {
void Graph::AddNode(size_t id, int label_num) {
    m_nodes.push_back(rewrite_cypher::Node(id, label_num));
}

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
        std::cout << "Node id:" << node.m_id << std::endl;
        std::cout << "In edges:" << std::endl;
        for (size_t eid : node.m_inedges) {
            Edge& edge = m_edges[eid];
            std::cout << edge.m_id << "-" << edge.m_source_id << ".";
        }
        std::cout << std::endl;

        std::cout << "Out edges:" << std::endl;
        for (size_t eid : node.m_outedges) {
            Edge& edge = m_edges[eid];
            std::cout << edge.m_id << "-" << edge.m_target_id << ".";
        }
        std::cout << std::endl;

        std::cout << "Undirection edges:" << std::endl;
        for (size_t eid : node.m_undirectededges) {
            Edge& edge = m_edges[eid];
            std::cout << edge.m_id << "-" << edge.m_direction << ".";
        }
        std::cout << std::endl;
    }
}
};  // namespace rewrite_cypher