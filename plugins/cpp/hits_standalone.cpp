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
    int iterations = 100;
    double delta = 0.0001;
    std::string name = "hits";

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(iterations, "iterations", true)
                .Comment("Max iterations to run");
        config.Add(delta, "delta", true)
                .Comment("Max delta value to stop");
    }

    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name:       " << name << std::endl;
        std::cout << "  iterations: " << iterations << std::endl;
        std::cout << "  delta:      " << delta << std::endl;
    }

    MyConfig(int & argc, char** &argv) : ConfigBase<Empty>(argc, argv) {
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
    int iterations = config.iterations;
    double delta = config.delta;

    OlapOnDisk<Empty> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    std::cout << "  num_vertices = " << graph.NumVertices() << std::endl;
    std::cout << "  num_edges = " << graph.NumEdges() << std::endl;
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    size_t max_authority_index = 0;
    size_t max_hub_index = 0;
    auto authority = graph.AllocVertexArray<double>();
    auto hub = graph.AllocVertexArray<double>();
    HITSCore(graph, authority, hub, iterations, delta);
    for (size_t vi = 0; vi < graph.NumVertices(); vi++) {
        if (authority[vi] > authority[max_authority_index]) {
            max_authority_index = vi;
        }
        if (hub[vi] > hub[max_hub_index]) {
            max_hub_index = vi;
        }
    }
    printf("max authority value is authority[%lu] = %lf\n",
                max_authority_index, authority[max_authority_index]);
    printf("max hub value is hub[%lu] = %lf\n", max_hub_index, hub[max_hub_index]);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (config.output_dir != "") {
        fma_common::OutputFmaStream fout;
        fout.Open(config.output_dir + "/" + config.name + "_out", 64 << 20);
        for (size_t vi = 0; vi < graph.NumVertices(); vi++) {
        std::string line = fma_common::StringFormatter::Format
                            ("{} {} {}\n", vi, authority[vi], hub[vi]);
           fout.Write(line.c_str(), line.size());
        }
        fout.Close();
    }
    auto output_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");
    return 0;
}
