/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/lgraph_olap.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

int main(int argc, char** argv) {
    double start_time;
    size_t num_iterations = 20;
    bool sync_flag = 1;
    // prepare
    start_time = get_time();
    Graph<Empty> graph;
    std::string output_file = "";
    if (argc < 2) {
        throw std::runtime_error("graph file cannot be empty!");
    }
    if (argc >= 3) {
        num_iterations = std::atol(argv[2]);
    }
    if (argc >= 4) {
        sync_flag = std::atol(argv[3]);
    }
    if (argc >= 5) {
        output_file = std::string(argv[4]);
    }
    std::string filename(argv[1]);
    graph.Load(filename, MAKE_SYMMETRIC);
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    double modularity = LPACore(graph, num_iterations, sync_flag);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto output_cost = get_time() - start_time;
    printf("modularity: %lf\n", modularity);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
