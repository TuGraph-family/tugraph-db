/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 */

#include "olap/olap_on_disk.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

std::tuple<size_t, bool> parse_line_search_file(const char * p,
                const char * end, std::pair<size_t, size_t> & v) {
    const char * orig = p;
    int64_t src = 0;
    int64_t dst = 0;
    p += fma_common::TextParserUtils::ParseInt64(p, end, src);
    while (p != end && (*p == ' ' || *p == '\t' || *p == ',')) p++;
    p += fma_common::TextParserUtils::ParseInt64(p, end, dst);
    v.first = src;
    v.second = dst;
    while (*p != '\n') p++;
    return std::tuple<size_t, bool>(p - orig, p != orig);
}

class MyConfig : public ConfigBase<Empty> {
 public:
    std::string name = "cn";
    int make_symmetric = 0;
    std::string search_dir = "";

    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<Empty>::AddParameter(config);
        config.Add(make_symmetric, "make_symmetric", true)
            .Comment("To make the input graph undirected or not");
        config.Add(search_dir, "search_dir", false)
            .Comment("Search pairs directory. Line format is node1,node2");
    }

    void Print() {
        ConfigBase<Empty>::Print();
        std::cout << "  name:           " << name << std::endl;
        std::cout << "  make_symmetric: " << make_symmetric << std::endl;
        std::cout << "  search_dir:     " << search_dir << std::endl;
    }

    MyConfig(int & argc, char** &argv) : ConfigBase<Empty>(argc, argv) {
        fma_common::Configuration config;
        AddParameter(config);
        config.ExitAfterHelp(true);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};

int main(int argc, char ** argv) {
    double start_time;
    MemUsage memUsage;
    memUsage.startMemRecord();

    // prepare
    start_time = get_time();
    MyConfig config(argc, argv);

    OlapOnDisk<Empty> graph;
    if (config.make_symmetric == 0) {
        graph.Load(config, INPUT_SYMMETRIC);
    } else {
        graph.Load(config, MAKE_SYMMETRIC);
    }
    memUsage.print();
    memUsage.reset();
    std::cout << "  |V| = " << graph.NumVertices() << std::endl;
    std::cout << "  |E| = " << graph.NumEdges() << std::endl;

    std::vector<std::pair<size_t, size_t> > read_list;
    std::vector<std::pair<size_t, size_t> > search_list;
    if (config.search_dir != "") {
         graph.LoadVertexArrayTxt<std::pair<size_t, size_t>>(read_list,
                            config.search_dir, parse_line_search_file);
    }
    for (auto &ele : read_list) {
        search_list.push_back(ele);
    }
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    std::vector<std::tuple<size_t, size_t, size_t> > result_list;
    for (auto search_pair : search_list) {
        size_t count_common = CNCore(graph, search_pair);
        result_list.push_back(std::make_tuple(search_pair.first, search_pair.second, count_common));
    }

    for (int i = 0; i < 5 && i < (int) result_list.size(); i++) {
        printf("cn(%lu,%lu) = %lu\n", std::get<0>(result_list[i]), std::get<1>(result_list[i]),
                std::get<2>(result_list[i]));
    }
    memUsage.print();
    memUsage.reset();
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (config.output_dir != "") {
        fma_common::OutputFmaStream fout;
        fout.Open(config.output_dir + "/" + config.name + "_out", 64 << 20);
        for (auto ele : result_list) {
            std::string line = fma_common::StringFormatter::Format("{} {} {}\n", std::get<0>(ele),
                                                                   std::get<1>(ele),
                                                                   std::get<2>(ele));
            fout.Write(line.c_str(), line.size());
        }
        fout.Close();
    }

    auto output_cost = get_time() - start_time;
    printf("prepare_cost = %.2lf(s)\n", prepare_cost);
    printf("core_cost = %.2lf(s)\n", core_cost);
    printf("output_cost = %.2lf(s)\n", output_cost);
    printf("total_cost = %.2lf(s)\n", prepare_cost + core_cost + output_cost);
    printf("ALL DONE.\n");

    return 0;
}
