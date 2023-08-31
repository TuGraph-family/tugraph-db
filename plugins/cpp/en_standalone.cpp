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
    size_t root_vid = 0;
    int hop_value = 1;
    std::string name = std::string("en");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(root_vid, "root_vid", true)
                .Comment("the root_vid of en");
        config.Add(hop_value, "hop_value", true)
                .Comment("the hop_value of en");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  root_vid: " << root_vid << std::endl;
        std::cout << "  hop_value: " << hop_value << std::endl;
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
    size_t root_vid = config.root_vid;
    std::string output_file = config.output_dir;
    int hop_value = config.hop_value;

    OlapOnDisk<Empty> graph;
    graph.Load(config);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto discovered_num = ENCore(graph, root_vid, hop_value);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    printf("num_reachable_vertices:%ld\n", discovered_num);
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost);
    printf("DONE.");

    return 0;
}
