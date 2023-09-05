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
    std::string name = std::string("louvain");
    int active_threshold = 0;
    int is_sync = 0;

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
        config.Add(active_threshold, "active_threshold", true)
            .Comment("Threshold of active_vertices in original graph."
            " Louvain process will stop when active vertices is smaller than active_threshold. "
            " If you want louvain process executed thoroughly until no active vertices,"
            " you can set this parameter to 1. When this parameter is smaller than or equal to 0,"
            " threshold will be one thousandth of the vertices of original graph");
        config.Add(is_sync, "is_sync", true)
            .Comment("if to run louvain in sync mode."
            "0 means async mode, non-zero value means sync mode.");
    }

    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name:             " << name << std::endl;
        std::cout << "  active_threshold: " << active_threshold << std::endl;
        std::cout << "  sync:             " << is_sync << std::endl;
    }

    MyConfig(int & argc, char** & argv) : ConfigBase<double>(argc, argv) {
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
    MyConfig config(argc, argv);
    start_time = get_time();
    std::string output_file = config.output_dir;

    OlapOnDisk<double> graph;
    graph.Load(config, MAKE_SYMMETRIC);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto label = graph.AllocVertexArray<size_t>();
    auto modularity = LouvainCore(graph, label, config.active_threshold, config.is_sync);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        graph.Write(config, label, graph.NumVertices(), config.name);
    }
    auto output_cost = get_time() - start_time;

    printf("The modularity of Graph is %.2lf\n", modularity);
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE\n");

    return 0;
}
