/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/lgraph_olap.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

int main(int argc, char** argv) {
    double start_time;

    // prepare
    start_time = get_time();
    Graph<Empty> graph;
    size_t root_vid = 0;
    if (argc < 2) {
        throw std::runtime_error("graph file cannot be empty!");
    } else if (argc >= 3) {
        root_vid = std::atol(argv[2]);
    }
    std::string filename(argv[1]);
    graph.Load(filename);
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    auto parent = graph.AllocVertexArray<size_t>();
    size_t count = BFSCore(graph, root_vid, parent);
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    // TODO(any): write to file
    printf("found_vertices = %ld\n", count);
    auto output_cost = get_time() - start_time;
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
