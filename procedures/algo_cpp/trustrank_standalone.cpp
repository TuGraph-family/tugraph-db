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

std::tuple<size_t, bool> parse_line_trustUser_file(const char * p, const char * end, size_t & v) {
    const char * orig = p;
    int64_t node = 0;
    p += fma_common::TextParserUtils::ParseInt64(p, end, node);
    v = node;
    while (*p != '\n') p++;
    return std::tuple<size_t, bool>(p - orig, p != orig);
}

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = "trustrank";
    size_t iterations = 20;
    size_t empty_inherit = 0;
    std::string trustedUser_dir = "";
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(iterations, "iterations", true)
                .Comment("Iterations the trustrank procdess will be executed");
        config.Add(trustedUser_dir, "trustedUser_dir", false)
                .Comment("Directory where the trusted-user-files are stored."
                " Only trusted-user-files can be stored under the directory");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  iterations: " << iterations << std::endl;
        std::cout << "  trustedUser_dir: " << trustedUser_dir << std::endl;
    }

    MyConfig(int &argc, char** &argv): ConfigBase<Empty>(argc, argv) {
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};

int main(int argc, char ** argv) {
    auto start_time = get_time();
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    start_time = get_time();
    MyConfig config(argc, argv);
    size_t iterations = config.iterations;

    OlapOnDisk<Empty> graph;
    graph.Load(config);
    memUsage.print();
    memUsage.reset();
    std::cout << "  num_vertices = " << graph.NumVertices() << std::endl;
    std::cout << "  num_edges = " << graph.NumEdges() << std::endl;

    std::vector< size_t > read_list;
    std::vector< size_t > trust_list;
    if (config.trustedUser_dir != "") {
         graph.LoadVertexArrayTxt<size_t>(read_list,
                config.trustedUser_dir, parse_line_trustUser_file);
    }
    for (auto &ele : read_list) {
        trust_list.push_back(ele);
    }

    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto curr = graph.AllocVertexArray<double>();
    TrustrankCore(graph, iterations, curr, trust_list);
    auto all_vertices = graph.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_curr_vi =
        graph.ProcessVertexActive<size_t>([&](size_t vi) { return vi; }, all_vertices, 0,
                                    [&](size_t a, size_t b) { return curr[a] > curr[b] ? a : b; });
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (config.output_dir != "") {
        graph.Write<double>(config, curr, graph.NumVertices(), config.name);
    }
    auto output_cost = get_time() - start_time;
    printf("max rank value is [%ld] = %lf\n", max_curr_vi, curr[max_curr_vi]);

    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");

    return 0;
}
