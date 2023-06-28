#pragma once
#include "edge.h"
#include <vector>

namespace rewrite_cypher {

class Edge;

class Node {
 private:
 public:
    size_t m_id;
    int m_label;
    std::vector<size_t> m_outedges;
    std::vector<size_t> m_inedges;
    std::vector<size_t> m_undirectededges;
    Node(){};
    Node(size_t id, int label) {
        m_id = id;
        m_label = label;
    }
    ~Node(){};
};
};  // namespace rewrite_cypher