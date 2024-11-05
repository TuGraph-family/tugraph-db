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
using namespace graphdb;
namespace fs = std::filesystem;
static std::string testdb = "testdb";
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

TEST(Transaction, commitAndRollback) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    std::unordered_set<std::string> v1_labels = {"label1","label2"};
    std::unordered_set<std::string> v2_labels = {"label3","label4"};
    std::unordered_set<std::string> v3_labels = {"label5","label6"};
    std::unordered_set<std::string> v4_labels = {"label7","label8"};
    auto v1 = txn->CreateVertex(v1_labels,properties);
    auto v2 = txn->CreateVertex(v2_labels,properties);
    auto v3 = txn->CreateVertex(v3_labels,properties);
    auto v4 = txn->CreateVertex(v4_labels,properties);
    auto e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    auto e2 = txn->CreateEdge(v2, v3, "edge_type23", properties);
    auto e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    auto e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    txn->Rollback();
    txn = graphDB->BeginTransaction();
    EXPECT_THROW_CODE(txn->GetVertexById(v1.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetVertexById(v2.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetVertexById(v3.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetVertexById(v4.GetId()), VertexIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e1.GetTypeId(), e1.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e2.GetTypeId(), e2.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e3.GetTypeId(), e3.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e4.GetTypeId(), e4.GetId()), EdgeIdNotFound);
    txn->Commit();
    txn = graphDB->BeginTransaction();
    v1 = txn->CreateVertex(v1_labels,properties);
    v2 = txn->CreateVertex(v2_labels,properties);
    v3 = txn->CreateVertex(v3_labels,properties);
    v4 = txn->CreateVertex(v4_labels,properties);
    e1 = txn->CreateEdge(v1, v2, "edge_type12", properties);
    e2 = txn->CreateEdge(v2, v3, "edge_type23", properties);
    txn->Commit();
    txn = graphDB->BeginTransaction();
    EXPECT_NO_THROW(txn->GetVertexById(v1.GetId()));
    EXPECT_NO_THROW(txn->GetVertexById(v2.GetId()));
    EXPECT_NO_THROW(txn->GetVertexById(v3.GetId()));
    EXPECT_NO_THROW(txn->GetVertexById(v4.GetId()));
    EXPECT_NO_THROW(txn->GetEdgeById(e1.GetTypeId(), e1.GetId()));
    EXPECT_NO_THROW(txn->GetEdgeById(e2.GetTypeId(), e2.GetId()));
    EXPECT_THROW_CODE(txn->GetEdgeById(e3.GetTypeId(), e3.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e4.GetTypeId(), e4.GetId()), EdgeIdNotFound);
    txn->Commit();
    txn = graphDB->BeginTransaction();
    e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    txn->Rollback();
    txn = graphDB->BeginTransaction();
    EXPECT_THROW_CODE(txn->GetEdgeById(e3.GetTypeId(), e3.GetId()), EdgeIdNotFound);
    EXPECT_THROW_CODE(txn->GetEdgeById(e4.GetTypeId(), e4.GetId()), EdgeIdNotFound);
    txn->Commit();
    txn = graphDB->BeginTransaction();
    e3 = txn->CreateEdge(v3, v4, "edge_type34", properties);
    e4 = txn->CreateEdge(v4, v1, "edge_type41", properties);
    txn->Commit();
    txn = graphDB->BeginTransaction();
    EXPECT_NO_THROW(txn->GetEdgeById(e3.GetTypeId(), e3.GetId()));
    EXPECT_NO_THROW(txn->GetEdgeById(e4.GetTypeId(), e4.GetId()));
    txn->Commit();
}
