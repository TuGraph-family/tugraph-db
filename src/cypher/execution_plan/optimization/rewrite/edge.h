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
// Created by seijiang on 23-07-19.
//
#pragma once
#include <vector>
#include "cypher/execution_plan/optimization/rewrite/node.h"
#include "parser/data_typedef.h"

namespace cypher::rewrite {
class Node;

class Edge {
 public:
    size_t m_id;
    size_t m_source_id;
    size_t m_target_id;
    std::set<int> m_labels;
    parser::LinkDirection m_direction;
    Edge() {}
    Edge(size_t id, size_t source_id, size_t target_id, std::set<int> labels,
         parser::LinkDirection direction) {
        m_source_id = source_id;
        m_target_id = target_id;
        m_labels = labels;
        m_id = id;
        m_direction = direction;
    }

    ~Edge() {}
};

};  // namespace cypher::rewrite
