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
    size_t samples = 64;
    std::string name = std::string("de");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(samples, "samples", true)
                .Comment("the samples of de");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  samples: " << samples << std::endl;
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
    size_t samples = config.samples;
    graph.Load(config, MAKE_SYMMETRIC);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    std::set<size_t> roots;
    unsigned int seed = 0;
    size_t num_vertices = graph.NumVertices();
    for (size_t k=0; k < samples; k++) {
        size_t s = rand_r(&seed) % num_vertices;
        roots.insert(s);
    }
    size_t max_path = 0;
    printf("1st phase from %lu seeds\n", roots.size());
    max_path = DECore(graph, roots);
    printf("1st phase max path: %ld\n", max_path + 1);
    printf("2nd phase from %lu seeds\n", roots.size());
    max_path = DECore(graph, roots);
    printf("2nd phase max path: %ld\n", max_path + 1);
    std::cout << "max_diamension:" << max_path + 1 << std::endl;
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost);
    printf("DONE.");

    return 0;
}
