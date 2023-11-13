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

void CountComp(OlapBase<Empty>& graph, ParallelVector<size_t>& label, size_t& max, size_t& num) {
    ParallelVector<size_t> cnt = graph.AllocVertexArray<size_t>();
    cnt.Fill(0);
    graph.ProcessVertexInRange<size_t>([&](size_t v) {
      if (graph.OutDegree(v) == 0 && graph.InDegree(v) == 0) return 0;
      size_t v_label = label[v];
      write_add(&cnt[v_label], (size_t)1);
      return 0;
    }, 0, label.Size());
    max = 0;
    num = graph.ProcessVertexInRange<size_t>([&](size_t v) {
      if (max < cnt[v])
          write_max(&max, cnt[v]);
      return ((graph.OutDegree(v) > 0 || graph.InDegree(v) > 0) && cnt[v] > 0) ? 1 : 0;
    }, 0, label.Size());
}

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = std::string("scc");

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
    }

    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
    }

    MyConfig(int &argc, char** &argv) : ConfigBase<Empty>(argc, argv) {
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
    std::string output_file = config.output_dir;

    OlapOnDisk<Empty> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    start_time = get_time();
    auto label = graph.AllocVertexArray<size_t>();
    SCCCore(graph, label);
    size_t num_components, max_component;
    CountComp(graph, label, max_component, num_components);
    printf("max_component = %ld\n", max_component);
    printf("num_components = %ld\n", num_components);
    auto core_cost = get_time() - start_time;
    memUsage.print();
    memUsage.reset();

    // output
    start_time = get_time();
    if (output_file != "") {
        graph.Write<size_t>(config, label, graph.NumVertices(), config.name);
    }
    auto output_cost = get_time() - start_time;

    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.\n");

    return 0;
}
