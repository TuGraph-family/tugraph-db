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
    double gamma = 0.2;
    double theta = 0.1;
    unsigned random_seed = 0;
    size_t threshold = 0;
    std::string name = std::string("leiden");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
        config.Add(gamma, "gamma", true)
                .Comment("the gamma of leiden");
        config.Add(theta, "theta", true)
                .Comment("the theta of leiden");
        config.Add(random_seed, "random_seed", true)
                .Comment("the random_seed of leiden");
        config.Add(threshold, "threshold", true)
                .Comment("the threshold of leiden");
    }

    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  gamma: " << gamma << std::endl;
        std::cout << "  theta: " << theta << std::endl;
        std::cout << "  random_seed: " << gamma << std::endl;
        std::cout << "  threshold: " << theta << std::endl;
    }

    MyConfig(int &argc, char** &argv) : ConfigBase<double>(argc, argv) {
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
    double gamma = config.gamma;
    double theta = config.theta;
    unsigned random_seed = config.random_seed;
    size_t threshold = config.threshold;

    OlapOnDisk<double> graph;
    graph.Load(config, MAKE_SYMMETRIC);
    printf("|V|=%lu, |E|=%lu\n", graph.NumVertices(), graph.NumEdges());
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto label = graph.AllocVertexArray<size_t>();
    LeidenCore(graph, label, random_seed, theta, gamma, threshold);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write to file
    auto output_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
