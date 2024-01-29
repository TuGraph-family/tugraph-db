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
#include <vector>
#include "cypher/execution_plan/optimization/rewrite/edge.h"

namespace cypher::rewrite {

class Edge;

class Node {
 private:
 public:
    size_t m_id;
    int m_label;
    std::vector<size_t> m_outedges;
    std::vector<size_t> m_inedges;
    std::vector<size_t> m_undirectededges;
    Node() {}
    Node(size_t id, int label) {
        m_id = id;
        m_label = label;
    }
    ~Node() {}
};
};  // namespace cypher::rewrite
