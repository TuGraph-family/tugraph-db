/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/lgraph_graph.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

int main(int argc, char** argv) {
    auto start_time = get_time();

    // prepare
    start_time = get_time();
    int num_iterations = 20;
    if (argc < 2) {
        throw std::runtime_error("graph file cannot be empty!");
    } else if (argc >= 3) {
        num_iterations = std::atoi(argv[2]);
    }
    Graph<Empty> graph;
    std::string filename(argv[1]);
    graph.Load(filename);
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
