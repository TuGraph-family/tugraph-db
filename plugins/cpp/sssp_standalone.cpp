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
    int64_t root_id = 0;
    std::string output_file = "";
    if (argc < 2) {
        throw std::runtime_error("graph file cannot be empty!");
    }
    if (argc >= 3) {
        root_id = std::atol(argv[2]);
    }
    if (argc >= 4) {
        output_file = std::string(argv[3]);
    }
    Graph<double> graph;
    graph.Load(argv[1], DUAL_DIRECTION, parse_line_weight<double>);
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    ParallelVector<double> distance = graph.AllocVertexArray<double>();
    SSSPCore(graph, root_id, distance);
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    size_t max_distance_vi = graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            if (distance[vi] > 1e10) {
                distance[vi] = -1;
            }
            return (size_t)vi;
        },
        0, graph.NumVertices(), 0,
        [&](size_t a, size_t b) { return distance[a] > distance[b] ? a : b; });
    if (output_file != "") {
        FILE* fout = fopen(output_file.c_str(), "w");
        for (size_t vi = 0; vi < graph.NumVertices(); vi++) {
            fprintf(fout, "%lu %lf\n", vi, distance[vi]);
        }
        fclose(fout);
    }
    printf("max distance is: distance[%ld]=%lf\n", max_distance_vi, distance[max_distance_vi]);
    auto output_cost = get_time() - start_time;
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
