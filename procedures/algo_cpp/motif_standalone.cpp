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

#include <sstream>
#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = "motif";
    int value_k = 3;
    std::string vertices_ids;
    void AddParameter(fma_common::Configuration& config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(value_k, "value_k", false).Comment("size of subgraph");
        config.Add(vertices_ids, "vertices_ids", false).Comment("vertices of computation");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name:         " << name << std::endl;
        std::cout << "  value_k:      " << value_k << std::endl;
        std::cout << "  vertices_ids: " << vertices_ids << std::endl;
    }
    MyConfig(int& argc, char**& argv) : ConfigBase<Empty>(argc, argv) {
        parse_line = parse_line_unweighted<Empty>;
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
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
    int value_k = config.value_k;
    int cnt = std::count(config.vertices_ids.begin(), config.vertices_ids.end(), ',') + 1;
    ParallelVector<size_t> vertices_ids(cnt);
    std::stringstream ss(config.vertices_ids);
    std::string vid;
    while (getline(ss, vid, ',')) {
        vertices_ids.Append(std::stoul(vid));
    }

    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    std::cout << "  num_vertices = " << graph.NumVertices() << std::endl;
    std::cout << "  num_edges = " << graph.NumEdges() << std::endl;
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<std::map<uint64_t, size_t>> counts =
        graph.AllocVertexArray<std::map<uint64_t, size_t>>();
    MotifCore(graph, vertices_ids, value_k, counts);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (config.output_dir != "") {
        std::function<bool(std::map<uint64_t, size_t>&)> filter_output =
            [&](std::map<uint64_t, size_t>& val) -> bool { return !val.empty(); };
        graph.Write<std::map<uint64_t, size_t>>(config, counts, graph.NumVertices(), config.name,
                                                filter_output);
    }
    for (auto& it : counts[0]) {
        printf("[%zu,%zu]\n", it.first, it.second);
    }
    auto output_cost = get_time() - start_time;

    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");

    return 0;
}
