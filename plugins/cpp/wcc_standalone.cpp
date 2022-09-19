/* Copyright (c) 2022 AntGroup. All Rights Reserved. */


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
        if (graph.OutDegree(v) == 0) return 0;
        size_t v_label = label[v];
        write_add(&cnt[v_label], (size_t)1);
        return 0;
    }, 0, label.Size());
    max = 0;
    num = graph.ProcessVertexInRange<size_t>([&](size_t v) {
      if (max < cnt[v])
          write_max(&max, cnt[v]);
      return (graph.OutDegree(v) > 0 && cnt[v] > 0) ? 1 : 0;
    }, 0, label.Size());
}

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = std::string("wcc");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
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

    OlapOnDisk<Empty> graph;
    graph.Load(config, MAKE_SYMMETRIC);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    ParallelVector<size_t> wcc_label = graph.AllocVertexArray<size_t>();
    WCCCore(graph, wcc_label);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    size_t num_components, max_component;
    CountComp(graph, wcc_label, max_component, num_components);
    printf("max_component = %ld\n", max_component);
    printf("num_components = %ld\n", num_components);
    if (output_file != "") {
        graph.Write(config, wcc_label, graph.NumVertices(), config.name);
    }
    auto output_cost = get_time() - start_time;
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.\n");

    return 0;
}
