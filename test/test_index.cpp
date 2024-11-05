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
#include "common/value.h"
#include "graphdb/graph_db.h"
#include "transaction/transaction.h"
namespace fs = std::filesystem;
static std::string testdb = "testdb";
using namespace graphdb;
TEST(VertexUniqueIndex, basic) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    txn = graphDB->BeginTransaction();
    for (auto i = 0; i < 100; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
        EXPECT_EQ(viter->GetVertex().GetProperty("str"), Value::String(std::to_string(i)));
    }
    for (auto i = 100; i < 110; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_FALSE(viter->Valid());
    }
    for (auto i = 0; i < 100; i++) {
        auto viter = txn->NewVertexIterator("label2",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<ScanVertexBylabelProperties*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
    }
    for (auto i = 0; i < 100; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"str", Value::String(std::to_string(i))}});
        EXPECT_TRUE(dynamic_cast<ScanVertexBylabelProperties*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
    }
    txn->Commit();
}

TEST(VertexUniqueIndex, delete) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    txn = graphDB->BeginTransaction();
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        if (viter->GetVertex().GetProperty("id").AsInteger() < 10) {
            viter->GetVertex().Delete();
        }
    }
    for (auto i = 0; i < 10; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_FALSE(viter->Valid());
    }
    for (auto i = 10; i < 100; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
        EXPECT_EQ(viter->GetVertex().GetProperty("str"), Value::String(std::to_string(i)));
    }
    txn->Commit();
}

TEST(VertexUniqueIndex, update) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    txn = graphDB->BeginTransaction();
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        auto& v = viter->GetVertex();
        auto id = v.GetProperty("id").AsInteger();
        v.SetProperties({{"id", Value::Integer(id + 100)}, {"str", Value::String(std::to_string(id + 100))}});
    }
    for (auto i = 0; i < 100; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_FALSE(viter->Valid());
    }
    for (auto i = 100; i < 200; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
        EXPECT_EQ(viter->GetVertex().GetProperty("str"), Value::String(std::to_string(i)));
    }

    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        auto& v = viter->GetVertex();
        auto id = v.GetProperty("id").AsInteger();
        v.SetProperties({{"id", Value::String(std::to_string(id))}});
    }
    for (auto i = 100; i < 200; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_FALSE(viter->Valid());
    }
    for (auto i = 100; i < 200; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::String(std::to_string(i))}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
    }
    txn->Commit();
}

TEST(VertexUniqueIndex, conflict) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    for (auto i = 0; i < 100; i++) {
        txn = graphDB->BeginTransaction();
        EXPECT_THROW_CODE(txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}}), IndexValueAlreadyExist);
        txn->Rollback();
    }
    txn = graphDB->BeginTransaction();
    auto viter = txn->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(10)}});
    EXPECT_TRUE(viter->Valid());
    EXPECT_THROW_CODE(viter->GetVertex().SetProperties({{"id", Value::Integer(20)}}), IndexValueAlreadyExist);
    txn->Rollback();
    txn = graphDB->BeginTransaction();
    auto v = txn->CreateVertex({"label1"}, {});
    EXPECT_THROW_CODE(v.SetProperties({{"id", Value::Integer(20)}}), IndexValueAlreadyExist);
    txn->Rollback();
}

TEST(VertexUniqueIndex, reopen) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    txn.reset();
    graphDB.reset();
    graphDB = GraphDB::Open(testdb, {});
    txn = graphDB->BeginTransaction();
    for (auto i = 0; i < 100; i++) {
        auto viter = txn->NewVertexIterator("label1",
                                            std::unordered_map<std::string, Value>{{"id", Value::Integer(i)}});
        EXPECT_TRUE(dynamic_cast<GetVertexByUniqueIndex*>(viter.get()));
        EXPECT_TRUE(viter->Valid());
        EXPECT_EQ(viter->GetVertex().GetProperty("str"), Value::String(std::to_string(i)));
    }
    txn->Commit();
}

TEST(VertexUniqueIndex, buildConflict) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->CreateVertex(v1_labels, {{"id", Value::Integer(10)}});
    txn->Commit();
    EXPECT_THROW_CODE(graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id"), IndexValueAlreadyExist);
    txn.reset();
    graphDB.reset();
    graphDB = GraphDB::Open(testdb, {});
    graphDB.reset();
    fs::remove_all(testdb);
    graphDB = GraphDB::Open(testdb, {});
    txn = graphDB->BeginTransaction();
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    EXPECT_THROW_CODE(graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id"), VertexIndexAlreadyExist);
}

TEST(VertexUniqueIndex, buildNonExists) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 100; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    txn = graphDB->BeginTransaction();
    EXPECT_THROW_CODE(txn->CreateVertex(v1_labels, {{"id", Value::Integer(10)}}), IndexValueAlreadyExist);
    txn->Rollback();
}
