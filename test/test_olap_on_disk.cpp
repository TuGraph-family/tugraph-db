/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "olap/olap_on_disk.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

struct ParamConfig {
    ParamConfig(std::string _type, std::string _input_dir, bool _id_mapping) {
        type = _type;
        input_dir = _input_dir;
        id_mapping = _id_mapping;
    }
    std::string type;
    std::string input_dir;
    bool id_mapping;
};

// test_unweight
class UnWeight_Config : public ConfigBase<Empty> {
 public:
    void AddParameter(fma_common::Configuration& config) {
        ConfigBase<Empty>::AddParameter(config);
    }
    void Print() {
        ConfigBase<Empty>::Print();
    }

    UnWeight_Config(int &argc, char** &argv) : ConfigBase<Empty>(argc, argv) {
        fma_common::Configuration config;
        AddParameter(config);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};

// test_weight and idmapping
class Weight_Config : public ConfigBase<double> {
 public:
    void AddParameter(fma_common::Configuration & config) {
        ConfigBase<double>::AddParameter(config);
    }

    void Print() {
        ConfigBase<double>::Print();
        std::cout << "  name: " << name << std::endl;
    }

    Weight_Config(int &argc, char** &argv): ConfigBase<double>(argc, argv) {
        parse_line = parse_line_weighted<double>;
        parse_string_line = parse_string_line_weighted<double>;
        fma_common::Configuration config;
        AddParameter(config);
        config.ParseAndFinalize(argc, argv);
        Print();
    }
};

template <typename T>
void CreateCsvFiles(const T& data) {
    fma_common::OutputFmaStream stream;
    for (auto& kv : data) {
        const std::string& file_name = kv.first;
        const std::string& data = kv.second;
        if (file_name == "") continue;
        stream.Open(file_name);
        stream.Write(data.data(), data.size());
        stream.Close();
        UT_LOG() << "  " << file_name << " created";
    }
}

static void WriteOlapDiskFiles() {
static const std::vector<std::pair<std::string, std::string>> data_import = {
{"test_data",
R"(0,1,1
0,2,2
0,3,3
0,4,4
0,5,5
1,2,2
1,3,3
1,5,5
2,4,4
2,6,6
2,8,8
3,4,4
4,5,5
4,17,17
5,6,6
5,10,10
5,19,19
7,11,11
7,13,13
8,10,10
9,12,12
9,14,14
9,20,20
10,11,11
10,13,13
11,13,13
12,15,15
12,18,18
13,15,15
13,20,20
14,15,15
15,17,17
15,19,19
16,17,17
17,18,18
)"}};
    CreateCsvFiles(data_import);
}

class TestOlapOnDisk : public TuGraphTestWithParam<struct ParamConfig> {};

TEST_P(TestOlapOnDisk, OlapOnDisk) {
    // configure test data
    WriteOlapDiskFiles();
    system("mkdir ut_data && mv test_data ut_data/");
    size_t index = 0;
    std::string input_dirs = GetParam().input_dir;
    bool id_mappings = GetParam().id_mapping;

    int argc = 3;
    const char* args[3] = {"unit_test", "--input_dir", "./"};
    char** argv = (char**)args;
    UnWeight_Config unweight_config(argc, argv);
    unweight_config.id_mapping = id_mappings;
    unweight_config.input_dir = input_dirs;

    // test GetType()
    auto type_name = unweight_config.GetType();
    UT_EXPECT_EQ(type_name, TEXT_FILE);

    // test olap_on_disk
    if (!id_mappings) {
        OlapOnDisk<Empty> graph;
        graph.Load(unweight_config, DUAL_DIRECTION);
        UT_EXPECT_THROW_MSG(graph.set_num_vertices(3), "|V| can only be set before loading!");
        UT_EXPECT_THROW_MSG(graph.Load(unweight_config, DUAL_DIRECTION),
                            "Graph should not be loaded twice!");
        auto active = graph.AllocVertexSubset();
        active.Fill();
        UT_EXPECT_EQ(active.Size(), 21);
        UT_EXPECT_EQ(graph.NumVertices(), 21);
        UT_EXPECT_EQ(graph.NumEdges(), 35);

        auto label = graph.AllocVertexArray<size_t>();
        graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                label[vi] = -1;
                return 1;
            },
            active);

        for (auto iter = label.begin(); iter != label.end(); iter++) {
            UT_EXPECT_EQ(*iter, -1);
        }

        UT_EXPECT_ANY_THROW(label.ReAlloc(graph.NumVertices()/2));
        UT_EXPECT_ANY_THROW(label.ReAlloc(0));
        UT_EXPECT_EQ(label.Size(), 21);
        UT_EXPECT_EQ(label.Back(), -1);

        graph.ProcessVertexInRange<size_t>([&](size_t v) {
            if (graph.OutDegree(v) == 0) return 0;
            for (auto& edge : graph.OutEdges(v)) {
                size_t dst = edge.neighbour;
                if (label[dst] == -1) {
                    auto lock = graph.GuardVertexLock(dst);
                    if (label[dst] == -1) {
                        label[dst] = 1;
                        index += 1;
                    }
                }
            }
            return 0;
        }, 0, label.Size());

        UT_EXPECT_EQ(index, 17);
        UT_EXPECT_EQ(label[1], 1);

        graph.Write(unweight_config, label, graph.NumVertices(), "test");
        unweight_config.output_dir = "./";
        graph.Write(unweight_config, label, graph.NumVertices(), "test_olap");
        system("rm -f test_olap*");

        // test loadfromarray and weight graph
        OlapOnDisk<double> sub_graph;
        auto edge_array = new EdgeUnit<double>[11];
        UT_EXPECT_THROW_MSG(sub_graph.LoadFromArray((char*)edge_array, 0,
                            0, DUAL_DIRECTION), "Construct empty graph");

        size_t sub_edge_index = 0;
        for (int i = 1; i < 11; i++) {
            edge_array[sub_edge_index].src = i;
            edge_array[sub_edge_index].dst = 0;
            edge_array[sub_edge_index].edge_data = 2.0;
            sub_edge_index++;
        }

        index = 0;
        sub_graph.LoadFromArray((char*)edge_array, 11, 10, DUAL_DIRECTION);
        delete [] edge_array;
        auto node = sub_graph.AllocVertexSubset();
        node.Fill();
        auto sub_label = sub_graph.AllocVertexArray<size_t>();
        sub_graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                sub_label[vi] = -1;
                return 1;
            },
            node);
        UT_EXPECT_EQ(sub_label.Size(), 11);
        UT_EXPECT_EQ(sub_label[0], -1);

        for (auto& edge : sub_graph.InEdges(0)) {
            UT_EXPECT_EQ(edge.edge_data, 2);
        }

        // test AcquireVertexLock and ReleaseVertexLock
        sub_graph.ProcessVertexInRange<size_t>([&](size_t v) {
            if (sub_graph.InDegree(v) == 0) return 0;
            for (auto& edge : sub_graph.InEdges(v)) {
                size_t dst = edge.neighbour;
                sub_graph.AcquireVertexLock(dst);
                if (sub_label[dst] == -1) {
                    sub_label[dst] = 1;
                    index += 1;
                }
                sub_graph.ReleaseVertexLock(dst);
            }
            return 0;
        }, 0, node.Size());
        UT_EXPECT_EQ(index, 10);

        // test parse_line_weight
        OlapOnDisk<double> weight_graph;
        Weight_Config weight_config(argc, argv);
        weight_config.id_mapping = id_mappings;
        weight_config.input_dir = input_dirs;
        weight_graph.Load(weight_config);
        UT_EXPECT_EQ(weight_graph.NumVertices(), 21);
        UT_EXPECT_EQ(weight_graph.NumEdges(), 35);
        for (auto& edge : weight_graph.OutEdges(10)) {
            UT_EXPECT_EQ(edge.edge_data, edge.neighbour);
        }
        for (auto& edge : weight_graph.InEdges(10)) {
            UT_EXPECT_EQ(edge.edge_data, 10);
        }
        UT_EXPECT_EQ(weight_graph.OutEdges(0).begin()->neighbour,
                    weight_graph.OutEdges(0).begin()->edge_data);
        UT_EXPECT_EQ(weight_graph.OutEdges(3).end()->neighbour,
                    weight_graph.OutEdges(3).end()->edge_data);

        // test set_num_vertices
        {
            OlapOnDisk<Empty> graph_two;
            graph_two.set_num_vertices(3);
            UT_EXPECT_EQ(graph_two.NumVertices(), 3);
        }
    } else {
        OlapOnDisk<double> graph;
        Weight_Config weight_config(argc, argv);
        UT_EXPECT_EQ(weight_config.GetType(), TEXT_FILE);
        weight_config.id_mapping = id_mappings;
        weight_config.input_dir = input_dirs;
        graph.Load(weight_config, MAKE_SYMMETRIC);

        // test ParallelBitset Data
        auto active_test = graph.AllocVertexSubset();
        active_test.Add(4);
        UT_EXPECT_EQ(active_test.Data()[0], 16);

        auto active = graph.AllocVertexSubset();
        active.Fill();
        UT_EXPECT_EQ(active.Size(), 21);
        auto label = graph.AllocVertexArray<size_t>();
        UT_EXPECT_EQ(graph.NumVertices(), 21);
        UT_EXPECT_EQ(graph.NumEdges(), 70);
        // test InEdges
        label.Fill(-1);
        graph.ProcessVertexActive<size_t>([&](size_t v) {
            if (graph.InDegree(v) == 0) return 0;
            for (auto& edge : graph.InEdges(v)) {
                size_t dst = edge.neighbour;
                graph.AcquireVertexLock(dst);
                if (label[dst] == -1) {
                    label[dst] = 1;
                }
                graph.ReleaseVertexLock(dst);
            }
            return 0;
        }, active);
        UT_EXPECT_EQ(label[1], 1);
    }
    system("rm -rf ./ut_data");
}

INSTANTIATE_TEST_CASE_P(TestOlapOnDisk, TestOlapOnDisk,
    testing::Values(ParamConfig{"text", "./ut_data/test_data", 0},
                    ParamConfig{"text", "./ut_data/", 0},
                    ParamConfig{"text", "./ut_data/", 1},
                    ParamConfig{"text", "./ut_data/test_data", 1}));
