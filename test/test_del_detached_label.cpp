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

#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "lgraph/lgraph.h"
#include "core/defs.h"
#include "core/data_type.h"
using namespace lgraph_api;

class TestDelDetachedLabel : public TuGraphTest{};

TEST_F(TestDelDetachedLabel, delete_edge) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    Galaxy galaxy(path);
    galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
    GraphDB db = galaxy.OpenGraph("default");
    VertexOptions vo;
    vo.primary_field = "id";
    vo.detach_property = true;
    UT_EXPECT_TRUE(db.AddVertexLabel("node1",
                                     std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                             {"int64", FieldType::INT64, true}}),
                                     vo));
    UT_EXPECT_TRUE(db.AddVertexLabel("node2",
                                     std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                             {"int64", FieldType::INT64, true}}),
                                     vo));
    EdgeOptions eo;
    eo.detach_property = true;
    UT_EXPECT_TRUE(db.AddEdgeLabel("edge1",
                                   std::vector<FieldSpec>({{"id", FieldType::INT32, false}}),
                                   eo));
    UT_EXPECT_TRUE(db.AddEdgeLabel("edge2",
                                   std::vector<FieldSpec>({}),
                                   eo));
    UT_EXPECT_TRUE(db.AddEdgeLabel("edge3",
                                   std::vector<FieldSpec>({{"id", FieldType::INT32, false}}),
                                   eo));
    std::vector<std::string> vp({"id", "int64"});
    std::vector<std::string> ep({"id"});

    int count = 100;
    std::vector<int64_t> node1_vids, node2_vids;
    auto txn = db.CreateWriteTxn();
    UT_EXPECT_EQ(txn.GetEdgeLabelId("edge1"), 0);
    UT_EXPECT_EQ(txn.GetEdgeLabelId("edge2"), 1);
    UT_EXPECT_EQ(txn.GetEdgeLabelId("edge3"), 2);
    for (int32_t i = 0; i < count; i++) {
        auto vid1 = txn.AddVertex(
            std::string("node1"), vp,
            {FieldData::Int32(i), FieldData::Int64(i)});
        node1_vids.push_back(vid1);
        auto vid2 = txn.AddVertex(
            std::string("node2"), vp,
            {FieldData::Int32(i), FieldData::Int64(i)});
        node2_vids.push_back(vid2);
    }
    for (int32_t i = 0; i < count; i++) {
        txn.AddEdge(node1_vids[i], node2_vids[i], std::string("edge1"),
                    ep, {FieldData::Int32(i)});
        txn.AddEdge(node1_vids[i], node2_vids[i], std::string("edge2"),
                    std::vector<std::string>{}, std::vector<FieldData>{});
        txn.AddEdge(node1_vids[i], node2_vids[i], std::string("edge3"),
                    ep, {FieldData::Int32(i)});
    }
    txn.Commit();
    size_t modified = 0;
    UT_EXPECT_TRUE(db.DeleteEdgeLabel("edge1", &modified));
    UT_EXPECT_EQ(modified, count);
    UT_EXPECT_TRUE(db.DeleteEdgeLabel("edge2", &modified));
    UT_EXPECT_EQ(modified, count);
    UT_EXPECT_TRUE(db.DeleteEdgeLabel("edge3", &modified));
    UT_EXPECT_EQ(modified, count);
}

TEST_F(TestDelDetachedLabel, common) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    Galaxy galaxy(path);
    galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
    GraphDB db = galaxy.OpenGraph("default");
    VertexOptions vo;
    vo.primary_field = "id";
    vo.detach_property = true;
    UT_EXPECT_TRUE(db.AddVertexLabel("node1",
                                     std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                             {"int64", FieldType::INT64, true}}),
                                     vo));
    UT_EXPECT_TRUE(db.AddVertexLabel("node2",
                                     std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                             {"int64", FieldType::INT64, true}}),
                                     vo));
    UT_EXPECT_TRUE(db.AddVertexLabel("node3",
                                     std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                             {"int64", FieldType::INT64, true}}),
                                     vo));
    EdgeOptions eo;
    eo.detach_property = true;
    UT_EXPECT_TRUE(db.AddEdgeLabel("edge1",
                                   std::vector<FieldSpec>({{"id", FieldType::INT32, false}}),
                                   eo));
    UT_EXPECT_TRUE(db.AddEdgeLabel("edge2",
                                   std::vector<FieldSpec>({}),
                                   eo));

    std::vector<std::string> vp({"id", "int64"});
    std::vector<std::string> ep({"id"});
    int count = 100;
    std::vector<int64_t> node1_vids, node2_vids, node3_vids;
    auto txn = db.CreateWriteTxn();
    for (int32_t i = 0; i < count; i++) {
        auto vid1 = txn.AddVertex(
            std::string("node1"), vp,
            {FieldData::Int32(i), FieldData::Int64(i)});
        node1_vids.push_back(vid1);
        auto vid2 = txn.AddVertex(
            std::string("node2"), vp,
            {FieldData::Int32(i), FieldData::Int64(i)});
        node2_vids.push_back(vid2);
        auto vid3 = txn.AddVertex(
            std::string("node3"), vp,
            {FieldData::Int32(i), FieldData::Int64(i)});
        node3_vids.push_back(vid3);
    }
    for (int32_t i = 0; i < count; i++) {
        txn.AddEdge(node1_vids[i], node2_vids[i], std::string("edge1"),
                    ep, {FieldData::Int32(i)});
        txn.AddEdge(node1_vids[i], node2_vids[i], std::string("edge2"),
                    std::vector<std::string>{}, std::vector<FieldData>{});
        txn.AddEdge(node2_vids[i], node3_vids[i], std::string("edge1"),
                    ep, {FieldData::Int32(i)});
    }
    int big_vertex = 10000;
    for (int i = 0; i < big_vertex; i++) {
        txn.AddEdge(node1_vids[0], node2_vids[0], std::string("edge1"),
                    ep, {FieldData::Int32(i)});
    }
    txn.Commit();
    auto count_num = [&db]() {
        auto txn = db.CreateReadTxn();
        int node1_count = 0;
        int node2_count = 0;
        int node3_count = 0;
        int edge1_count = 0;
        int edge2_count = 0;
        for (auto viter = txn.GetVertexIterator(); viter.IsValid(); viter.Next()) {
            if (viter.GetLabel() == "node1") {
                node1_count++;
            } else if (viter.GetLabel() == "node2") {
                node2_count++;
            } else if (viter.GetLabel() == "node3") {
                node3_count++;
            }
            for (auto eiter = viter.GetOutEdgeIterator(); eiter.IsValid(); eiter.Next()) {
                if (eiter.GetLabel() == "edge1") {
                    edge1_count++;
                } else if (eiter.GetLabel() == "edge2") {
                    edge2_count++;
                }
            }
        }
        return std::make_tuple(node1_count, node2_count, node3_count, edge1_count, edge2_count);
    };
    int node1_count = 0;
    int node2_count = 0;
    int node3_count = 0;
    int edge1_count = 0;
    int edge2_count = 0;
    std::tie(node1_count, node2_count, node3_count, edge1_count, edge2_count) = count_num();
    UT_EXPECT_EQ(node1_count, count);
    UT_EXPECT_EQ(node2_count, count);
    UT_EXPECT_EQ(node3_count, count);
    UT_EXPECT_EQ(edge1_count, 2*count + big_vertex);
    UT_EXPECT_EQ(edge2_count, count);
    txn = db.CreateReadTxn();
    std::vector<std::tuple<bool, std::string, int64_t>> expect{
        {true, "node1", count},
        {true, "node2", count},
        {true, "node3", count},
        {false, "edge1", 2*count + big_vertex},
        {false, "edge2", count}
    };
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();
    size_t modified = 0;
    UT_EXPECT_TRUE(db.DeleteVertexLabel("node1", &modified));
    UT_EXPECT_EQ(modified, count);
    std::tie(node1_count, node2_count, node3_count, edge1_count, edge2_count) = count_num();
    UT_EXPECT_EQ(node1_count, 0);
    UT_EXPECT_EQ(node2_count, count);
    UT_EXPECT_EQ(node3_count, count);
    UT_EXPECT_EQ(edge1_count, count);
    UT_EXPECT_EQ(edge2_count, 0);
    txn = db.CreateReadTxn();
    expect = {
        {true, "node2", count},
        {true, "node3", count},
        {false, "edge1", count},
        {false, "edge2", 0}
    };
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();
    UT_EXPECT_TRUE(db.DeleteEdgeLabel("edge1", &modified));
    UT_EXPECT_EQ(modified, count);
    std::tie(node1_count, node2_count, node3_count, edge1_count, edge2_count) = count_num();
    UT_EXPECT_EQ(node1_count, 0);
    UT_EXPECT_EQ(node2_count, count);
    UT_EXPECT_EQ(node3_count, count);
    UT_EXPECT_EQ(edge1_count, 0);
    UT_EXPECT_EQ(edge2_count, 0);
    txn = db.CreateReadTxn();
    expect = {
        {true, "node2", count},
        {true, "node3", count},
        {false, "edge2", 0}
    };
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();

    UT_EXPECT_TRUE(db.DeleteEdgeLabel("edge2", &modified));
    UT_EXPECT_EQ(modified, 0);
    txn = db.CreateReadTxn();
    expect = {
        {true, "node2", count},
        {true, "node3", count}
    };
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();

    UT_EXPECT_TRUE(db.AddEdgeLabel("edge1",
                                   std::vector<FieldSpec>({{"id", FieldType::INT32, false}}),
                                   eo));
    UT_EXPECT_TRUE(db.AddEdgeLabel("edge2",
                                   std::vector<FieldSpec>({}),
                                   eo));
    txn = db.CreateWriteTxn();
    for (int32_t i = 0; i < count; i++) {
        txn.AddEdge(node2_vids[i], node3_vids[i], std::string("edge1"),
                    ep, {FieldData::Int32(i)});
        txn.AddEdge(node3_vids[i], node2_vids[i], std::string("edge1"),
                    ep, {FieldData::Int32(i)});
        txn.AddEdge(node2_vids[i], node3_vids[i], std::string("edge2"),
                    std::vector<std::string>{}, std::vector<FieldData>{});
        txn.AddEdge(node3_vids[i], node2_vids[i], std::string("edge2"),
                    std::vector<std::string>{}, std::vector<FieldData>{});
    }
    int point_self = 100;
    for (int i = 0; i < point_self; i++) {
        txn.AddEdge(node2_vids[0], node2_vids[0], std::string("edge1"),
                    ep, {FieldData::Int32(i)});
    }
    txn.Commit();
    std::tie(node1_count, node2_count, node3_count, edge1_count, edge2_count) = count_num();
    UT_EXPECT_EQ(node1_count, 0);
    UT_EXPECT_EQ(node2_count, count);
    UT_EXPECT_EQ(node3_count, count);
    UT_EXPECT_EQ(edge1_count, 2*count + point_self);
    UT_EXPECT_EQ(edge2_count, 2*count);
    txn = db.CreateReadTxn();
    expect = {
        {true, "node2", count},
        {true, "node3", count},
        {false, "edge1", 2*count + point_self},
        {false, "edge2", 2*count}
    };
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();

    UT_EXPECT_TRUE(db.DeleteEdgeLabel("edge1", &modified));
    UT_EXPECT_EQ(modified, 2*count + point_self);
    std::tie(node1_count, node2_count, node3_count, edge1_count, edge2_count) = count_num();
    UT_EXPECT_EQ(node1_count, 0);
    UT_EXPECT_EQ(node2_count, count);
    UT_EXPECT_EQ(node3_count, count);
    UT_EXPECT_EQ(edge1_count, 0);
    UT_EXPECT_EQ(edge2_count, 2*count);
    txn = db.CreateReadTxn();
    expect = {
        {true, "node2", count},
        {true, "node3", count},
        {false, "edge2", 2*count}
    };
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();

    UT_EXPECT_TRUE(db.DeleteVertexLabel("node2", &modified));
    UT_EXPECT_EQ(modified, count);
    std::tie(node1_count, node2_count, node3_count, edge1_count, edge2_count) = count_num();
    UT_EXPECT_EQ(node1_count, 0);
    UT_EXPECT_EQ(node2_count, 0);
    UT_EXPECT_EQ(node3_count, count);
    UT_EXPECT_EQ(edge1_count, 0);
    UT_EXPECT_EQ(edge2_count, 0);
    txn = db.CreateReadTxn();
    expect = {
        {true, "node3", count},
        {false, "edge2", 0}
    };
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();
    UT_EXPECT_TRUE(db.DeleteVertexLabel("node3", &modified));
    UT_EXPECT_EQ(modified, count);
    UT_EXPECT_TRUE(db.DeleteEdgeLabel("edge2", &modified));
    UT_EXPECT_EQ(modified, 0);
    std::tie(node1_count, node2_count, node3_count, edge1_count, edge2_count) = count_num();
    UT_EXPECT_EQ(node1_count, 0);
    UT_EXPECT_EQ(node2_count, 0);
    UT_EXPECT_EQ(node3_count, 0);
    UT_EXPECT_EQ(edge1_count, 0);
    UT_EXPECT_EQ(edge2_count, 0);
    txn = db.CreateReadTxn();
    expect = {};
    UT_EXPECT_EQ(expect, txn.CountDetail());
    txn.Abort();
}
