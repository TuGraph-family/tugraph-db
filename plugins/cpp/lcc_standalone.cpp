/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = std::string("lcc");
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
    start_time = get_time();
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
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
    auto score = graph.AllocVertexArray<double>();
    double average_clco = LCCCore(graph, score);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    if (output_file != "") {
        graph.Write(config, score, graph.NumVertices(), config.name);
    }
    printf("average_clco is: %lf\n", average_clco);
    auto output_cost = get_time() - start_time;

    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
