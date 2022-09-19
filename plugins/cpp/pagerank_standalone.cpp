/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<Empty> {
 public:
    size_t num_iterations = 20;
    std::string name = std::string("pagerank");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(num_iterations, "num_iterations", true)
                .Comment("the num_iterations of pagerank");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        std::cout << "  num_iterations: " << num_iterations << std::endl;
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
    auto start_time = get_time();
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    start_time = get_time();
    MyConfig config(argc, argv);
    int num_iterations = config.num_iterations;

    OlapOnDisk<Empty> graph;
    graph.Load(config);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    ParallelVector<double> pr = graph.AllocVertexArray<double>();
    PageRankCore(graph, num_iterations, pr);
    auto all_vertices = graph.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_pr_vi =
        graph.ProcessVertexActive<size_t>([&](size_t vi) { return vi; }, all_vertices, 0,
                                    [&](size_t a, size_t b) { return pr[a] > pr[b] ? a : b; });
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    // TODO(any):
    printf("max rank value is pr[%ld] = %lf\n", max_pr_vi, pr[max_pr_vi]);
    auto output_cost = get_time() - start_time;
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
