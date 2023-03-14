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
#define DOUBLE_MAX std::numeric_limits<double>::max()

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<double> {
 public:
    std::string root = "0";
    std::string name = std::string("sssp");

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
        config.Add(root, "root", true).Comment("the root of sssp");
    }

    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  root: " << root << std::endl;
    }

    MyConfig(int &argc, char** &argv): ConfigBase<double>(argc, argv) {
        parse_line = parse_line_weighted<double>;
        parse_string_line = parse_string_line_weighted<double>;
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};

int main(int argc, char** argv) {
    auto start_time = get_time();
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    MyConfig config(argc, argv);
    std::string output_file = config.output_dir;

    OlapOnDisk<double> graph;
    size_t root_vid;
    graph.Load(config, DUAL_DIRECTION);
    if (config.id_mapping) {
        root_vid = graph.hash_list_.find(config.root);
    } else {
        root_vid = std::stoi(config.root);
    }
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    ParallelVector<double> distance = graph.AllocVertexArray<double>();
    SSSPCore(graph, root_vid, distance);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    size_t max_distance_vi = graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            if (distance[vi] > 1e10) {
                distance[vi] = -1;
            }
            return (size_t)vi;
        },
        0, graph.NumVertices(), 0,
        [&](size_t a, size_t b) { return distance[a] > distance[b] ? a : b; });
    if (config.id_mapping) {
        printf("max distance is: distance[%s]=%lf\n",
                graph.mapped_to_origin_[max_distance_vi].c_str(), distance[max_distance_vi]);
    } else {
        printf("max distance is: distance[%ld]=%lf\n", max_distance_vi, distance[max_distance_vi]);
    }

    if (output_file != "") {
        std::function<bool(double&)> filter_output = [&] (double & val) -> bool {
                return (val == DOUBLE_MAX) ? false : true;
            };
        graph.Write(config, distance, graph.NumVertices(), config.name, filter_output);
    }
    auto output_cost = get_time() - start_time;
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
