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
#include <random>
#include "cypher/execution_plan/result_iterator.h"
namespace fs = std::filesystem;
using namespace graphdb;
static std::string testdb = "testdb";

TEST(ExecutionPlan, all_node_scan) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
        cypher::RTContext rtx;
        auto iter = txn->Execute(&rtx, "explain match(n) return n");
        EXPECT_TRUE(iter->Valid());
        EXPECT_EQ(iter->GetRecord().size(), 1);
        std::string plan = R"("Execution Plan:
Produce Results
    Project [n]
        Node Scan [n,ScanAllVertex]
")";
        EXPECT_EQ(iter->GetRecord()[0].ToString(), plan);
}
TEST(ExecutionPlan, unique_index) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    cypher::RTContext rtx;
    txn->Execute(
           &rtx,
           "CALL db.createUniquePropertyConstraint('person_name_unique', 'person', 'name')")
        ->Consume();
    auto iter = txn->Execute(&rtx, "explain match(n:person {name:'bob'}) return n");
    EXPECT_TRUE(iter->Valid());
    EXPECT_EQ(iter->GetRecord().size(), 1);
    std::string plan = R"("Execution Plan:
Produce Results
    Project [n]
        Node Scan [n,GetVertexByUniqueIndex]
")";
    txn->Execute(&rtx, "create (n:person {name:'bob', age:10})")->Consume();
    EXPECT_EQ(iter->GetRecord()[0].ToString(), plan);
    iter = txn->Execute(&rtx, "explain match(n:person {age:20}) return n");
    EXPECT_TRUE(iter->Valid());
    EXPECT_EQ(iter->GetRecord().size(), 1);
    plan = R"("Execution Plan:
Produce Results
    Project [n]
        Node Scan [n,ScanVertexBylabelProperties]
")";
    EXPECT_EQ(iter->GetRecord()[0].ToString(), plan);
}

TEST(ExecutionPlan, scan_by_label) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    cypher::RTContext rtx;
    txn->Execute(&rtx, "create (n:person {name:'bob'}) create (n:person {name:'alice'})")->Consume();
    auto iter = txn->Execute(&rtx, "explain match(n:person) return n");
    EXPECT_TRUE(iter->Valid());
    EXPECT_EQ(iter->GetRecord().size(), 1);
    std::string plan = R"("Execution Plan:
Produce Results
    Project [n]
        Node Scan [n,ScanVertexBylabel]
")";
    EXPECT_EQ(iter->GetRecord()[0].ToString(), plan);
    iter = txn->Execute(&rtx, "explain match(n:person {name:'bob'}) return n");
    EXPECT_TRUE(iter->Valid());
    EXPECT_EQ(iter->GetRecord().size(), 1);
    plan = R"("Execution Plan:
Produce Results
    Project [n]
        Node Scan [n,ScanVertexBylabelProperties]
")";
    EXPECT_EQ(iter->GetRecord()[0].ToString(), plan);
}

TEST(ExecutionPlan, no_vertex) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    auto txn = graphDB->BeginTransaction();
    cypher::RTContext rtx;
    auto iter = txn->Execute(&rtx, "explain match(n:person) return n");
    EXPECT_TRUE(iter->Valid());
    EXPECT_EQ(iter->GetRecord().size(), 1);
    std::string plan = R"("Execution Plan:
Produce Results
    Project [n]
        Node Scan [n,NoVertexFound]
")";
    EXPECT_EQ(iter->GetRecord()[0].ToString(), plan);
}