/* Copyright (c) 2022 AntGroup. All Rights Reserved. */


#include "lgraph/lgraph_graph.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

void CountComp(Graph<Empty>& graph, ParallelVector<size_t>& label, size_t& max, size_t& num) {
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

int main(int argc, char** argv) {
    double start_time;

    // prepare
    start_time = get_time();
    std::string output_file = "";
    if (argc < 2) {
        throw std::runtime_error("graph file cannot be empty");
    } else if (argc >= 3) {
        output_file = std::string(argv[2]);
    }
    Graph<Empty> graph;
    graph.Load(argv[1], MAKE_SYMMETRIC);
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    ParallelVector<size_t> wcc_label = graph.AllocVertexArray<size_t>();
    WCCCore(graph, wcc_label);
    auto core_cost = get_time() - start_time;
    printf("core_cost = %.2lf(s)\n", core_cost);

    // output
    start_time = get_time();
    size_t num_components, max_component;
    CountComp(graph, wcc_label, max_component, num_components);
    printf("max_component = %ld\n", max_component);
    printf("num_components = %ld\n", num_components);
    if (output_file != "") {
        FILE* fout = fopen(output_file.c_str(), "w");
        for (size_t vi = 0; vi < graph.NumVertices(); vi++) {
            fprintf(fout, "%lu %ld\n", vi, wcc_label[vi]);
        }
        fclose(fout);
    }
    auto output_cost = get_time() - start_time;
    printf("output_cost = %.2lf(s)\n", output_cost);

    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.\n");

    return 0;
}
