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

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = "fast_triangle";
    int make_symmetric = 1;
    void AddParameter(fma_common::Configuration &config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(make_symmetric, "make_symmetric", true)
            .Comment("To make input graph undirected or not");
    }

    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name:            " << name << std::endl;
        std::cout << "  make_symmetric:  " << make_symmetric << std::endl;
    }

    MyConfig(int &argc, char **&argv) : ConfigBase<Empty>(argc, argv) {
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};

int main(int argc, char **argv) {
    auto start_time = get_time();
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    MyConfig config(argc, argv);
    int make_symmetric = config.make_symmetric;

    OlapOnDisk<Empty> graph;
    if (make_symmetric == 0) {
        graph.Load(config, INPUT_SYMMETRIC);
    } else {
        graph.Load(config, MAKE_SYMMETRIC);
    }
    memUsage.print();
    memUsage.reset();
    std::cout << "  num_vertices = " << graph.NumVertices() << std::endl;
    std::cout << "  num_edges = " << graph.NumEdges() << std::endl;
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto num_triangle = graph.AllocVertexArray<size_t>();
    num_triangle.Fill(0);
    auto discovered_triangles = FastTriangleCore(graph, num_triangle);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (config.output_dir != "") {
        graph.Write<size_t>(config, num_triangle, graph.NumVertices(), config.name);
    }
    printf("discovered %lu triangles\n", discovered_triangles);
    auto output_cost = get_time() - start_time;

    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");

    return 0;
}
