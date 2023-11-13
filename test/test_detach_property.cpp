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

#include <map>
#include <string>
#include "gtest/gtest.h"
#include "lgraph/lgraph.h"
#include "restful/server/json_convert.h"
#include "core/data_type.h"
#include "fma-common/string_formatter.h"
#include "fma-common/fma_stream.h"
#include "./test_tools.h"
using namespace lgraph_api;

class TestDetachProperty : public TuGraphTest {};

TEST_F(TestDetachProperty, normal) {
    const std::string& dir = "./testdb";
    lgraph::AutoCleanDir cleaner(dir);
    Galaxy galaxy(dir, false, true);
    galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
    auto graph = galaxy.OpenGraph(lgraph::_detail::DEFAULT_GRAPH_DB_NAME);
    VertexOptions v_options;
    v_options.detach_property = true;
    v_options.primary_field = "id";
    UT_EXPECT_EQ(v_options.to_string(), "detach_property: 1, primary_field: id");
    bool ret;
    ret = graph.AddVertexLabel(std::string("v1"),
                         std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                 {"content", FieldType::STRING, true},
                                                       {"blob", FieldType::BLOB, true}}),
                         v_options);
    UT_EXPECT_TRUE(ret);
    ret = graph.AddVertexLabel(std::string("v2"),
                         std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                 {"content", FieldType::STRING, true}}),
                         v_options);
    v_options.clear();

    UT_EXPECT_TRUE(ret);
    EdgeOptions e_options;
    e_options.detach_property = true;
    e_options.edge_constraints = {{"v1", "v2"}};
    UT_EXPECT_EQ(e_options.to_string(),
                 "detach_property: 1, edge_constraints: [v1 -> v2], temporal_field: , "
                 "temporal_field_order: ASC");
    ret = graph.AddEdgeLabel("e1", std::vector<FieldSpec>(
                                       {{"weight", FieldType::INT16, false}}),
                             e_options);
    UT_EXPECT_TRUE(ret);
    e_options.clear();

    auto txn = graph.CreateWriteTxn();
    auto v1 = txn.AddVertex(
        "v1", {"id", "content", "blob"},
        std::vector<FieldData>({FieldData::String("v1"),
                                FieldData::String("v1"), FieldData::Blob("blob")}));

    auto v2 = txn.AddVertex(
        "v2", {"id", "content"},
        std::vector<FieldData>({FieldData::String("v2"), FieldData::String("v2")}));
    auto e1 = txn.AddEdge(v1, v2, "e1", {"weight"}, std::vector<FieldData>{FieldData::Int16(1)});
    txn.Commit();

    // read property
    {
        auto txn = graph.CreateReadTxn();
        auto iter = txn.GetVertexIterator(v1);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v1");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v1");
        UT_EXPECT_EQ(iter.GetField("blob").AsBlob(), "blob");

        auto allVFields = iter.GetAllFields();
        UT_EXPECT_EQ(allVFields.at("id").AsString(), "v1");
        UT_EXPECT_EQ(allVFields.at("content").AsString(), "v1");
        std::string str = R"(V[0]:v1 {id=v1, content=v1, blob=[BLOB]}
	 -> E[0]:e1, DST = 1, EP = {weight=1}
	 <- [])"; // NOLINT
        UT_EXPECT_EQ(iter.ToString(), str);

        iter = txn.GetVertexIterator(v2);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v2");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v2");
        auto vfields = iter.GetFields(std::vector<std::string>{"id", "content"});
        UT_EXPECT_EQ(vfields[0].AsString(), "v2");
        UT_EXPECT_EQ(vfields[1].AsString(), "v2");

        auto eiter = txn.GetOutEdgeIterator(e1);
        UT_EXPECT_EQ(eiter.GetField("weight").AsInt16(), 1);
        auto allEFields = eiter.GetAllFields();
        UT_EXPECT_EQ(allEFields.at("weight").AsInt16(), 1);
        auto efields = eiter.GetFields(std::vector<std::string>{"weight"});
        UT_EXPECT_EQ(efields[0].AsInt16(), 1);
    }
    // update property
    {
        auto txn = graph.CreateWriteTxn();
        auto iter = txn.GetVertexIterator(v1);
        iter.SetField("content", FieldData::String("v11"));
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v11");

        iter = txn.GetVertexIterator(v2);
        iter.SetField("content", FieldData::String("v22"));
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v22");

        auto eiter = txn.GetOutEdgeIterator(e1);
        eiter.SetField("weight", FieldData::Int16(2));
        UT_EXPECT_EQ(eiter.GetField("weight").AsInt16(), 2);
        txn.Commit();
    }
    // read property
    {
        auto txn = graph.CreateReadTxn();
        auto iter = txn.GetVertexIterator(v1);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v1");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v11");
        UT_EXPECT_EQ(txn.GetTxn()->GetVertexField(iter.GetId(), std::string("id")),
                     FieldData::String("v1"));
        auto fields = txn.GetTxn()->GetVertexFields(
            iter.GetId(), std::vector<std::string>{"id", "content"});
        UT_EXPECT_EQ(fields[0].AsString(), "v1");
        UT_EXPECT_EQ(fields[1].AsString(), "v11");

        iter = txn.GetVertexIterator(v2);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v2");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v22");

        auto eiter = txn.GetOutEdgeIterator(e1);
        UT_EXPECT_EQ(eiter.GetField("weight").AsInt16(), 2);
        UT_EXPECT_EQ(eiter.ToString(), "E[0]: DST = 1, EP = {weight=2}");
    }
    // delete
    {
        auto txn = graph.CreateWriteTxn();
        auto iter = txn.GetVertexIterator(v1);
        iter.Delete();
        iter = txn.GetVertexIterator(v2);
        iter.Delete();
        txn.Commit();
        txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE((txn.Count() == std::pair<uint64_t, uint64_t>(0, 0)));
    }
    // recreate
    {
        txn = graph.CreateWriteTxn();
        auto v1 = txn.AddVertex(
            "v1", {"id", "content"},
            std::vector<FieldData>({FieldData::String("v1"), FieldData::String("v1")}));

        auto v2 = txn.AddVertex(
            "v2", {"id", "content"},
            std::vector<FieldData>({FieldData::String("v2"), FieldData::String("v2")}));
        auto e1 =
            txn.AddEdge(v1, v2, "e1", {"weight"}, std::vector<FieldData>{FieldData::Int16(1)});
        txn.Commit();

        auto txn = graph.CreateWriteTxn();
        auto iter = txn.GetVertexIterator(v1);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v1");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v1");

        iter = txn.GetVertexIterator(v2);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v2");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v2");

        auto eiter = txn.GetOutEdgeIterator(e1);
        UT_EXPECT_EQ(eiter.GetField("weight").AsInt16(), 1);

        UT_EXPECT_EQ(txn.GetTxn()->GetEdgeField(e1, std::string("weight")),
                     FieldData::Int16(1));

        auto fields = txn.GetTxn()->GetEdgeFields(
            e1, std::vector<std::string>{"weight"});
        UT_EXPECT_EQ(fields[0].AsInt16(), 1);

        eiter.Delete();
        txn.Commit();
    }

    graph.DropAllVertex();
    {
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE((txn.Count() == std::pair<uint64_t, uint64_t>(0, 0)));
    }
}

TEST_F(TestDetachProperty, reopen) {
    const std::string& dir = "./testdb";
    lgraph::AutoCleanDir cleaner(dir);
    int64_t v1, v2;
    EdgeUid e1;
    {
        Galaxy galaxy(dir, false, true);
        galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS);
        auto graph = galaxy.OpenGraph(lgraph::_detail::DEFAULT_GRAPH_DB_NAME);
        VertexOptions v_options;
        v_options.detach_property = true;
        v_options.primary_field = "id";
        UT_EXPECT_EQ(v_options.to_string(), "detach_property: 1, primary_field: id");
        bool ret;
        ret = graph.AddVertexLabel(std::string("v1"),
                                   std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                           {"content", FieldType::STRING, true},
                                                           {"blob", FieldType::BLOB, true}}),
                                   v_options);
        UT_EXPECT_TRUE(ret);
        ret = graph.AddVertexLabel(std::string("v2"),
                                   std::vector<FieldSpec>({{"id", FieldType::STRING, false},
                                                           {"content", FieldType::STRING, true}}),
                                   v_options);
        v_options.clear();

        UT_EXPECT_TRUE(ret);
        EdgeOptions e_options;
        e_options.detach_property = true;
        e_options.edge_constraints = {{"v1", "v2"}};
        UT_EXPECT_EQ(e_options.to_string(),
                     "detach_property: 1, edge_constraints: [v1 -> v2], temporal_field: , "
                     "temporal_field_order: ASC");
        ret = graph.AddEdgeLabel(
            "e1", std::vector<FieldSpec>({{"weight", FieldType::INT16, false}}), e_options);
        UT_EXPECT_TRUE(ret);

        auto txn = graph.CreateWriteTxn();
        v1 =
            txn.AddVertex("v1", {"id", "content", "blob"},
                          std::vector<FieldData>({FieldData::String("v1"), FieldData::String("v1"),
                                                  FieldData::Blob("blob")}));

        v2 = txn.AddVertex(
            "v2", {"id", "content"},
            std::vector<FieldData>({FieldData::String("v2"), FieldData::String("v2")}));
        e1 =
            txn.AddEdge(v1, v2, "e1",
                        {"weight"}, std::vector<FieldData>{FieldData::Int16(1)});
        txn.Commit();
    }

    {
        Galaxy galaxy(dir, false, true);
        galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS);
        auto graph = galaxy.OpenGraph(lgraph::_detail::DEFAULT_GRAPH_DB_NAME);

        auto txn = graph.CreateReadTxn();
        auto iter = txn.GetVertexIterator(v1);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v1");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v1");
        UT_EXPECT_EQ(iter.GetField("blob").AsBlob(), "blob");

        iter = txn.GetVertexIterator(v2);
        UT_EXPECT_EQ(iter.GetField("id").AsString(), "v2");
        UT_EXPECT_EQ(iter.GetField("content").AsString(), "v2");

        auto eiter = txn.GetOutEdgeIterator(e1);
        UT_EXPECT_EQ(eiter.GetField("weight").AsInt16(), 1);
    }
}

void WriteFile(const std::string &name, const std::string &content) {
    fma_common::OutputFmaStream out(name);
    out.Write(content.data(), content.size());
}

TEST_F(TestDetachProperty, onlineImport) {
    std::string host = "127.0.0.1";
    int port = 6464;
    int rpc_port = 16464;
    std::string db_dir = "./lgraph_db";
    std::string dir = "./import_data";
    lgraph::AutoCleanDir cleaner(dir);
    lgraph::AutoCleanDir db_cleaner(db_dir);
    std::string config_file = dir + "/import.conf";
    std::string v1 = dir + "/node1.csv";
    std::string v2 = dir + "/node2.csv";
    std::string e1 = dir + "/edge1.csv";
    std::string e2 = dir + "/edge2.csv";

    std::string config_jsonline = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "id2", "type":"INT64", "unique":true},
                {"name" : "comment", "type":"STRING", "optional":true}
            ],
            "detach_property" : true
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "name", "type":"STRING", "optional":true},
                {"name" : "weight", "type":"FLOAT"}
            ],
            "detach_property" : true
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","id2","SKIP","comment"]
        },
        {
            "path" : "./import_data/node2.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","id2", "comment"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","name","SKIP", "weight"]
        },
        {
            "path" : "./import_data/edge2.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","name","weight"]
        }
    ]
}
                      )";

    auto StartServer = [&]() {
        std::string server_cmd = UT_FMT(
            "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
            " --enable_backup_log true --host 127.0.0.1 --verbose 1 --directory {}" ,
            port, rpc_port, db_dir);
        UT_LOG() << "cmd: " << server_cmd;
        auto server = std::unique_ptr<lgraph::SubProcess>(new lgraph::SubProcess(server_cmd));
        if (!server->ExpectOutput("Server started.")) {
            UT_WARN() << "Server failed to start, stderr:\n" << server->Stderr();
        }
        return server;
    };
    auto TryImport = [&](const std::string &expect_output, int expect_ec,
                         std::string config_file_, bool continue_on_error = false) {
        std::string import_cmd = UT_FMT(
            "./lgraph_import --online true --config_file \"{}\" -r http://127.0.0.1:{} "
            "--continue_on_error {} -u {} -p {}",
            config_file_, port, continue_on_error, lgraph::_detail::DEFAULT_ADMIN_NAME,
            lgraph::_detail::DEFAULT_ADMIN_PASS);

        UT_LOG() << "cmd: " << import_cmd;
        lgraph::SubProcess proc(import_cmd);
        UT_EXPECT_TRUE(proc.ExpectOutput(expect_output, 10 * 1000));
        proc.Wait();
        UT_EXPECT_EQ(expect_ec, proc.GetExitCode());
    };
    auto ValidateGraph = [&](const std::function<void(lgraph_api::GraphDB &)> &validate) {
        lgraph_api::Galaxy galaxy(db_dir);
        galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS);
        lgraph_api::GraphDB graph = galaxy.OpenGraph("default");
        validate(graph);
    };

    {
        UT_LOG() << "Testing import jsonline";
        WriteFile(config_file, config_jsonline);
        WriteFile(v1, R"(
[1,  2, "test", "aaaa"]
[2,  3, null, "aaaa"]
["3",4, "test", null]
["4",5, "test", "aaaa"]
                      )");
        WriteFile(v2, R"(
[5,  6, "test", "cccc"]
[6,  7, null, ""]
["7",8, "test", null]
["8",9, "test", "aaaa"]
                      )");
        WriteFile(e1, R"(
[1,  2, "test", "skip",  5]
[1,  3, null,   "skip",  7]
[1,  4, "test", "skip",  8]
[1,  5, "",     "",      9]
                      )");
        WriteFile(e2, R"(
[2,  5, "test", 10]
[3,  4, null,   11]
[4,  3, "test", 12]
[5,  2, "",     13]
                      )");
        db_cleaner.Clean();
        auto server = StartServer();
        TryImport("Import finished", 0, config_file);
        server.reset();
        ValidateGraph([](lgraph_api::GraphDB &g) {
            auto txn = g.CreateReadTxn();
            UT_EXPECT_EQ(txn.GetNumVertices(), 8);
            auto iter1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
            UT_EXPECT_EQ(iter1.GetField("id2").AsInt64(), 2);

            auto iter2 = txn.GetVertexByUniqueIndex("node", "id", FieldData(3));
            UT_EXPECT_EQ(iter2.GetField("id2").AsInt64(), 4);
            UT_EXPECT_TRUE(iter2.GetField("comment").is_null());
            auto eiter1 = iter2.GetOutEdgeIterator();
            UT_EXPECT_EQ(eiter1.GetField("weight").AsFloat(), 11);
            UT_EXPECT_TRUE(eiter1.GetField("name").is_null());

            auto eiter2 = iter2.GetInEdgeIterator();
            UT_EXPECT_EQ(eiter2.GetField("weight").AsFloat(), 7);
            eiter2.Next();
            UT_EXPECT_TRUE(eiter2.IsValid());
            UT_EXPECT_EQ(eiter2.GetField("weight").AsFloat(), 12);
            eiter2.Next();
            UT_EXPECT_TRUE(!eiter2.IsValid());
        });
    }
}
