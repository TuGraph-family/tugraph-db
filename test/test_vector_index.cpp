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

#include <chrono>
#include <filesystem>
#include <random>
#include <thread>

#include "common/flags.h"
#include "common/logger.h"
#include "common/value.h"
#include "cypher/execution_plan/result_iterator.h"
#include "graphdb/graph_db.h"
#include "test_util.h"
#include "transaction/transaction.h"
using namespace graphdb;
namespace fs = std::filesystem;
static std::string testdb = "testdb";

namespace {

bool WaitUntilBusy(
    GraphDB* graph_db, const std::unordered_set<uint32_t>& lids,
    const std::unordered_set<uint32_t>& pids,
    std::chrono::milliseconds timeout = std::chrono::seconds(5)) {
  auto deadline = std::chrono::steady_clock::now() + timeout;
  while (std::chrono::steady_clock::now() < deadline) {
    if (graph_db->busy_index().Busy(lids, pids)) {
      return true;
    }
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
  }
  return false;
}

}  // namespace

TEST(VectorIndex, build) {
  fs::remove_all(testdb);
  auto graphDB = GraphDB::Open(testdb, {});
  std::string index_name = "vector_index";
  auto txn = graphDB->BeginTransaction();
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(1)},
                     {"embedding", Value::DoubleArray({1.0, 1.0, 1.0, 1.0})}});
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(2)},
                     {"embedding", Value::DoubleArray({2.0, 2.0, 2.0, 2.0})}});
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(3)},
                     {"embedding", Value::DoubleArray({3.0, 3.0, 3.0, 3.0})}});
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(4)},
                     {"embedding", Value::DoubleArray({4.0, 4.0, 4.0, 4.0})}});
  txn->Commit();
  graphDB->AddVertexVectorIndex(index_name, "label1", "embedding", 4, "l2", 16,
                                100);
  txn = graphDB->BeginTransaction();
  int count = 0;
  for (auto viter = txn->QueryVertexByKnnSearch(index_name,
                                                {1.0, 2.0, 3.0, 4.0}, 10, 100);
       viter->Valid(); viter->Next()) {
    count++;
  }
  EXPECT_EQ(count, 4);
  txn->Commit();
}

class VectorIndexParamTest : public ::testing::TestWithParam<int> {};

TEST_P(VectorIndexParamTest, dim) {
  fs::remove_all(testdb);
  auto graphDB = GraphDB::Open(testdb, {});
  std::string index_name = "vector_index";
  int dim = GetParam();
  graphDB->AddVertexVectorIndex(index_name, "person", "embedding", dim, "l2",
                                16, 100);
  auto txn = graphDB->BeginTransaction();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0, 1.0);
  for (auto i = 0; i < 1000; i++) {
    std::vector<Value> embedding;
    embedding.reserve(dim);
    for (auto j = 0; j < dim; j++) {
      embedding.push_back(Value::Float(dis(gen)));
    }
    txn->CreateVertex({"person"}, {{"id", Value::Integer(1)},
                                   {"embedding", Value::Array(embedding)}});
  }
  txn->Commit();
  std::vector<float> query;
  query.reserve(dim);
  for (auto j = 0; j < dim; j++) {
    query.emplace_back(dis(gen));
  }
  txn = graphDB->BeginTransaction();
  for (auto viter = txn->QueryVertexByKnnSearch(index_name, query, 20, 100);
       viter->Valid(); viter->Next()) {
  }
  txn->Commit();
}

INSTANTIATE_TEST_SUITE_P(VectorIndex, VectorIndexParamTest,
                         testing::Values(128, 512, 1024, 2048, 4096));

TEST(VectorIndex, DISABLED_read_benchmark) {
  fs::remove_all(testdb);
  auto graphDB = GraphDB::Open(testdb, {});
  std::string index_name = "vector_index";
  int vector_count = 100000;
  int dim = 1024;
  graphDB->AddVertexVectorIndex(index_name, "person", "embedding", dim, "l2",
                                16, 100);
  auto txn = graphDB->BeginTransaction();
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_real_distribution<float> dis(0.0, 1.0);
  for (auto i = 0; i < vector_count; i++) {
    std::vector<Value> embedding;
    embedding.reserve(dim);
    for (auto j = 0; j < dim; j++) {
      embedding.push_back(Value::Float(dis(gen)));
    }
    txn->CreateVertex({"person"}, {{"id", Value::Integer(1)},
                                   {"embedding", Value::Array(embedding)}});
  }
  txn->Commit();
  txn.reset();
  for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
    index->ApplyWAL();
  }

  auto start = std::chrono::high_resolution_clock::now();
  std::vector<float> query;
  query.reserve(dim);
  for (auto j = 0; j < dim; j++) {
    query.emplace_back(dis(gen));
  }
  txn = graphDB->BeginTransaction();
  int count = 0;
  for (auto viter = txn->QueryVertexByKnnSearch(index_name, query, 10, 100);
       viter->Valid(); viter->Next()) {
    count++;
  }
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> elapsed = end - start;
  LOG_INFO("count:{}, elapsed: {}", count, elapsed.count());
  txn->Commit();
}

TEST(VectorIndex, del) {
  fs::remove_all(testdb);
  auto graphDB = GraphDB::Open(testdb, {});
  std::string index_name = "vector_index";
  graphDB->AddVertexVectorIndex(index_name, "label1", "embedding", 4, "l2", 16,
                                100);
  auto txn = graphDB->BeginTransaction();
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(1)},
                     {"embedding", Value::DoubleArray({1.0, 1.0, 1.0, 1.0})}});
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(2)},
                     {"embedding", Value::DoubleArray({2.0, 2.0, 2.0, 2.0})}});
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(3)},
                     {"embedding", Value::DoubleArray({3.0, 3.0, 3.0, 3.0})}});
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(4)},
                     {"embedding", Value::DoubleArray({4.0, 4.0, 4.0, 4.0})}});
  txn->Commit();
  for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
    index->ApplyWAL();
  }
  txn = graphDB->BeginTransaction();
  std::set<int64_t> ids, expect;
  for (auto viter = txn->QueryVertexByKnnSearch(index_name,
                                                {1.0, 2.0, 3.0, 4.0}, 10, 100);
       viter->Valid(); viter->Next()) {
    auto id = viter->GetVertexScore().vertex.GetProperty("id").AsInteger();
    ids.insert(id);
  }
  expect = {1, 2, 3, 4};
  EXPECT_EQ(ids, expect);
  cypher::RTContext rtx;
  txn->Execute(&rtx, "match(n {id:1}) delete n")->Consume();
  txn->Commit();
  for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
    index->ApplyWAL();
  }
  txn = graphDB->BeginTransaction();
  ids.clear();
  for (auto viter = txn->QueryVertexByKnnSearch(index_name,
                                                {1.0, 2.0, 3.0, 4.0}, 10, 100);
       viter->Valid(); viter->Next()) {
    auto id = viter->GetVertexScore().vertex.GetProperty("id").AsInteger();
    ids.insert(id);
  }
  expect = {2, 3, 4};
  EXPECT_EQ(ids, expect);
  txn->Commit();
  txn = graphDB->BeginTransaction();
  auto resultIterator = txn->Execute(&rtx, "match p=(n)-[*..1]-(m) return p");
  LOG_INFO(resultIterator->GetHeader());
  for (; resultIterator->Valid(); resultIterator->Next()) {
    LOG_INFO(resultIterator->GetRecord());
  }
  txn->Commit();
}

TEST(VectorIndex, restart) {
  fs::remove_all(testdb);
  std::string index_name = "vector_index";
  {
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexVectorIndex(index_name, "label1", "embedding", 4, "l2",
                                  16, 100);
    auto txn = graphDB->BeginTransaction();
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(1)},
                     {"embedding", Value::DoubleArray({1.0, 1.0, 1.0, 1.0})}});
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(2)},
                     {"embedding", Value::DoubleArray({2.0, 2.0, 2.0, 2.0})}});
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(3)},
                     {"embedding", Value::DoubleArray({3.0, 3.0, 3.0, 3.0})}});
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(4)},
                     {"embedding", Value::DoubleArray({4.0, 4.0, 4.0, 4.0})}});
    txn->Commit();
  }
  {
    auto graphDB = GraphDB::Open(testdb, {});
    for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
      index->ApplyWAL();
    }
    auto txn = graphDB->BeginTransaction();
    std::set<int64_t> ids, expect;
    for (auto viter = txn->QueryVertexByKnnSearch(
             index_name, {1.0, 2.0, 3.0, 4.0}, 10, 100);
         viter->Valid(); viter->Next()) {
      auto id = viter->GetVertexScore().vertex.GetProperty("id").AsInteger();
      ids.insert(id);
    }
    expect = {1, 2, 3, 4};
    EXPECT_EQ(ids, expect);
  }
}

TEST(VectorIndex, serialize) {
  fs::remove_all(testdb);
  std::string index_name = "vector_index";
  FLAGS_vt_serialize_interval = 3;
  {
    auto graphDB = GraphDB::Open(testdb, {});
    graphDB->AddVertexVectorIndex(index_name, "label1", "embedding", 4, "l2",
                                  16, 100);
    auto txn = graphDB->BeginTransaction();
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(1)},
                     {"embedding", Value::DoubleArray({1.0, 1.0, 1.0, 1.0})}});
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(2)},
                     {"embedding", Value::DoubleArray({2.0, 2.0, 2.0, 2.0})}});
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(3)},
                     {"embedding", Value::DoubleArray({3.0, 3.0, 3.0, 3.0})}});
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(4)},
                     {"embedding", Value::DoubleArray({4.0, 4.0, 4.0, 4.0})}});
    txn->Commit();
    for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
      index->ApplyWAL();
    }
  }
  {
    LOG_INFO("restart graphdb");
    auto graphDB = GraphDB::Open(testdb, {});
    for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
      index->ApplyWAL();
    }
    auto txn = graphDB->BeginTransaction();
    std::set<int64_t> ids, expect;
    for (auto viter = txn->QueryVertexByKnnSearch(
             index_name, {1.0, 2.0, 3.0, 4.0}, 10, 100);
         viter->Valid(); viter->Next()) {
      auto id = viter->GetVertexScore().vertex.GetProperty("id").AsInteger();
      ids.insert(id);
    }
    expect = {1, 2, 3, 4};
    EXPECT_EQ(ids, expect);
  }
}

TEST(VectorIndex, deleteLabelsUpdatesMembershipCorrectly) {
  fs::remove_all(testdb);
  auto graphDB = GraphDB::Open(testdb, {});
  std::string index_name = "vector_index";
  graphDB->AddVertexVectorIndex(index_name, "label1", "embedding", 4, "l2", 16,
                                100);

  auto txn = graphDB->BeginTransaction();
  txn->CreateVertex({"label1", "label2"},
                    {{"id", Value::Integer(1)},
                     {"embedding", Value::DoubleArray({1.0, 1.0, 1.0, 1.0})}});
  txn->CreateVertex({"label1"},
                    {{"id", Value::Integer(2)},
                     {"embedding", Value::DoubleArray({2.0, 2.0, 2.0, 2.0})}});
  txn->Commit();

  for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
    index->ApplyWAL();
  }

  txn = graphDB->BeginTransaction();
  std::set<int64_t> ids;
  for (auto viter = txn->QueryVertexByKnnSearch(index_name,
                                                {1.0, 1.0, 1.0, 1.0}, 10, 100);
       viter->Valid(); viter->Next()) {
    ids.insert(viter->GetVertexScore().vertex.GetProperty("id").AsInteger());
  }
  EXPECT_EQ(ids, (std::set<int64_t>{1, 2}));
  txn->Commit();

  txn = graphDB->BeginTransaction();
  auto keep_vertex = txn->NewVertexIterator(
      "label1",
      std::unordered_map<std::string, Value>{{"id", Value::Integer(1)}});
  EXPECT_TRUE(keep_vertex->Valid());
  keep_vertex->GetVertex().DeleteLabels({"label2"});

  auto remove_vertex = txn->NewVertexIterator(
      "label1",
      std::unordered_map<std::string, Value>{{"id", Value::Integer(2)}});
  EXPECT_TRUE(remove_vertex->Valid());
  remove_vertex->GetVertex().DeleteLabels({"label1"});
  txn->Commit();

  for (auto& [name, index] : graphDB->meta_info().GetVertexVectorIndex()) {
    index->ApplyWAL();
  }

  txn = graphDB->BeginTransaction();
  ids.clear();
  for (auto viter = txn->QueryVertexByKnnSearch(index_name,
                                                {1.0, 1.0, 1.0, 1.0}, 10, 100);
       viter->Valid(); viter->Next()) {
    ids.insert(viter->GetVertexScore().vertex.GetProperty("id").AsInteger());
  }
  EXPECT_EQ(ids, (std::set<int64_t>{1}));

  auto remaining = txn->NewVertexIterator(
      "label1",
      std::unordered_map<std::string, Value>{{"id", Value::Integer(1)}});
  EXPECT_TRUE(remaining->Valid());
  EXPECT_EQ(remaining->GetVertex().GetLabels(),
            (std::unordered_set<std::string>{"label1"}));

  auto removed = txn->NewVertexIterator(
      "label1",
      std::unordered_map<std::string, Value>{{"id", Value::Integer(2)}});
  EXPECT_FALSE(removed->Valid());
  txn->Commit();
}

TEST(VectorIndex, buildBlocksWrites) {
  fs::remove_all(testdb);
  auto graphDB = GraphDB::Open(testdb, {});
  std::string index_name = "vector_index";

  auto txn = graphDB->BeginTransaction();
  for (int i = 0; i < 10000; ++i) {
    txn->CreateVertex(
        {"label1"}, {{"id", Value::Integer(i)},
                     {"embedding", Value::DoubleArray({1.0, 2.0, 3.0, 4.0, 5.0,
                                                       6.0, 7.0, 8.0})}});
  }
  txn->Commit();

  auto lid = graphDB->id_generator().GetOrCreateLid("label1");
  auto pid = graphDB->id_generator().GetOrCreatePid("embedding");
  std::exception_ptr build_error;
  std::thread builder([&]() {
    try {
      graphDB->AddVertexVectorIndex(index_name, "label1", "embedding", 8, "l2",
                                    16, 100);
    } catch (...) {
      build_error = std::current_exception();
    }
  });

  ASSERT_TRUE(WaitUntilBusy(graphDB.get(), {lid}, {pid}));

  bool blocked = false;
  auto deadline = std::chrono::steady_clock::now() + std::chrono::seconds(2);
  while (std::chrono::steady_clock::now() < deadline && !blocked) {
    auto write_txn = graphDB->BeginTransaction();
    try {
      write_txn->CreateVertex(
          {"label1"},
          {{"id", Value::Integer(20000)},
           {"embedding",
            Value::DoubleArray({1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0})}});
      write_txn->Rollback();
    } catch (LgraphException& e) {
      write_txn->Rollback();
      if (e.code() == ErrorCode::IndexBusy) {
        blocked = true;
        break;
      }
      FAIL() << "Unexpected exception message: " << e.what();
    }
  }

  builder.join();
  if (build_error) {
    try {
      std::rethrow_exception(build_error);
    } catch (const std::exception& e) {
      FAIL() << e.what();
    }
  }

  EXPECT_TRUE(blocked);

  txn = graphDB->BeginTransaction();
  int count = 0;
  for (auto viter = txn->QueryVertexByKnnSearch(
           index_name, {1.0, 2.0, 3.0, 4.0, 5.0, 6.0, 7.0, 8.0}, 10, 100);
       viter->Valid(); viter->Next()) {
    count++;
  }
  EXPECT_EQ(count, 10);
  txn->Commit();
}
