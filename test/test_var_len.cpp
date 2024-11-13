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
#include "common/logger.h"
#include "common/value.h"
#include "graphdb/graph_db.h"
#include "cypher/execution_plan/result_iterator.h"
using namespace graphdb;
namespace fs = std::filesystem;
static std::string testdb = "varlendb";

TEST(VarLenDB, db) {
    fs::remove_all(testdb);
    auto graphDB = GraphDB::Open(testdb, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    txn->Execute(&rtx, "CREATE (rachel:Person {name: 'Rachel Kempson', birthyear: 1910})-[:HAS_CHILD]->(vanessa:Person {name: 'Vanessa Redgrave', birthyear: 1937})")->Consume();
    txn->Commit();
    txn = graphDB->BeginTransaction();
    auto resultIterator = txn->Execute(&rtx, "MATCH p=(n:Person {name: 'Vanessa Redgrave'})<-[]-(m) RETURN p;");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        auto path = std::any_cast<common::Path>(records[0].data).data;
        EXPECT_TRUE(path[0].is_node);
        EXPECT_EQ(std::any_cast<common::Node>(path[0].data).properties["name"].AsString(), "Vanessa Redgrave");
        EXPECT_FALSE(path[1].is_node);
        EXPECT_EQ(std::any_cast<common::Relationship>(path[1].data).type, "HAS_CHILD");
        EXPECT_TRUE(path[2].is_node);
        EXPECT_EQ(std::any_cast<common::Node>(path[2].data).properties["name"].AsString(), "Rachel Kempson");
    }
    txn->Commit();
    txn = graphDB->BeginTransaction();
    resultIterator = txn->Execute(&rtx, "match p=(n)-[*..1]->(m) return p");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        auto path = std::any_cast<common::Path>(records[0].data).data;
        EXPECT_TRUE(path[0].is_node);
        EXPECT_EQ(std::any_cast<common::Node>(path[0].data).properties["name"].AsString(), "Rachel Kempson");
        EXPECT_FALSE(path[1].is_node);
        EXPECT_EQ(std::any_cast<common::Relationship>(path[1].data).type, "HAS_CHILD");
        EXPECT_TRUE(path[2].is_node);
        EXPECT_EQ(std::any_cast<common::Node>(path[2].data).properties["name"].AsString(), "Vanessa Redgrave");
    }
    txn->Commit();
    txn = graphDB->BeginTransaction();
    resultIterator = txn->Execute(&rtx, "return time({time:localtime()});");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetBoltRecord();
        LOG_INFO(records.size());
    }
    txn->Commit();
}
