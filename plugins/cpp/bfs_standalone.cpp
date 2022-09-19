/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<Empty> {
 public:
    size_t root = 0;
    std::string name = std::string("bfs");
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(root, "root", true)
                .Comment("the root of bfs");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name: " << name << std::endl;
        if (root != size_t(-1)) {
            std::cout << "  root: " << root << std::endl;
        } else {
            std::cout << "  root: UNSET" << std::endl;
        }
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
    size_t root_vid = config.root;

    OlapOnDisk<Empty> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    // core
    start_time = get_time();
    auto parent = graph.AllocVertexArray<size_t>();
    size_t count = BFSCore(graph, root_vid, parent);
    memUsage.print();
    memUsage.reset();
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
