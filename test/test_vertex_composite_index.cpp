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
#include "fma-common/configuration.h"
#include "fma-common/utils.h"
#include "core/kv_store.h"
#include "core/mock_kv.h"
#include "core/vertex_index.h"
#include "core/lightning_graph.h"
#include "./ut_utils.h"
#include "lgraph/lgraph_vertex_composite_index_iterator.h"
using namespace lgraph_api;
using namespace fma_common;

class TestVertexCompositeIndex : public TuGraphTest {};

int TestUniqueVertexCompositeIndexImpl() {
    Galaxy galaxy("./testdb", lgraph::_detail::DEFAULT_ADMIN_NAME,
                  lgraph::_detail::DEFAULT_ADMIN_PASS, false, true);
    UT_ASSERT(galaxy.CreateGraph("test", "test", 1<<20));
    auto graph = galaxy.OpenGraph("test");
    std::vector<FieldSpec> v_fds = {{"id", FieldType::INT64, false},
                                    {"score", FieldType::INT64, false},
                                    {"name", FieldType::STRING, false},
                                    {"class", FieldType::STRING, true}};
    UT_EXPECT_TRUE(graph.AddVertexLabel("person", v_fds, VertexOptions("id")));
    auto txn = graph.CreateWriteTxn();
    for (int i = 0; i < 10; ++i) {
        txn.AddVertex("person", {"id", "score", "name"},
                      {FieldData::Int64(i), FieldData::Int64(i),
                       FieldData::String(std::to_string(i))});
    }
    txn.Commit();
    graph.AddVertexCompositeIndex("person", {"score", "name"}, CompositeIndexType::UniqueIndex);
    txn = graph.CreateWriteTxn();
    for (int i = 10; i < 20; ++i) {
        txn.AddVertex("person", {"id", "score", "name"},
                      {FieldData::Int64(i), FieldData::Int64(i),
                       FieldData::String(std::to_string(i))});
    }
    txn.Commit();
    txn = graph.CreateReadTxn();
    auto composite_indexes = txn.ListVertexCompositeIndexes();
    UT_EXPECT_TRUE(composite_indexes.size() == 1);
    UT_EXPECT_TRUE(composite_indexes[0].fields[0] == "score");
    UT_EXPECT_TRUE(composite_indexes[0].fields[1] == "name");
    auto it = txn.GetVertexByUniqueCompositeIndex("person",
                           {"score", "name"}, {FieldData::Int64(10), FieldData::String("10")});
    UT_EXPECT_TRUE(it.GetField("id").AsInt64() == 10);
    int count = 0;
    for (auto it1 = txn.GetVertexCompositeIndexIterator("person",
              std::vector<std::string>{"score", "name"}, {FieldData::Int64(10),
              FieldData::String("10")}, {FieldData::Int64(20), FieldData::String("20")});
         it1.IsValid(); it1.Next()) {
        count++;
    }
    UT_EXPECT_TRUE(count == 10);
    txn.Abort();
    graph.AddVertexCompositeIndex("person", {"name", "class"}, CompositeIndexType::UniqueIndex);
    txn = graph.CreateWriteTxn();
    for (int i = 20; i < 30; ++i) {
        txn.AddVertex("person", {"id", "score", "name", "class"},
                      {FieldData::Int64(i), FieldData::Int64(i),
                       FieldData::String(std::to_string(i)),
                       FieldData::String(std::to_string(i))});
    }
    txn.Commit();
    count = 0;
    txn = graph.CreateReadTxn();
    composite_indexes = txn.ListVertexCompositeIndexes();
    UT_EXPECT_TRUE(composite_indexes.size() == 2);
    for (auto it1 = txn.GetVertexCompositeIndexIterator("person",
             std::vector<std::string>{"name", "class"}, {FieldData::String("20"),
             FieldData::String("20")}, {FieldData::String("25"), FieldData::String("25")});
         it1.IsValid(); it1.Next()) {
        count++;
    }
    UT_EXPECT_TRUE(count == 6);
    txn.Abort();
    UT_EXPECT_TRUE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.DeleteVertexCompositeIndex("person", {"score", "name"});
    UT_EXPECT_FALSE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AddVertexCompositeIndex("person", {"score", "name"}, CompositeIndexType::UniqueIndex);
    UT_EXPECT_TRUE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AlterVertexLabelDelFields("person", {"score"});
    UT_EXPECT_FALSE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AddVertexCompositeIndex("person", {"score", "name"}, CompositeIndexType::UniqueIndex);
    UT_EXPECT_TRUE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AlterVertexLabelModFields("person", {FieldSpec("score", FieldType::INT64, false)});
    UT_EXPECT_FALSE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    fma_common::file_system::RemoveDir("./testdb");
    return 0;
}

int TestNonUniqueVertexCompositeIndexImpl() {
    Galaxy galaxy("./testdb", lgraph::_detail::DEFAULT_ADMIN_NAME,
                  lgraph::_detail::DEFAULT_ADMIN_PASS, false, true);
    UT_ASSERT(galaxy.CreateGraph("test", "test", 1<<20));
    auto graph = galaxy.OpenGraph("test");
    std::vector<FieldSpec> v_fds = {{"id", FieldType::INT64, false},
                                    {"score", FieldType::INT64, false},
                                    {"name", FieldType::STRING, false},
                                    {"class", FieldType::STRING, true}};
    UT_EXPECT_TRUE(graph.AddVertexLabel("person", v_fds, VertexOptions("id")));
    auto txn = graph.CreateWriteTxn();
    for (int i = 0; i < 10; ++i) {
        txn.AddVertex("person", {"id", "score", "name"},
                      {FieldData::Int64(i), FieldData::Int64(1),
                       FieldData::String("1")});
    }
    txn.Commit();
    graph.AddVertexCompositeIndex("person", {"score", "name"}, CompositeIndexType::NonUniqueIndex);
    txn = graph.CreateWriteTxn();
    for (int i = 10; i < 20; ++i) {
        txn.AddVertex("person", {"id", "score", "name"},
                      {FieldData::Int64(i), FieldData::Int64(2),
                       FieldData::String("2")});
    }
    txn.Commit();
    txn = graph.CreateReadTxn();
    auto composite_indexes = txn.ListVertexCompositeIndexes();
    UT_EXPECT_TRUE(composite_indexes.size() == 1);
    UT_EXPECT_TRUE(composite_indexes[0].fields[0] == "score");
    UT_EXPECT_TRUE(composite_indexes[0].fields[1] == "name");
    int count = 0;
    for (auto it = txn.GetVertexCompositeIndexIterator("person",
         {"score", "name"}, {FieldData::Int64(1), FieldData::String("1")},
         {FieldData::Int64(1), FieldData::String("1")}); it.IsValid(); it.Next()) {
        auto tmp_it = txn.GetVertexIterator();
        tmp_it.Goto(it.GetVid());
        auto id = tmp_it.GetField("id").AsInt64();
        UT_EXPECT_TRUE(id == count);
        count++;
    }
    UT_EXPECT_TRUE(count == 10);
    txn.Abort();
    graph.AddVertexCompositeIndex("person", {"name", "class"}, CompositeIndexType::NonUniqueIndex);
    txn = graph.CreateWriteTxn();
    for (int i = 20; i < 30; ++i) {
        txn.AddVertex("person", {"id", "score", "name", "class"},
                      {FieldData::Int64(i), FieldData::Int64(3),
                       FieldData::String("3"),
                       FieldData::String("3")});
    }
    txn.Commit();
    txn = graph.CreateReadTxn();
    composite_indexes = txn.ListVertexCompositeIndexes();
    UT_EXPECT_TRUE(composite_indexes.size() == 2);
    count = 20;
    for (auto it = txn.GetVertexCompositeIndexIterator("person",
             {"name", "class"}, {FieldData::String("3"), FieldData::String("3")},
             {FieldData::String("3"), FieldData::String("3")}); it.IsValid(); it.Next()) {
        auto tmp_it = txn.GetVertexIterator();
        tmp_it.Goto(it.GetVid());
        auto id = tmp_it.GetField("id").AsInt64();
        UT_EXPECT_TRUE(id == count);
        count++;
    }
    txn.Abort();
    UT_EXPECT_TRUE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.DeleteVertexCompositeIndex("person", {"score", "name"});
    UT_EXPECT_FALSE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AddVertexCompositeIndex("person", {"score", "name"}, CompositeIndexType::NonUniqueIndex);
    UT_EXPECT_TRUE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AlterVertexLabelDelFields("person", {"score"});
    UT_EXPECT_FALSE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AddVertexCompositeIndex("person", {"score", "name"}, CompositeIndexType::NonUniqueIndex);
    UT_EXPECT_TRUE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    graph.AlterVertexLabelModFields("person", {FieldSpec("score", FieldType::INT64, false)});
    UT_EXPECT_FALSE(graph.IsVertexCompositeIndexed("person", {"score", "name"}));
    fma_common::file_system::RemoveDir("./testdb");
    return 0;
}

TEST_F(TestVertexCompositeIndex, compositeIndex) {
    TestUniqueVertexCompositeIndexImpl();
    TestNonUniqueVertexCompositeIndexImpl();
}
