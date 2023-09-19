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

std::tuple<size_t, bool> parse_line_black(const char * p, const char * end, size_t & v) {
    const char * orig = p;
    int64_t node = 0;
    p += fma_common::TextParserUtils::ParseInt64(p, end, node);
    v = node;
    while (*p != '\n') p++;
    return std::tuple<size_t, bool>(p - orig, p != orig);
}

class MyConfig : public ConfigBase<double> {
 public:
    std::string name = "mssp";
    std::string roots_dir = "";

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
        config.Add(roots_dir, "roots_dir", true)
            .Comment("roots_dir of mssp");
    }
    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  roots_dir: " << roots_dir << std::endl;
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

int main(int argc, char ** argv) {
    double start_time;
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    start_time = get_time();
    MyConfig config(argc, argv);
    std::string roots_dir = config.roots_dir;
    std::string output_file = config.output_dir;

    OlapOnDisk<double> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    std::cout << "  num_vertices = " << graph.NumVertices() << std::endl;
    std::cout << "  num_edges = " << graph.NumEdges() << std::endl;

    std::vector<size_t> roots_list;
    std::vector<size_t> roots;
    if (roots_dir != "") {
         graph.LoadVertexArrayTxt<size_t>(roots_list, roots_dir, parse_line_black);
    }
    for (auto &ele : roots_list) {
        roots.push_back(ele);
    }
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<double> distance = graph.AllocVertexArray<double>();
    MSSPCore(graph, roots, distance);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto all_vertices = graph.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_distance_vi = graph.ProcessVertexActive<size_t>(
        [&](size_t vi) {
            if (distance[vi] > 1e10) {
                distance[vi] = -1;
            }
            return (size_t)vi;
        },
        all_vertices, 0,
        [&](size_t a, size_t b) { return distance[a] > distance[b] ? a : b; });
    printf("max distance is: %lf\n", distance[max_distance_vi]);

    if (output_file != "") {
        std::function<bool(double&)> filter_output = [&] (double & val) -> bool {
                return (val == -1) ? false : true;
            };
        graph.Write<double>(config, distance, graph.NumVertices(), config.name, filter_output);
    }
    auto output_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");

    return 0;
}
