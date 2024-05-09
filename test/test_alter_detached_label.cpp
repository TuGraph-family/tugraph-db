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

class TestAlterDetachedLabel : public TuGraphTest{};

TEST_F(TestAlterDetachedLabel, edge_add_field) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    {
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        VertexOptions vo;
        vo.primary_field = "id";
        vo.detach_property = true;
        UT_EXPECT_TRUE(db.AddVertexLabel(
            "Person", std::vector<FieldSpec>({{"id", FieldType::INT32, false}}), vo));

        EdgeOptions eo;
        eo.detach_property = true;
        UT_EXPECT_TRUE(db.AddEdgeLabel("Relation",
                                       std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                               {"str", FieldType::STRING, true},
                                                               {"int64", FieldType::INT64, true}}),
                                       eo));

        std::vector<std::string> vp({"id"});
        std::vector<int64_t> vids;
        int count = 10000;
        auto txn = db.CreateWriteTxn();
        for (int32_t i = 0; i < count; i++) {
            auto vid1 = txn.AddVertex(std::string("Person"), vp, {FieldData::Int32(i)});
            vids.emplace_back(vid1);
        }
        std::vector<std::pair<EdgeUid, int32_t>> check;
        std::vector<std::string> ep({"id", "str", "int64"});
        for (int i = 0; i < vids.size() - 1; i++) {
            auto euid = txn.AddEdge(
                vids[i], vids[i + 1], std::string("Relation"), ep,
                {FieldData::Int32(i), FieldData::String(std::to_string(i)), FieldData::Int64(i)});
            check.emplace_back(euid, i);
        }
        txn.Commit();

        FieldSpec fs("addr", FieldType::STRING, true);
        size_t modified = 0;
        UT_EXPECT_TRUE(db.AlterEdgeLabelAddFields("Relation", {fs}, {FieldData()}, &modified));
        UT_EXPECT_EQ(modified, check.size());
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto eiter = txn.GetOutEdgeIterator(item.first);
            UT_EXPECT_EQ(eiter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_EQ(eiter.GetField("str").AsString(), std::to_string(item.second));
            UT_EXPECT_EQ(eiter.GetField("int64").AsInt64(), item.second);
            UT_EXPECT_TRUE(eiter.GetField("addr").IsNull());
        }
        txn.Abort();
        txn = db.CreateWriteTxn();
        for (auto& item : check) {
            auto eiter = txn.GetOutEdgeIterator(item.first);
            eiter.SetField("addr", FieldData::String(std::to_string(item.second)));
        }
        txn.Commit();
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto eiter = txn.GetOutEdgeIterator(item.first);
            UT_EXPECT_EQ(eiter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_EQ(eiter.GetField("str").AsString(), std::to_string(item.second));
            UT_EXPECT_EQ(eiter.GetField("int64").AsInt64(), item.second);
            UT_EXPECT_EQ(eiter.GetField("addr").AsString(), std::to_string(item.second));
        }
        txn.Abort();
    }
    // check re-open
    {
        Galaxy galaxy_tmp(path);
    }
}

TEST_F(TestAlterDetachedLabel, edge_del_field) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    {
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        VertexOptions vo;
        vo.primary_field = "id";
        vo.detach_property = true;
        UT_EXPECT_TRUE(db.AddVertexLabel(
            "Person", std::vector<FieldSpec>({{"id", FieldType::INT32, false}}), vo));

        EdgeOptions eo;
        eo.detach_property = true;
        UT_EXPECT_TRUE(db.AddEdgeLabel("Relation",
                                       std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                               {"str", FieldType::STRING, true},
                                                               {"int64", FieldType::INT64, true}}),
                                       eo));
        UT_EXPECT_TRUE(db.AddEdgeIndex("Relation", "str", lgraph_api::IndexType::NonuniqueIndex));
        UT_EXPECT_TRUE(db.AddEdgeIndex("Relation", "int64", lgraph_api::IndexType::NonuniqueIndex));
        std::vector<std::string> vp({"id"});
        std::vector<int64_t> vids;
        int count = 10000;
        auto txn = db.CreateWriteTxn();
        for (int32_t i = 0; i < count; i++) {
            auto vid1 = txn.AddVertex(std::string("Person"), vp, {FieldData::Int32(i)});
            vids.emplace_back(vid1);
        }
        std::vector<std::pair<EdgeUid, int32_t>> check;
        std::vector<std::string> ep({"id", "str", "int64"});
        for (int i = 0; i < vids.size() - 1; i++) {
            auto euid = txn.AddEdge(
                vids[i], vids[i + 1], std::string("Relation"), ep,
                {FieldData::Int32(i), FieldData::String(std::to_string(i)), FieldData::Int64(i)});
            check.emplace_back(euid, i);
        }
        txn.Commit();
        size_t modified = 0;
        UT_EXPECT_TRUE(db.AlterEdgeLabelDelFields("Relation", {"int64", "str"}, &modified));
        UT_EXPECT_EQ(modified, check.size());
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto eiter = txn.GetOutEdgeIterator(item.first);
            UT_EXPECT_EQ(eiter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_TRUE(eiter.GetField("str").IsNull());
            UT_EXPECT_TRUE(eiter.GetField("int64").IsNull());
        }
        txn.Abort();
    }
    // check re-open
    {
        Galaxy galaxy_tmp(path);
    }
}

TEST_F(TestAlterDetachedLabel, edge_mod_field) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    {
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        VertexOptions vo;
        vo.primary_field = "id";
        vo.detach_property = true;
        UT_EXPECT_TRUE(db.AddVertexLabel(
            "Person", std::vector<FieldSpec>({{"id", FieldType::INT32, false}}), vo));

        EdgeOptions eo;
        eo.detach_property = true;
        UT_EXPECT_TRUE(db.AddEdgeLabel("Relation",
                                       std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                               {"str", FieldType::STRING, true},
                                                               {"int64", FieldType::INT64, true}}),
                                       eo));
        UT_EXPECT_TRUE(db.AddEdgeIndex("Relation", "str", lgraph_api::IndexType::NonuniqueIndex));
        UT_EXPECT_TRUE(db.AddEdgeIndex("Relation", "int64", lgraph_api::IndexType::NonuniqueIndex));
        std::vector<std::string> vp({"id"});
        std::vector<int64_t> vids;
        int count = 10000;
        auto txn = db.CreateWriteTxn();
        for (int32_t i = 0; i < count; i++) {
            auto vid1 = txn.AddVertex(std::string("Person"), vp, {FieldData::Int32(i)});
            vids.emplace_back(vid1);
        }
        std::vector<std::pair<EdgeUid, int32_t>> check;
        std::vector<std::string> ep({"id", "str", "int64"});
        for (int i = 0; i < vids.size() - 1; i++) {
            auto euid = txn.AddEdge(
                vids[i], vids[i + 1], std::string("Relation"), ep,
                {FieldData::Int32(i), FieldData::String(std::to_string(i)), FieldData::Int64(i)});
            check.emplace_back(euid, i);
        }
        txn.Commit();
        size_t modified = 0;
        std::vector<FieldSpec> mod{FieldSpec("int64", FieldType::INT32, true)};
        UT_EXPECT_TRUE(db.AlterEdgeLabelModFields("Relation", mod, &modified));
        UT_EXPECT_EQ(modified, check.size());
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto eiter = txn.GetOutEdgeIterator(item.first);
            UT_EXPECT_EQ(eiter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_EQ(eiter.GetField("str").AsString(), std::to_string(item.second));
            UT_EXPECT_EQ(eiter.GetField("int64").AsInt32(), item.second);
        }
        UT_EXPECT_FALSE(txn.IsEdgeIndexed("Relation", "int64"));
        txn.Abort();
    }
    // check re-open
    {
        Galaxy galaxy_tmp(path);
    }
}

TEST_F(TestAlterDetachedLabel, vertex_add_field) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    {
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        VertexOptions vo;
        vo.primary_field = "id";
        vo.detach_property = true;
        UT_EXPECT_TRUE(
            db.AddVertexLabel("Person",
                              std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                      {"str", FieldType::STRING, true},
                                                      {"int64", FieldType::INT64, true}}),
                              vo));

        std::vector<std::string> vp({"id", "str", "int64"});
        std::vector<std::pair<int64_t, int32_t>> check;
        int count = 10000;
        auto txn = db.CreateWriteTxn();
        for (int32_t i = 0; i < count; i++) {
            auto vid1 = txn.AddVertex(
                std::string("Person"), vp,
                {FieldData::Int32(i), FieldData::String(std::to_string(i)), FieldData::Int64(i)});
            check.emplace_back(vid1, i);
        }
        txn.Commit();
        FieldSpec fs("addr", FieldType::STRING, true);
        size_t modified = 0;
        UT_EXPECT_TRUE(db.AlterVertexLabelAddFields("Person", {fs}, {FieldData()}, &modified));
        UT_EXPECT_EQ(modified, count);
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto viter = txn.GetVertexIterator(item.first);
            UT_EXPECT_EQ(viter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_EQ(viter.GetField("str").AsString(), std::to_string(item.second));
            UT_EXPECT_EQ(viter.GetField("int64").AsInt64(), item.second);
            UT_EXPECT_TRUE(viter.GetField("addr").IsNull());
        }
        txn.Abort();
        txn = db.CreateWriteTxn();
        for (auto& item : check) {
            auto viter = txn.GetVertexIterator(item.first);
            viter.SetField("addr", FieldData::String(std::to_string(item.second)));
        }
        txn.Commit();
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto viter = txn.GetVertexIterator(item.first);
            UT_EXPECT_EQ(viter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_EQ(viter.GetField("str").AsString(), std::to_string(item.second));
            UT_EXPECT_EQ(viter.GetField("int64").AsInt64(), item.second);
            UT_EXPECT_EQ(viter.GetField("addr").AsString(), std::to_string(item.second));
        }
        txn.Abort();
    }
    // check re-open
    {
        Galaxy galaxy_tmp(path);
    }
}

TEST_F(TestAlterDetachedLabel, vertex_delete_field) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    {
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        VertexOptions vo;
        vo.primary_field = "id";
        vo.detach_property = true;
        UT_EXPECT_TRUE(
            db.AddVertexLabel("Person",
                              std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                      {"str", FieldType::STRING, true},
                                                      {"int64", FieldType::INT64, true}}),
                              vo));
        std::vector<std::string> vp({"id", "str", "int64"});
        std::vector<std::pair<int64_t, int32_t>> check;
        int count = 10000;
        auto txn = db.CreateWriteTxn();
        for (int32_t i = 0; i < count; i++) {
            auto vid1 = txn.AddVertex(
                std::string("Person"), vp,
                {FieldData::Int32(i), FieldData::String(std::to_string(i)), FieldData::Int64(i)});
            check.emplace_back(vid1, i);
        }
        txn.Commit();
        size_t modified = 0;
        UT_EXPECT_TRUE(db.AlterVertexLabelDelFields("Person", {"int64", "str"}, &modified));
        UT_EXPECT_EQ(modified, count);
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto viter = txn.GetVertexIterator(item.first);
            UT_EXPECT_EQ(viter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_TRUE(viter.GetField("str").IsNull());
            UT_EXPECT_TRUE(viter.GetField("int64").IsNull());
        }
        txn.Abort();
    }
    // check re-open
    {
        Galaxy galaxy_tmp(path);
    }
}

TEST_F(TestAlterDetachedLabel, vertex_mod_field) {
    std::string path = "./testdb";
    auto ADMIN = lgraph::_detail::DEFAULT_ADMIN_NAME;
    auto ADMIN_PASS = lgraph::_detail::DEFAULT_ADMIN_PASS;
    lgraph::AutoCleanDir cleaner(path);
    {
        Galaxy galaxy(path);
        galaxy.SetCurrentUser(ADMIN, ADMIN_PASS);
        GraphDB db = galaxy.OpenGraph("default");
        VertexOptions vo;
        vo.primary_field = "id";
        vo.detach_property = true;
        UT_EXPECT_TRUE(
            db.AddVertexLabel("Person",
                              std::vector<FieldSpec>({{"id", FieldType::INT32, false},
                                                      {"str", FieldType::STRING, true},
                                                      {"int64", FieldType::INT64, true}}),
                              vo));
        std::vector<std::string> vp({"id", "str", "int64"});
        std::vector<std::pair<int64_t, int32_t>> check;
        int count = 10000;
        auto txn = db.CreateWriteTxn();
        for (int32_t i = 0; i < count; i++) {
            auto vid1 = txn.AddVertex(
                std::string("Person"), vp,
                {FieldData::Int32(i), FieldData::String(std::to_string(i)), FieldData::Int64(i)});
            check.emplace_back(vid1, i);
        }
        txn.Commit();
        size_t modified = 0;
        std::vector<FieldSpec> mod{FieldSpec("int64", FieldType::INT32, true)};
        UT_EXPECT_TRUE(db.AlterVertexLabelModFields("Person", mod, &modified));
        UT_EXPECT_EQ(modified, count);
        txn = db.CreateReadTxn();
        for (auto& item : check) {
            auto viter = txn.GetVertexIterator(item.first);
            UT_EXPECT_EQ(viter.GetField("id").AsInt32(), item.second);
            UT_EXPECT_EQ(viter.GetField("str").AsString(), std::to_string(item.second));
            UT_EXPECT_EQ(viter.GetField("int64").AsInt32(), item.second);
        }
        txn.Abort();
    }
    // check re-open
    {
        Galaxy galaxy_tmp(path);
    }
}
