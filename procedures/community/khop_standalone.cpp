/*
 * Copyright 2024 Yingqi Zhao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/**
 * @brief The k-hop algorithm.
 * 
 * @param root Identifier of root node.
 * @param value_k Number of search layers (value of k in K-hop algorithm).
 * @return Number of nodes in K-hop.
 */
#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string root = "0";
    std::string name = std::string("khop");
    size_t value_k = 3;
    void AddParameter(fma_common::Configuration& config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(root, "root", true).Comment("Identifier of the root node.");
        config.Add(value_k, "value_k", true).Comment(
            "Number of search layers(value of k in K-hop algorithm).");
    }
    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << " name: " << name << std::endl;
        std::cout << " root: " << name << std::endl;
        std::cout << " value_k: " << value_k << std::endl;
    }
    MyConfig(int& argc, char**& argv) : ConfigBase<Empty>(argc, argv) {
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};

extern size_t k_hop(OlapBase<Empty>& graph, size_t root_vid,
    ParallelVector<size_t>& result, size_t k);

int main(int argc, char** argv) {
    double start_time;
    MemUsage memUsage;
    memUsage.startMemRecord();
    start_time = get_time();
    MyConfig config(argc, argv);
    OlapOnDisk<Empty> graph;
    graph.Load(config, INPUT_SYMMETRIC);
    size_t root_vid;
    auto result = graph.AllocVertexArray<size_t>();
    result.Fill(0);
    if (config.id_mapping)
        root_vid = graph.hash_list_.find(config.root);
    else
        root_vid = std::stoi(config.root);
    size_t value_k = config.value_k;
    memUsage.print();
    memUsage.reset();
    auto prepare_cost = get_time()- start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);

    start_time = get_time();
    size_t count_result = k_hop(graph, root_vid, result, value_k);
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time()- start_time;

    start_time = get_time();
    if (config.output_dir != "") {
        graph.Write<size_t>(config, result, graph.NumVertices(), config.name);
    }
    printf("\n================\n");
    printf("Find %lu vertexes in %lu-hop from node NO.%lu", count_result, value_k, root_vid);
    printf("\n================\n");
    auto output_cost = get_time()- start_time;

    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("DONE.\n");
    return 0;
}
