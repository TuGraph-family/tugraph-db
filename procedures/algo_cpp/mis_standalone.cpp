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
    std::string name = std::string("mis");
    int make_symmetric = 0;
    void AddParameter(fma_common::Configuration& config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(make_symmetric, "make_symmetric", true)
            .Comment("To make input graph undirected or not");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  make_symmetric:  " << make_symmetric << std::endl;
    }

    MyConfig(int& argc, char**& argv) : ConfigBase<Empty>(argc, argv) {
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

    start_time = get_time();
    OlapOnDisk<Empty> graph;
    MyConfig config(argc, argv);

    if (config.make_symmetric == 0) {
        graph.Load(config, INPUT_SYMMETRIC);
    } else {
        graph.Load(config, MAKE_SYMMETRIC);
    }
    memUsage.print();
    memUsage.reset();
    std::cout << "  num_vertices = " << graph.NumVertices() << std::endl;
    std::cout << "  num_edges = " << graph.NumEdges() << std::endl;
    auto prepare_cost = get_time() - start_time;

    start_time = get_time();
    auto mis = graph.AllocVertexArray<bool>();
    size_t mis_size = 0;
    MISCore(graph, mis, mis_size);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    start_time = get_time();
    if (config.output_dir != "") {
        std::function<bool(bool&)> filter_output = [&](bool& val) -> bool { return val; };
        graph.Write<bool>(config, mis, graph.NumVertices(), config.name, filter_output);
    }
    auto output_cost = get_time() - start_time;

    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");

    return 0;
}
