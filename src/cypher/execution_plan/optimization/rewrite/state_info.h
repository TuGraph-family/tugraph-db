/**
 * Copyright 2024 AntGroup CO., Ltd.
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
#pragma once
#include <iostream>

namespace cypher::rewrite {

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

};  // namespace cypher::rewrite
