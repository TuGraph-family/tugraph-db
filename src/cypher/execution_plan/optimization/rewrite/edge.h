#pragma once
#include "node.h"
#include <vector>
#include "parser/data_typedef.h"
namespace rewrite_cypher {
class Node;

class Edge {
 public:
    size_t m_id;
    size_t m_source_id;
    size_t m_target_id;
    std::set<int> m_labels;
    parser::LinkDirection m_direction;
    Edge(){};
    Edge(size_t id, size_t source_id, size_t target_id, std::set<int> labels,
         parser::LinkDirection direction) {
        m_source_id = source_id;
        m_target_id = target_id;
        m_labels = labels;
        m_id = id;
        m_direction = direction;
    }

    ~Edge(){};
};

};  // namespace rewrite_cypher