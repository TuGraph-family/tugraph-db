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
#include "graphdb/graph_db.h"
#include "common/logger.h"
#include "test_util.h"
#include <random>
#include "common/value.h"
#include "transaction/transaction.h"
namespace fs = std::filesystem;
using namespace graphdb;
static std::string testdb = "testdb";
std::unordered_set<std::string> labels = {"label1","label2"};
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

static void CreateVertex(GraphDB* db) {
    auto txn = db->BeginTransaction();
    for (auto i = 0; i < 1000; i++) {
        txn->CreateVertex(labels,properties);
    }
    txn->Commit();
}

TEST(Concurrency, vertex) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    std::vector<std::thread> threads;
    for (size_t i = 0; i < 10; i++) {
        threads.emplace_back(CreateVertex, graphDB.get());
    }
    for (auto& t : threads) {
        t.join();
    }
    auto txn = graphDB->BeginTransaction();
    int count = 0;
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        auto& v = viter->GetVertex();
        count++;
        EXPECT_EQ(v.GetAllProperty(), properties);
        EXPECT_EQ(v.GetLabels(), labels);
    }
    EXPECT_EQ(count, 10000);
    txn->Commit();
}

static void CreateEdge(GraphDB* db, int index) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(index*10, index*10 + 9);
    auto txn = db->BeginTransaction();
    for (auto i = 0; i < 1000; i++) {
        int start = dis(gen);
        int end = dis(gen);
        auto v1iter = txn->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id",Value::Integer(start)}});
        EXPECT_TRUE(v1iter->Valid());
        auto v2iter = txn->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id",Value::Integer(end)}});
        EXPECT_TRUE(v2iter->Valid());
        txn->CreateEdge(v1iter->GetVertex(), v2iter->GetVertex(), "edge1", properties);
    }
    txn->Commit();
}

TEST(Concurrency, edge) {
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
    std::vector<std::thread> threads;
    for (size_t i = 0; i < 10; i++) {
        threads.emplace_back(CreateEdge, graphDB.get(), i);
    }
    for (auto& t : threads) {
        t.join();
    }
    txn = graphDB->BeginTransaction();
    int count = 0;
    for (auto viter = txn->NewVertexIterator(); viter->Valid(); viter->Next()) {
        for (auto eiter = viter->GetVertex().NewEdgeIterator(EdgeDirection::OUTGOING, {}, {});
            eiter->Valid(); eiter->Next()) {
            EXPECT_EQ(eiter->GetEdge().GetAllProperty(), properties);
            count++;
        }
    }
    EXPECT_EQ(count, 10000);
    txn->Commit();
}

TEST(Concurrency, vertexConflict) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexPropertyIndex("label1_id", true, "label1", "id");
    auto txn1 = graphDB->BeginTransaction();
    txn1->CreateVertex(
            {"label1"},
            {{"id", Value::Integer(1)}, {"str", Value::String("1")}});
    auto txn2 = graphDB->BeginTransaction();
    EXPECT_THROW_CODE_MSG(txn2->CreateVertex(
            {"label1"},
            {{"id", Value::Integer(1)}, {"str", Value::String("2")}}),
                          StorageEngineError, "Timeout waiting to lock key");
    txn2->Rollback();
    txn1->Commit();
    auto txn3 = graphDB->BeginTransaction();
    int count = 0;
    for (auto viter = txn3->NewVertexIterator(); viter->Valid(); viter->Next()) {
        EXPECT_EQ(viter->GetVertex().GetProperty("str"), Value::String("1"));
        count++;
    }
    EXPECT_EQ(count, 1);
}

TEST(Concurrency, edgeConflict) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    for (auto i = 0; i < 3; i++) {
        txn->CreateVertex(
                v1_labels,
                {{"id", Value::Integer(i)},
                 {"str", Value::String(std::to_string(i))}});
    }
    txn->Commit();
    auto txn1 = graphDB->BeginTransaction();
    {
        auto v1iter = txn1->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(0)}});
        EXPECT_TRUE(v1iter->Valid());
        auto v2iter = txn1->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(1)}});
        EXPECT_TRUE(v2iter->Valid());
        txn1->CreateEdge(v1iter->GetVertex(), v2iter->GetVertex(), "edge", {});
    }
    auto txn2 = graphDB->BeginTransaction();
    {
        auto v1iter = txn1->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(1)}});
        EXPECT_TRUE(v1iter->Valid());
        auto v2iter = txn1->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(2)}});
        EXPECT_TRUE(v2iter->Valid());
        EXPECT_THROW_CODE_MSG(txn2->CreateEdge(v1iter->GetVertex(), v2iter->GetVertex(), "edge", {}),
                              StorageEngineError, "Timeout waiting to lock key");
    }
    txn2->Rollback();
    txn1->Commit();
    auto txn3 = graphDB->BeginTransaction();
    {
        int count = 0;
        for (auto viter = txn1->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(0)}});
            viter->Valid(); viter->Next()) {
            for (auto eiter = viter->GetVertex().NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
                count++;
            }
        }
        EXPECT_EQ(count, 1);
        count = 0;
        for (auto viter = txn1->NewVertexIterator("label1", std::unordered_map<std::string, Value>{{"id", Value::Integer(1)}});
             viter->Valid(); viter->Next()) {
            for (auto eiter = viter->GetVertex().NewEdgeIterator(EdgeDirection::OUTGOING, {}, {}); eiter->Valid(); eiter->Next()) {
                count++;
            }
        }
        EXPECT_EQ(count, 0);
    }
    txn3->Commit();
}
