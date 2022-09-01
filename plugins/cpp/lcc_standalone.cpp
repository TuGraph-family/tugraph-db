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
    std::string output_file = "";
    if (argc < 2) {
        throw std::runtime_error("graph file cannot be empty!");
    }
    if (argc >= 3) {
        output_file = std::string(argv[3]);
    }
    std::string filename(argv[1]);
    graph.Load(filename, MAKE_SYMMETRIC);
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    auto score = graph.AllocVertexArray<double>();
    double average_clco = LCCCore(graph, score);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        FILE* fout = fopen(output_file.c_str(), "w");
        for (size_t i = 0; i < graph.NumVertices(); i++) {
            fprintf(fout, "%lu %lf\n", i, score[i]);
        }
        fclose(fout);
    }
    printf("average_clco is: %lf\n", average_clco);
    auto output_cost = get_time() - start_time;

    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
