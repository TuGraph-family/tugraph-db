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
    size_t value_k = 3;
    std::string name = std::string("kcliques");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(value_k, "value_k", true)
                .Comment("the value_k of kcliques");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  value_k: " << value_k << std::endl;
    }

    MyConfig(int &argc, char** &argv): ConfigBase<Empty>(argc, argv) {
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
    MyConfig config(argc, argv);
    OlapOnDisk<Empty> graph;
    int value_k = config.value_k;

    graph.Load(config, MAKE_SYMMETRIC);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto cliques_cnt = graph.AllocVertexArray<size_t>();
    cliques_cnt.Fill(0);
    size_t left_edges = KCliquesCore(graph, value_k, cliques_cnt);
    auto core_cost = get_time() - start_time;
    memUsage.print();
    memUsage.reset();

    // output
    start_time = get_time();
    if (config.output_dir != "") {
        graph.Write<size_t>(config, cliques_cnt, graph.NumVertices(), config.name);
    }
    printf("found %ld %d-cliques\n", left_edges, value_k);
    auto output_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
