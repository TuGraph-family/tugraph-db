/**
* Copyright 2024 AntGroup CO., Ltd.
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

#include <gtest/gtest.h>
#include <filesystem>
#include "test_util.h"
#include "graphdb/ftindex/include/lib.rs.h"
#include "common/logger.h"
#include "common/value.h"
#include "graphdb/graph_db.h"
#include "transaction/transaction.h"
using namespace graphdb;
namespace fs = std::filesystem;
static std::string testdb = "testdb";
static std::string test_ftindex = "test_ftindex";
TEST(FTIndex, basic_v1) {
    fs::remove_all(test_ftindex);
    ::rust::Vec<::rust::String> properties;
    properties.push_back("title");
    properties.push_back("body");
    auto ft = new_ftindex(test_ftindex, properties);
    ::rust::Vec<::rust::String> fields = {"title", "body"};
    {
        ::rust::Vec<::rust::String> values = {"title1 common_title title2", "body1 common_body body2"};
        ft_add_document(*ft, 1, fields, values);
    }
    {
        ::rust::Vec<::rust::String> values = {"title3 common_title title4", "body3 common_body, body4"};
        ft_add_document(*ft, 2, fields, values);
    }
    ft_commit(*ft, "payload");
    auto ret = ft_query(*ft, "common_title", {10});
    EXPECT_EQ(ret.size(), 2);
    ret = ft_query(*ft, "common_body", {10});
    EXPECT_EQ(ret.size(), 2);
    ret = ft_query(*ft, "common_title AND title1", {10});
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0].id, 1);
    ret = ft_query(*ft, "title3", {10});
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0].id, 2);
}

TEST(FTIndex, basic_v2) {
    fs::remove_all(test_ftindex);
    ::rust::Vec<::rust::String> properties;
    properties.push_back("title");
    properties.push_back("body");
    auto ft = new_ftindex(test_ftindex, properties);
    ::rust::Vec<::rust::String> fields = {"title", "body"};
    {
        ::rust::Vec<::rust::String> values = {"title1 common_title title11", "body1 common_body body11"};
        ft_add_document(*ft, 1, fields, values);
    }
    {
        ::rust::Vec<::rust::String> values = {"title3 common_title title33", "body3 common_body, body33"};
        ft_add_document(*ft, 2, fields, values);
    }
    ft_commit(*ft, "payload");
    auto ret = ft_query(*ft, "title1", {10});
    EXPECT_EQ(ret.size(), 1);
    ret = ft_query(*ft, "body1", {10});
    EXPECT_EQ(ret.size(), 1);
}

TEST(FTIndex, chinese) {
    fs::remove_all(test_ftindex);
    ::rust::Vec<::rust::String> properties;
    properties.push_back("title");
    properties.push_back("body");
    auto ft = new_ftindex(test_ftindex, properties);
    ::rust::Vec<::rust::String> fields = {"title", "body"};
    {
        ::rust::Vec<::rust::String> values = {"标题1 公共标题 标题2", "内容1 公共内容 内容2"};
        ft_add_document(*ft, 1, fields, values);
    }
    {
        ::rust::Vec<::rust::String> values = {"标题3 公共标题 标题4", "内容3 公共内容 内容4"};
        ft_add_document(*ft, 2, fields, values);
    }
    ft_commit(*ft, "payload");
    auto ret = ft_query(*ft, "标题1", {10});
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0].id, 1);
    ret = ft_query(*ft, "公共标题", {10});
    EXPECT_EQ(ret.size(), 2);
}

TEST(FTIndex, update) {
    fs::remove_all(test_ftindex);
    ::rust::Vec<::rust::String> properties;
    properties.push_back("title");
    properties.push_back("body");
    auto ft = new_ftindex(test_ftindex, properties);
    ::rust::Vec<::rust::String> fields = {"title", "body"};
    {
        ::rust::Vec<::rust::String> values = {"title1 common_title title2", "body1 common_body body2"};
        ft_add_document(*ft, 1, fields, values);
    }
    {
        ::rust::Vec<::rust::String> values = {"title3 common_title title4", "body3 common_body, body4"};
        ft_add_document(*ft, 2, fields, values);
    }
    ft_commit(*ft, "payload");
    ft_delete_document(*ft, 1);
    ft_commit(*ft, "payload");
    auto ret = ft_query(*ft, "common_title", {10});
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0].id, 2);
    {
        ::rust::Vec<::rust::String> values = {"title11 common_title title22", "body11 common_body body22"};
        ft_add_document(*ft, 1, fields, values);
    }
    ft_commit(*ft, "payload");
    ret = ft_query(*ft, "title11", {10});
    EXPECT_EQ(ret.size(), 1);
    EXPECT_EQ(ret[0].id, 1);
}

static std::unordered_map<std::string, Value> properties = {
        {"property1", Value::Bool(true)},
        {"property2", Value::Integer(100)},
        {"property3", Value::String("string")},
        {"property4", Value::Double(1.1314)},
        {"property5", Value::BoolArray({true, false})},
        {"property6", Value::IntegerArray({1, 2, 3})},
        {"property7", Value::StringArray({"string1", "string2"})},
        {"property8", Value::DoubleArray({11.11, 22.22})}
};

TEST(FTIndex, indexVertex) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexFullTextIndex("ft_index", {"label1"}, {"str"});
    auto txn = graphDB->BeginTransaction();
    txn->CreateVertex({"label1","label2"},
                      {{"id", Value::Integer(1)},
                       {"str", Value::String("string1 string string11")}});
    txn->CreateVertex({"label1","label2"},
                      {{"id", Value::Integer(2)},
                       {"str", Value::String("string2 string string22")}});
    txn->CreateVertex({"label3"},
                      {{"id", Value::Integer(3)},
                       {"str", Value::String("string3 string string33")}});
    txn->CreateVertex({"label1"},
                      {{"id", Value::Integer(4)},
                       {"str", Value::Integer(4)}});
    txn->Commit();
    for (auto& [name, index] : graphDB->meta_info().GetVertexFullTextIndex()) {
        index->ApplyWAL();
    }
    txn = graphDB->BeginTransaction();
    int count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string1", 10); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertexScore().vertex.GetProperty("id"), Value::Integer(1));
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string2", 10); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertexScore().vertex.GetProperty("id"), Value::Integer(2));
        count++;
    }
    EXPECT_EQ(count, 1);
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string", 10); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 2);
    txn->Commit();
}

TEST(FTIndex, deleteVertex) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexFullTextIndex("ft_index", {"label1"}, {"str"});
    auto txn = graphDB->BeginTransaction();
    txn->CreateVertex({"label1","label2"},
                      {{"id", Value::Integer(1)},
                       {"str", Value::String("string1 string string11")}});
    txn->CreateVertex({"label1","label2"},
                      {{"id", Value::Integer(2)},
                       {"str", Value::String("string2 string string22")}});
    txn->CreateVertex({"label3"},
                      {{"id", Value::Integer(3)},
                       {"str", Value::String("string3 string string33")}});
    txn->CreateVertex({"label1"},
                      {{"id", Value::Integer(4)},
                       {"str", Value::Integer(4)}});
    txn->Commit();
    for (auto& [name, index] : graphDB->meta_info().GetVertexFullTextIndex()) {
        index->ApplyWAL();
    }
    txn = graphDB->BeginTransaction();
    int count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string1", 10); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertexScore().vertex.GetProperty("id"), Value::Integer(1));
        count++;
    }
    EXPECT_EQ(count, 1);
    txn->Rollback();
    {
        txn = graphDB->BeginTransaction();
        auto viter = txn->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(1)}});
        EXPECT_TRUE(viter->Valid());
        viter->GetVertex().Delete();
        txn->Commit();
    }
    for (auto& [name, index] : graphDB->meta_info().GetVertexFullTextIndex()) {
        index->ApplyWAL();
    }
    txn = graphDB->BeginTransaction();
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string1", 10); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertexScore().vertex.GetProperty("id"), Value::Integer(1));
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string", 10); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertexScore().vertex.GetProperty("id"), Value::Integer(2));
        count++;
    }
    EXPECT_EQ(count, 1);
    txn->Commit();
    {
        txn = graphDB->BeginTransaction();
        auto viter = txn->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(2)}});
        EXPECT_TRUE(viter->Valid());
        viter->GetVertex().RemoveProperty("str");
        txn->Commit();
    }
    for (auto& [name, index] : graphDB->meta_info().GetVertexFullTextIndex()) {
        index->ApplyWAL();
    }
    txn = graphDB->BeginTransaction();
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string2", 10); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string", 10); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    txn->Commit();
}

TEST(FTIndex, updateVertex) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexFullTextIndex("ft_index", {"label1"}, {"str"});
    auto txn = graphDB->BeginTransaction();
    txn->CreateVertex({"label1","label2"},
                      {{"id", Value::Integer(1)},
                       {"str", Value::String("string1 string string11")}});
    txn->CreateVertex({"label1","label2"},
                      {{"id", Value::Integer(2)},
                       {"str", Value::String("string2 string string22")}});
    txn->CreateVertex({"label3"},
                      {{"id", Value::Integer(3)},
                       {"str", Value::String("string3 string string33")}});
    txn->CreateVertex({"label1"},
                      {{"id", Value::Integer(4)},
                       {"str", Value::Integer(4)}});
    txn->Commit();
    for (auto& [name, index] : graphDB->meta_info().GetVertexFullTextIndex()) {
        index->ApplyWAL();
    }
    txn = graphDB->BeginTransaction();
    int count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string1", 10); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertexScore().vertex.GetProperty("id"), Value::Integer(1));
        viter->GetVertexScore().vertex.SetProperties({{"str", Value::Integer(10)}});
        count++;
    }
    EXPECT_EQ(count, 1);
    txn->Commit();
    for (auto& [name, index] : graphDB->meta_info().GetVertexFullTextIndex()) {
        index->ApplyWAL();
    }
    txn = graphDB->BeginTransaction();
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string1", 10); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 0);
    {
        auto viter = txn->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(1)}});
        EXPECT_TRUE(viter->Valid());
        viter->GetVertex().SetProperties({{"str", Value::String("string1 string string11")}});
    }
    txn->Commit();
    for (auto& [name, index] : graphDB->meta_info().GetVertexFullTextIndex()) {
        index->ApplyWAL();
    }
    txn = graphDB->BeginTransaction();
    count = 0;
    for (auto viter = txn->QueryVertexByFTIndex("ft_index", "string1", 10); viter->Valid(); viter->Next()) {
        count++;
    }
    EXPECT_EQ(count, 1);
    txn->Commit();
}
