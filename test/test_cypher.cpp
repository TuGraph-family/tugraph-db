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
#include <set>

#include "common/value.h"
#include "cypher/execution_plan/result_iterator.h"
#include "graphdb/graph_db.h"

using namespace graphdb;
namespace fs = std::filesystem;
static std::string testdb = "cypher_testdb";

TEST(Cypher, unwind_create_three_vertices) {
  fs::remove_all(testdb);
  auto graphDB = GraphDB::Open(testdb, {});
  cypher::RTContext rtx;

  auto txn = graphDB->BeginTransaction();
  txn->Execute(&rtx, "unwind [1,2,3] as num create(n:test) set n.id = num;")
      ->Consume();
  txn->Commit();

  txn = graphDB->BeginTransaction();
  size_t count = 0;
  std::set<int64_t> ids;
  for (auto viter = txn->NewVertexIterator("test"); viter->Valid();
       viter->Next()) {
    count++;
    ids.insert(viter->GetVertex().GetProperty("id").AsInteger());
  }

  EXPECT_EQ(count, 3);
  EXPECT_EQ(ids, (std::set<int64_t>{1, 2, 3}));
  txn->Commit();
}
