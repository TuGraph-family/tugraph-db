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

class MyConfig : public ConfigBase<double> {
 public:
    size_t iterations = 20;

    std::string name = std::string("wlpa");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
        config.Add(iterations, "iterations", true)
                .Comment("the iterations of lpa");
    }

    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  iterations: " << iterations << std::endl;
    }

    MyConfig(int &argc, char** &argv): ConfigBase<double>(argc, argv) {
        parse_line = parse_line_weighted<double>;
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
    std::string output_file = config.output_dir;
    size_t iterations = config.iterations;

    OlapOnDisk<double> graph;
    graph.Load(config, MAKE_SYMMETRIC);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto label_map = graph.AllocVertexArray<std::vector<std::pair<size_t, size_t>>>();
    auto modularity = WLPACore(graph, iterations);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    auto output_cost = get_time() - start_time;

    printf("modularity: %lf\n", modularity);
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
