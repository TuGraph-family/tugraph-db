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

#include "common/value.h"
#include "server/galaxy.h"
#include "test_util.h"
#include "transaction/transaction.h"
namespace fs = std::filesystem;
std::string test_galaxy = "test_galaxy";
TEST(Galaxy, basic) {
  fs::remove_all(test_galaxy);
  auto galaxy = server::Galaxy::Open(test_galaxy, {});
  {
    auto graphDB = galaxy->OpenGraph("default");
    EXPECT_TRUE(graphDB != nullptr);
  }
  galaxy->CreateGraph("graph1");
  galaxy.reset(nullptr);
  galaxy = server::Galaxy::Open(test_galaxy, {});
  EXPECT_TRUE(galaxy->OpenGraph("default") != nullptr);
  EXPECT_TRUE(galaxy->OpenGraph("graph1") != nullptr);
  galaxy->DeleteGraph("graph1");
  EXPECT_THROW_CODE(galaxy->OpenGraph("graph1"), NoSuchGraph);
  galaxy.reset();
  galaxy = server::Galaxy::Open(test_galaxy, {});
  EXPECT_TRUE(galaxy->OpenGraph("default") != nullptr);
  EXPECT_THROW_CODE(galaxy->OpenGraph("graph1"), NoSuchGraph);
  galaxy.reset();
}

TEST(Galaxy, DISABLED_createGraph) {
  fs::remove_all(test_galaxy);
  auto galaxy = server::Galaxy::Open(test_galaxy, {});
  for (int i = 0; i < 10000; i++) {
    galaxy->CreateGraph("graph" + std::to_string(i));
  }
  std::this_thread::sleep_for(std::chrono::seconds(30));
}

TEST(Galaxy, clearGraph) {
  fs::remove_all(test_galaxy);
  auto galaxy = server::Galaxy::Open(test_galaxy, {});
  galaxy->CreateGraph("graph1");
  auto old_graph = galaxy->OpenGraph("graph1");
  auto old_graph_id = old_graph->db_meta().graph_id();
  auto old_path = old_graph->path();

  auto txn = old_graph->BeginTransaction();
  txn->CreateVertex({"person"}, {{"id", Value::Integer(1)}});
  txn->Commit();

  auto cleared_graph = galaxy->ClearGraph("graph1");
  EXPECT_NE(cleared_graph->db_meta().graph_id(), old_graph_id);
  EXPECT_EQ(cleared_graph->db_meta().graph_name(), "graph1");

  auto cleared_txn = cleared_graph->BeginTransaction();
  int vertex_count = 0;
  for (auto viter = cleared_txn->NewVertexIterator(); viter->Valid();
       viter->Next()) {
    vertex_count++;
  }
  EXPECT_EQ(vertex_count, 0);
  cleared_txn->Commit();

  txn.reset();
  old_graph.reset();
  EXPECT_FALSE(fs::exists(old_path));
}
