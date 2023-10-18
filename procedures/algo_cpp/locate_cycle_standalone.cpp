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
    size_t k = 3;
    std::string name = std::string("locate_cycle");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(k, "k", true)
                .Comment("the k of locate_cycle");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  k: " << k << std::endl;
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
    std::string output_file = config.output_dir;
    size_t k = config.k;

    OlapOnDisk<Empty> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto rings_num = LocateCycleCore(graph, k);
    auto core_cost = get_time() - start_time;
    memUsage.print();
    memUsage.reset();
    printf("the num of rings is: %ld\n", rings_num);

    // output
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost);
    printf("DONE.");

    return 0;
}
