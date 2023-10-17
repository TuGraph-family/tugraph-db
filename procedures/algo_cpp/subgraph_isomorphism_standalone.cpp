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

#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = "subgraph_isomorphism";
    std::vector<std::unordered_set<size_t>> query;
    std::string query_s;
    void AddParameter(fma_common::Configuration& config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(query_s, "query", false).Comment("edges of subgraph");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name:           " << name << std::endl;
        std::cout << "  query_vertices: " << query.size() << std::endl;
    }
    MyConfig(int& argc, char**& argv) : ConfigBase<Empty>(argc, argv) {
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        printf("query = %s\n", query_s.c_str());

        json input = json::parse("{\"query\":" + query_s + "}");
        printf("input = %s\n", input.dump().c_str());
        query = input["query"].get<std::vector<std::unordered_set<size_t>>>();
        Print();
    }
};

int main(int argc, char** argv) {
    double start_time;
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    start_time = get_time();
    OlapOnDisk<Empty> graph;
    MyConfig config(argc, argv);

    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    std::cout << "  num_vertices = " << graph.NumVertices() << std::endl;
    std::cout << "  num_edges = " << graph.NumEdges() << std::endl;
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<size_t> counts = graph.AllocVertexArray<size_t>();
    SubgraphIsomorphismCore(graph, config.query, counts);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (config.output_dir != "") {
        graph.Write<size_t>(config, counts, graph.NumVertices(), config.name);
    }
    auto output_cost = get_time() - start_time;

    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");

    return 0;
}
