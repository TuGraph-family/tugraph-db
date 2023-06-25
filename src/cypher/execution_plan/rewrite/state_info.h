

#pragma once
#include <iostream>

namespace rewrite_cypher {

class StateInfo {
 public:
    size_t m_vid;
    size_t m_next_vid;
    size_t m_eid;
    size_t m_direction;
    std::map<size_t, std::set<size_t>> m_id_map;
    StateInfo() {}
    StateInfo(size_t vid, size_t next_vid, size_t eid, size_t direction,
              std::map<size_t, std::set<size_t>>* id_map) {
        m_vid = vid;
        m_next_vid = next_vid;
        m_eid = eid;
        m_direction = direction;
        m_id_map = *id_map;
    }
};

};  // namespace rewrite_cypher