/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<double> {
 public:
    std::string name = std::string("apsp");

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
    }

    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name: " << name << std::endl;
    }

    MyConfig(int &argc, char** &argv) : ConfigBase<double>(argc, argv) {
        parse_line = parse_line_weighted<double>;
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        // this->GetDataFromParameter();
        Print();
    }
};

int main(int argc, char** argv) {
    double start_time;
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    MyConfig config(argc, argv);
    start_time = get_time();
    std::string output_file = config.output_dir;

    OlapOnDisk<double> graph;
    graph.Load(config, DUAL_DIRECTION);
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    std::vector< std::tuple<size_t, size_t, double> > result;
    std::tuple<size_t, size_t, double> max_tuple;
    APSPCore(graph, result, max_tuple);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        FILE* fout = fopen(output_file.c_str(), "w");
        for (auto ele : result) {
            fprintf(fout, "%lu %lu %lf\n", std::get<0>(ele), std::get<1>(ele), std::get<2>(ele));
        }
        fclose(fout);
    }
    auto output_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.");

    return 0;
}
