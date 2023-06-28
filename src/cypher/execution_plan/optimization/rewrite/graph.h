#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "node.h"
#include "edge.h"

#include "cypher/execution_plan/ops/op.h"
namespace rewrite_cypher {

class Graph {
 public:
    std::vector<rewrite_cypher::Node> m_nodes;
    std::vector<rewrite_cypher::Edge> m_edges;

    Graph(/* args */){};
    ~Graph(){};
    void AddNode(size_t id, int label);

    void AddEdge(size_t id, size_t source_id, size_t target_id, std::set<int> labels,
                 parser::LinkDirection direction);

    void PrintGraph();
};

};  // namespace rewrite_cypher