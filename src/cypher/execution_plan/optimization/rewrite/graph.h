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
#pragma once
#include <iostream>
#include <string>
#include <vector>
#include "cypher/execution_plan/optimization/rewrite/node.h"
#include "cypher/execution_plan/optimization/rewrite/edge.h"

#include "cypher/execution_plan/ops/op.h"
namespace cypher::rewrite {

class Graph {
 public:
    std::vector<rewrite::Node> m_nodes;
    std::vector<rewrite::Edge> m_edges;

    Graph() {}
    ~Graph() {}
    void AddNode(size_t id, int label);

    void AddEdge(size_t id, size_t source_id, size_t target_id, std::set<int> labels,
                 parser::LinkDirection direction);

    void PrintGraph();
};

};  // namespace cypher::rewrite
