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
#include <unordered_map>
#include <vector>
#include "cypher/execution_plan/optimization/rewrite/node.h"
#include "cypher/execution_plan/optimization/rewrite/graph.h"

#include "cypher/execution_plan/ops/op.h"
#include "graph/common.h"
#include "parser/data_typedef.h"
#include "cypher/execution_plan/optimization/rewrite/state_info.h"
#include "cypher/execution_plan/execution_plan.h"

namespace cypher {

typedef std::map<NodeID, std::string> SchemaNodeMap;
typedef std::map<RelpID, std::tuple<NodeID, NodeID, std::set<std::string>, parser::LinkDirection>>
    SchemaRelpMap;
typedef std::pair<SchemaNodeMap, SchemaRelpMap> SchemaGraphMap;

namespace rewrite {

class SchemaRewrite {
 public:
    std::map<std::string, int> label2idx;        // label to index
    int label_cnt = 0;
    std::vector<std::string> idx2label;          // index to label

    std::map<size_t, cypher::NodeID> vidx2pidx;  // vertex id to pattern graph id
    std::map<cypher::NodeID, size_t> pidx2vidx;  // pattern graph id to vertex id
    std::map<size_t, cypher::NodeID> eidx2pidx;  // edge id to pattern graph id

    std::vector<std::set<size_t>> edge_core;  // pattern graph edge id to dst graph edge id
    std::vector<cypher::SchemaGraphMap> sgm;
    cypher::SchemaNodeMap* m_schema_node_map;
    cypher::SchemaRelpMap* m_schema_relp_map;

    std::vector<bool> visited;
    std::vector<bool> edge_visited;

    Graph target_graph;
    Graph query_graph;
    std::vector<int> core_2;
    size_t query_size;
    size_t target_size;
    size_t map_cnt = 0;  // matched vertices count in graph
    size_t depth = 0;    // depth in traversal

    SchemaRewrite() {}
    ~SchemaRewrite() {}

    std::vector<cypher::SchemaGraphMap> GetEffectivePath(const lgraph::SchemaInfo& schema_info,
                                                         cypher::SchemaNodeMap* schema_node_map,
                                                         cypher::SchemaRelpMap* schema_relp_map);
    int GetLabelNum(std::string label);
    void PrintLabel2Idx();
    void PrintMapping();
    void AddMapping();
    void Reset();
    bool CheckNodeLabel(size_t vid, size_t t_vid);
    // void Backtrack(size_t vid);
    std::map<size_t, std::set<size_t>> GetNextTVidByEdgeId(std::set<size_t>& edge_ids,
                                                           size_t t_vid);
    std::set<size_t> GetNextEdgeIds(Edge* edge, size_t t_vid, size_t direction);
    void MatchRecursive(size_t vid, size_t t_vid);
    std::vector<StateInfo> GenCandidateStateInfo();
};

};  // namespace rewrite
};  // namespace cypher
