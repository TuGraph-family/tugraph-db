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
#include "lgraph/olap_on_db.h"
#include "fma-common/utils.h"
#include "import/import_v3.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

class TestOlapOnDB : public TuGraphTest {};

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

template <typename T>
void ClearCsvFiles(const T& data) {
    fma_common::OutputFmaStream stream;
    for (auto& kv : data) {
        const std::string& file_name = kv;
        fma_common::file_system::RemoveDir(file_name.c_str());
        UT_LOG() << "  " << file_name << " deleted";
    }
}
static void WriteOlapDbFiles() {
static const std::vector<std::pair<std::string, std::string>> data_import = {
    {"test_olap_on_db.conf",
     R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "value", "type" : "STRING", "optional": true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type" : "DOUBLE"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "test_vertices.csv",
            "format" : "CSV",
            "label" : "node",
            "header" : 0,
            "columns" : ["id"]
        },
        {
            "path" : "test_weighted.csv",
            "format" : "CSV",
            "label" : "edge",
            "header" : 0,
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID", "weight"]
        }
    ]
}
)"},
    {"test_vertices.csv",
     R"(0
        1
        2
        3
        4
        5
        6
        7
        8
        9
        10
        11
        12
        13
        14
        15
        16
        17
        18
        19
        20

)"},
    {"test_weighted.csv",
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

/**
 * \brief   Default Parser for weighted edges for graph.
 *
 * \return  Edge is converted into graph or not.
 */
template <typename VertexData>
std::function<void(VertexIterator &, VertexData &)> vertex_extract =
        [] (VertexIterator &vit, VertexData &vertex_data) {
    vertex_data = 2;
    return true;
};

std::function<bool(VertexIterator &)> vertex_filter =
        [] (VertexIterator &vid) -> bool{
    if (vid.GetId() == 0) {
        return false;
    }
    return true;
};

TEST_F(TestOlapOnDB, OlapOnDB) {
    // configure test data
    WriteOlapDbFiles();
    std::string db_path = "./testdb_olap";
    lgraph::import_v3::Importer::Config config;
    config.config_file = "./test_olap_on_db.conf";
    config.db_dir = db_path;
    config.delete_if_exists = true;
    config.graph = "default";

    config.parse_block_threads = 1;
    config.parse_file_threads = 1;
    config.generate_sst_threads = 1;
    lgraph::import_v3::Importer importer(config);
    importer.DoImportOffline();
    Galaxy g(db_path);
    g.SetCurrentUser("admin", "73@TuGraph");
    GraphDB db = g.OpenGraph("default");
    auto txn = db.CreateReadTxn();

    // test ConstructWithVid()
    OlapOnDB<double> test_db_one(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED,
                                 nullptr, edge_convert_default<double>);
    UT_EXPECT_EQ(test_db_one.OutDegree(0), 5);
    UT_EXPECT_EQ(test_db_one.InDegree(0), 5);
    UT_EXPECT_EQ(test_db_one.OutDegree(6), 2);
    UT_EXPECT_EQ(test_db_one.InDegree(6), 2);
    UT_EXPECT_EQ(test_db_one.OutDegree(17), 4);
    UT_EXPECT_EQ(test_db_one.InDegree(17), 4);
    UT_EXPECT_EQ(test_db_one.OutDegree(20), 2);
    UT_EXPECT_EQ(test_db_one.InDegree(20), 2);
    auto active = test_db_one.AllocVertexSubset();
    active.Fill();
    UT_EXPECT_EQ(active.Size(), 21);
    auto label = test_db_one.AllocVertexArray<size_t>();
    UT_EXPECT_EQ(test_db_one.NumVertices(), 21);
    UT_EXPECT_EQ(test_db_one.NumEdges(), 70);
    test_db_one.ProcessVertexActive<size_t>(
        [&](size_t vi) {
            label[vi] = -1;
            return 1;
        },
        active);
    UT_EXPECT_EQ(label[0], -1);
    for (auto& edge : test_db_one.OutEdges(0)) {
        UT_EXPECT_EQ(edge.edge_data, 1.0);
    }

    auto vertex_lists = test_db_one.ExtractVertexData(vertex_extract<size_t>);
    UT_EXPECT_EQ(vertex_lists[0], 2);

    OlapOnDB<Empty> unparallel_one(db, txn, SNAPSHOT_UNDIRECTED);
    UT_EXPECT_EQ(unparallel_one.OutDegree(0), 5);
    UT_EXPECT_EQ(unparallel_one.InDegree(0), 5);
    UT_EXPECT_EQ(unparallel_one.OutDegree(6), 2);
    UT_EXPECT_EQ(unparallel_one.InDegree(6), 2);
    UT_EXPECT_EQ(unparallel_one.OutDegree(17), 4);
    UT_EXPECT_EQ(unparallel_one.InDegree(17), 4);
    UT_EXPECT_EQ(unparallel_one.OutDegree(20), 2);
    UT_EXPECT_EQ(unparallel_one.InDegree(20), 2);

    OlapOnDB<double> unparallel_db_two(db, txn,
                     SNAPSHOT_UNDIRECTED, nullptr, edge_convert_weight<double>);
    UT_EXPECT_EQ(unparallel_db_two.OutDegree(0), 5);
    UT_EXPECT_EQ(unparallel_db_two.InDegree(0), 5);
    UT_EXPECT_EQ(unparallel_db_two.OutDegree(6), 2);
    UT_EXPECT_EQ(unparallel_db_two.InDegree(6), 2);
    UT_EXPECT_EQ(unparallel_db_two.OutDegree(17), 4);
    UT_EXPECT_EQ(unparallel_db_two.InDegree(17), 4);
    UT_EXPECT_EQ(unparallel_db_two.OutDegree(20), 2);
    UT_EXPECT_EQ(unparallel_db_two.InDegree(20), 2);

    for (auto& edge : unparallel_db_two.OutEdges(0)) {
        UT_EXPECT_EQ(edge.edge_data, edge.neighbour);
    }

    OlapOnDB<double> parallel_one(db, txn, SNAPSHOT_PARALLEL, nullptr, edge_convert_weight<double>);
    UT_EXPECT_EQ(parallel_one.OutDegree(0), 5);
    UT_EXPECT_EQ(parallel_one.InDegree(0), 0);
    UT_EXPECT_EQ(parallel_one.OutDegree(6), 0);
    UT_EXPECT_EQ(parallel_one.InDegree(6), 2);
    UT_EXPECT_EQ(parallel_one.OutDegree(17), 1);
    UT_EXPECT_EQ(parallel_one.InDegree(17), 3);
    UT_EXPECT_EQ(parallel_one.OutDegree(20), 0);
    UT_EXPECT_EQ(parallel_one.InDegree(20), 2);

    for (auto& edge : parallel_one.OutEdges(10)) {
        UT_EXPECT_EQ(edge.edge_data, edge.neighbour);
    }

    OlapOnDB<double> parallel_two(db, txn, SNAPSHOT_PARALLEL,
                                  nullptr, edge_convert_default<double>);
    UT_EXPECT_EQ(parallel_two.OutDegree(0), 5);
    UT_EXPECT_EQ(parallel_two.InDegree(0), 0);
    UT_EXPECT_EQ(parallel_two.OutDegree(6), 0);
    UT_EXPECT_EQ(parallel_two.InDegree(6), 2);
    UT_EXPECT_EQ(parallel_two.OutDegree(17), 1);
    UT_EXPECT_EQ(parallel_two.InDegree(17), 3);
    UT_EXPECT_EQ(parallel_two.OutDegree(20), 0);
    UT_EXPECT_EQ(parallel_two.InDegree(20), 2);

    for (auto& edge : parallel_two.OutEdges(10)) {
        UT_EXPECT_EQ(edge.edge_data, 1.0);
    }

    // test Construct()
    OlapOnDB<Empty> test_db_two(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_IDMAPPING);
    UT_EXPECT_EQ(test_db_two.OutDegree(0), 5);
    UT_EXPECT_EQ(test_db_two.InDegree(0), 0);
    UT_EXPECT_EQ(test_db_two.OutDegree(6), 0);
    UT_EXPECT_EQ(test_db_two.InDegree(6), 2);
    UT_EXPECT_EQ(test_db_two.OutDegree(17), 1);
    UT_EXPECT_EQ(test_db_two.InDegree(17), 3);
    UT_EXPECT_EQ(test_db_two.OutDegree(20), 0);
    UT_EXPECT_EQ(test_db_two.InDegree(20), 2);
    UT_EXPECT_EQ(test_db_two.NumVertices(), 21);
    UT_EXPECT_EQ(test_db_two.NumEdges(), 35);
    UT_EXPECT_EQ(test_db_two.MappedVid(0), 0);
    UT_EXPECT_EQ(test_db_two.OriginalVid(0), 0);
    UT_EXPECT_EQ(test_db_two.MappedVid(test_db_two.OriginalVid(4)), 4);

    OlapOnDB<double> unparallel_two(db, txn, SNAPSHOT_UNDIRECTED,
                     vertex_filter, edge_convert_weight<double>);
    UT_EXPECT_EQ(unparallel_two.OutDegree(0), 3);
    UT_EXPECT_EQ(unparallel_two.InDegree(0), 3);
    UT_EXPECT_EQ(unparallel_two.OutDegree(6), 2);
    UT_EXPECT_EQ(unparallel_two.InDegree(6), 2);
    UT_EXPECT_EQ(unparallel_two.OutDegree(13), 2);
    UT_EXPECT_EQ(unparallel_two.InDegree(13), 2);
    UT_EXPECT_EQ(unparallel_two.OutDegree(20), 0);
    UT_EXPECT_EQ(unparallel_two.InDegree(20), 0);
    UT_EXPECT_EQ(unparallel_two.NumVertices(), 20);
    UT_EXPECT_EQ(unparallel_two.NumEdges(), 60);
    UT_EXPECT_EQ(unparallel_two.MappedVid(unparallel_two.OriginalVid(17)), 17);

    std::string vertex_label = "node";
    std::string edge_label = "edge";
    OlapOnDB<Empty> filter_graph(db, txn, SNAPSHOT_PARALLEL,
                [&vertex_label](VertexIterator& vit) {
                return vit.GetLabel() == vertex_label;
            }, [&edge_label](OutEdgeIterator& eit, Empty& edata) {
                return eit.GetLabel() == edge_label;
            });
    UT_EXPECT_EQ(filter_graph.OutDegree(0), 5);
    UT_EXPECT_EQ(filter_graph.InDegree(0), 0);
    UT_EXPECT_EQ(filter_graph.OutDegree(6), 0);
    UT_EXPECT_EQ(filter_graph.InDegree(6), 2);
    UT_EXPECT_EQ(filter_graph.OutDegree(13), 2);
    UT_EXPECT_EQ(filter_graph.InDegree(13), 3);
    UT_EXPECT_EQ(filter_graph.OutDegree(20), 0);
    UT_EXPECT_EQ(filter_graph.InDegree(20), 2);
    UT_EXPECT_EQ(filter_graph.NumVertices(), 21);
    UT_EXPECT_EQ(filter_graph.NumEdges(), 35);
    txn.Commit();

    // WriteToGraphDB
    ParallelVector<size_t> parent = filter_graph.AllocVertexArray<size_t>();
    for (int i = 0; i < parent.Size(); i++) {
        parent[i] = i;
    }
    filter_graph.WriteToGraphDB(parent, "value");
    txn = db.CreateReadTxn();
    auto vit = txn.GetVertexIterator();
    vit.Goto(2);
    UT_EXPECT_EQ(vit.GetField("value").ToString(), "2");
    vit.Goto(20);
    UT_EXPECT_EQ(vit.GetField("value").ToString(), "20");

    // WriteToFile
    std::string file_path = "./test_write_to_db.csv";
    filter_graph.WriteToFile(parent, file_path);

    vertex_label = "id";
    edge_label = "";
    UT_EXPECT_THROW_MSG(
    OlapOnDB<Empty>(db, txn, SNAPSHOT_PARALLEL,
                [&vertex_label](VertexIterator& vit) {
                if (vertex_label == "") return true;
                return vit.GetLabel() == vertex_label;
            }, [&edge_label](OutEdgeIterator& eit, Empty& edata) {
                if (edge_label == "") return true;
                return eit.GetLabel() == edge_label;
            }), "The graph vertex cannot be empty");

    vertex_label = "";
    edge_label = "id";
    UT_EXPECT_THROW_MSG(
    OlapOnDB<Empty>(db, txn, SNAPSHOT_PARALLEL,
                [&vertex_label](VertexIterator& vit) {
                if (vertex_label == "") return true;
                return vit.GetLabel() == vertex_label;
            }, [&edge_label](OutEdgeIterator& eit, Empty& edata) {
                if (edge_label == "") return true;
                return eit.GetLabel() == edge_label;
            }), "The graph edge cannot be empty");

    // test ConstructWithDegree()
    OlapOnDB<Empty> test_db_three(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED);
    UT_EXPECT_EQ(test_db_three.OutDegree(0), 5);
    UT_EXPECT_EQ(test_db_three.InDegree(0), 5);
    UT_EXPECT_EQ(test_db_three.OutDegree(6), 2);
    UT_EXPECT_EQ(test_db_three.InDegree(6), 2);
    UT_EXPECT_EQ(test_db_three.OutDegree(13), 5);
    UT_EXPECT_EQ(test_db_three.InDegree(13), 5);
    UT_EXPECT_EQ(test_db_three.OutDegree(20), 2);
    UT_EXPECT_EQ(test_db_three.InDegree(20), 2);
    UT_EXPECT_EQ(test_db_three.NumVertices(), 21);
    UT_EXPECT_EQ(test_db_three.NumEdges(), 70);
    UT_EXPECT_EQ(test_db_two.MappedVid(3), 3);
    UT_EXPECT_EQ(test_db_two.OriginalVid(7), 7);

    OlapOnDB<Empty> directed_three(db, txn, SNAPSHOT_PARALLEL);
    UT_EXPECT_EQ(directed_three.OutDegree(0), 5);
    UT_EXPECT_EQ(directed_three.InDegree(0), 0);
    UT_EXPECT_EQ(directed_three.OutDegree(6), 0);
    UT_EXPECT_EQ(directed_three.InDegree(6), 2);
    UT_EXPECT_EQ(directed_three.OutDegree(13), 2);
    UT_EXPECT_EQ(directed_three.InDegree(13), 3);
    UT_EXPECT_EQ(directed_three.OutDegree(20), 0);
    UT_EXPECT_EQ(directed_three.InDegree(20), 2);
    UT_EXPECT_EQ(directed_three.NumVertices(), 21);
    UT_EXPECT_EQ(directed_three.NumEdges(), 35);
    UT_EXPECT_EQ(directed_three.MappedVid(1), 1);
    UT_EXPECT_EQ(directed_three.OriginalVid(20), 20);
    txn.Commit();

    // test ExtractVertexData
    auto write_txn = db.CreateWriteTxn();
    {
        OlapOnDB<Empty> test_db_four(db, write_txn, SNAPSHOT_PARALLEL);
        UT_EXPECT_EQ(test_db_four.NumEdges(), 35);
        auto vertex_list = test_db_four.ExtractVertexData(vertex_extract<size_t>);
        UT_EXPECT_EQ(vertex_list[0], 2);
        UT_EXPECT_EQ(vertex_list.Size(), 21);
        UT_EXPECT_EQ(test_db_four.InDegree(0), 0);
    }
    {
        OlapOnDB<Empty> parallel_four(db, write_txn, SNAPSHOT_PARALLEL | SNAPSHOT_IDMAPPING);
        auto extract_list = parallel_four.ExtractVertexData(vertex_extract<size_t>);
        UT_EXPECT_EQ(extract_list[0], 2);
        UT_EXPECT_EQ(extract_list.Size(), 21);
    }
    write_txn.Commit();
    std::vector<std::string> del_filename = {"test_olap_on_db.conf",
                            "test_vertices.csv", "test_weighted.csv", "test_write_to_db.csv"};
    ClearCsvFiles(del_filename);
}
