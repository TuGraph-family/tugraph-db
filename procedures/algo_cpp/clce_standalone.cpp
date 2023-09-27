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
    size_t samples = 64;
    std::string name = std::string("clce");

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
        config.Add(samples, "samples", true)
                .Comment("the samples of clce");
    }

    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  samples: " << samples << std::endl;
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
    start_time = get_time();
    // prepare
    MyConfig config(argc, argv);
    size_t samples = config.samples;
    std::string output_file = config.output_dir;

    OlapOnDisk<double> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto score = graph.AllocVertexArray<double>();
    auto path_num = graph.AllocVertexArray<size_t>();
    CLCECore(graph, samples, score, path_num);
    auto active_all = graph.AllocVertexSubset();
    active_all.Fill();
    size_t max_score_vi = 0;
    graph.ProcessVertexActive<size_t>([&](size_t vi) {
        if (path_num[vi] > samples / 5 && score[vi] > score[max_score_vi]) {
            max_score_vi = vi;
        }
        return 0;
    }, active_all);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        FILE* fout = fopen(output_file.c_str(), "w");
#pragma omp parallel for
        for (size_t i = 0; i < graph.NumVertices(); i++) {
            fprintf(fout, "%lu %lu %lf\n", i, path_num[i], score[i]);
        }
        fclose(fout);
    }
    auto output_cost = get_time() - start_time;
    printf("max_score is score[%lu] = %lf\n", max_score_vi, score[max_score_vi]);
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
