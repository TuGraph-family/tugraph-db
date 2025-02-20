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

#include <random>
#include "fma-common/configuration.h"
#include "fma-common/file_system.h"

#include "gtest/gtest.h"

#include "db/galaxy.h"
#include "db/db.h"
#include "./ut_utils.h"

class TestSnapshot : public TuGraphTest {};

static lgraph::VertexId AddVertex(lgraph::Transaction& txn, const std::string& name,
                                  const std::string& type, bool enable_fast_alter) {
    std::vector<size_t> fids;
    if (enable_fast_alter) {
        fids = {0, 1};  // there is no need to reorder the fields.
    } else {
        fids = {1, 0};
    }
    std::vector<std::string> values = {name, type};
    return txn.AddVertex((size_t)0, fids, values);
}

static lgraph::EdgeId AddEdge(lgraph::Transaction& txn, lgraph::VertexId src, lgraph::VertexId dst,
                              int8_t source, float weight) {
    std::vector<size_t> fids = {0, 1};
    lgraph::FieldData s(source);
    lgraph::FieldData w(weight);
    std::vector<lgraph::FieldData> values = {s, w};
    return txn.AddEdge(src, dst, (size_t)0, fids, values).eid;
}

void CreateTestDB() {
    using namespace lgraph;

    fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
    Galaxy galaxy("./testdb");
    AccessControlledDB db = galaxy.OpenGraph("admin", "default");
    std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                    {"type", FieldType::INT8, false}};
    std::vector<FieldSpec> e_fds = {{"source", FieldType::INT8, false},
                                    {"weight", FieldType::FLOAT, false}};
    UT_EXPECT_TRUE(db.AddLabel(true, "v", v_fds, VertexOptions("name")));
    UT_EXPECT_TRUE(db.AddLabel(false, "e", e_fds, EdgeOptions()));
    // ASSERT(db.AddVertexIndex("v", "name", false));
    while (!db.IsVertexIndexed("v", "name")) fma_common::SleepUs(100);

    Transaction txn = db.CreateWriteTxn();
    VertexId vid1_ = AddVertex(txn, "v1", "1", false);
    VertexId vid2_ = AddVertex(txn, "v2", "2", false);

    {
        VertexIndexIterator iit = txn.GetVertexIndexIterator("v", "name", "v2", "v2");
        UT_EXPECT_TRUE(iit.IsValid());
        UT_EXPECT_EQ(iit.GetVid(), 1);
    }
    AddEdge(txn, vid1_, vid1_, 11, 11.0);
    AddEdge(txn, vid1_, vid2_, 12, 12.0);
    AddEdge(txn, vid2_, vid1_, 21, 21.0);
    AddEdge(txn, vid2_, vid2_, 22, 22.0);

    txn.Commit();
}

TEST_F(TestSnapshot, Snapshot) {
    using namespace lgraph;
    using namespace fma_common;

    size_t n = 10;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(n, "n_snapshot,n", true).Comment("Number of times to do snapshot");
    config.ParseAndFinalize(argc, argv);
    { CreateTestDB(); }
    {
        // create snapshot

        Galaxy galaxy("./testdb");
        std::vector<std::string> vector_graph = {"default"};
        galaxy.WarmUp("admin", vector_graph);
        fma_common::file_system::RemoveDir("./backup");
        galaxy.Backup("./backup", true);
        fma_common::file_system::RemoveDir("./snap");
        std::string path_snap = "./snap";
        galaxy.SaveSnapshot(path_snap);
        // load snapshot
        UT_EXPECT_TRUE(galaxy.LoadSnapshot(path_snap));
    }
    fma_common::file_system::RemoveDir("./snap");
    fma_common::SleepS(1);  // waiting for memory reclaiming by async task
}
