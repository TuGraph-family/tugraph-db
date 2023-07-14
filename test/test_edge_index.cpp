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
#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"

#include "gtest/gtest.h"

#include "core/kv_store.h"
#include "core/mock_kv.h"
#include "core/edge_index.h"
#include "core/lightning_graph.h"
#include "./ut_utils.h"

class TestEdgeIndex : public TuGraphTest {};

using namespace lgraph;
using namespace fma_common;

int TestEdgeIndexImpl() {
    AutoCleanDir d1("./testkv");
    KvStore store("./testkv", (size_t)1 << 30, true);
    // Start test, see if we already has a db
    KvTransaction txn = store.CreateWriteTxn();
    store.DropAll(txn);
    txn.Commit();

    // now create the index table
    txn = store.CreateWriteTxn();
    // test integer keys
    {
        KvTable idx_tab =
            EdgeIndex::OpenTable(txn, store, "int32_index_test", FieldType::INT32, false);
        EdgeIndex idx(idx_tab, FieldType::INT32, false);
        // add many eids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(1), i, i + 2, 1, i, j));
            }
        }
        for (int64_t i = 100; i < 600; i += 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(2), i, i + 2, 1, i, j));
            }
        }
        // add some eids
        for (int64_t i = 98; i >= 0; i -= 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(2), i, i + 2, 1, i, j));
            }
        }
        // add some euids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(!idx.Add(txn, Value::ConstRef(2), i, i + 2, 1, i, j));
            }
        }
        auto tit = idx.GetUnmanagedIterator(txn, Value::ConstRef(2), Value::ConstRef(2));
        while (tit.GetSrcVid() != 80) tit.Next();
        UT_EXPECT_TRUE(tit.IsValid());
        // delete some euids
        for (int64_t i = 88; i >= 80; i -= 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Delete(txn, Value::ConstRef(2), i, i + 2, 1, i, j));
            }
        }
        tit.RefreshContentIfKvIteratorModified();
        UT_EXPECT_TRUE(tit.IsValid());
        // test updates
        UT_EXPECT_TRUE(idx.Update(txn, Value::ConstRef(2), Value::ConstRef(1), 0, 2, 1, 0, 1));
        // output indexed euids
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(1), Value::ConstRef(1));
            int64_t local_vid = 1;
            int64_t local_eid = 0;
            it.Next();
            while (it.IsValid()) {
                local_vid += 2 * (local_eid / 100);
                local_eid = (local_eid % 100) + 1;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                UT_EXPECT_EQ(it.GetTemporalId(), local_vid);
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(2), Value::ConstRef(2));
            int64_t local_vid = 0;
            int64_t local_eid = 1;
            while (it.IsValid()) {
                local_vid += 2 * (local_eid / 100);
                if (local_vid == 80) local_vid += 10;
                local_eid = (local_eid % 100) + 1;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(1), Value::ConstRef(2));
            int64_t local_vid = 1;
            int64_t local_eid = 0;
            it.Next();
            while (it.IsValid()) {
                local_vid += 2 * (local_eid / 100);
                local_eid = (local_eid % 100) + 1;
                if (local_vid > 499) break;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                it.Next();
            }
            local_vid = 0;
            local_eid = 1;
            while (it.IsValid()) {
                local_vid += 2 * (local_eid / 100);
                if (local_vid == 80) local_vid += 10;
                local_eid = (local_eid % 100) + 1;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(2), Value::ConstRef(1));
            UT_EXPECT_FALSE(it.IsValid());
        }
    }
    // test Fullkeys
    {
        KvTable idx_tab =
            EdgeIndex::OpenTable(txn, store, "stringss_index_test", FieldType::STRING, true);
        EdgeIndex idx(idx_tab, FieldType::STRING, true);
        // add many euids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(
                idx.Add(txn, Value::ConstRef("amazon story" +
                        std::to_string(i)), i, i + 2, 1, 0, 1));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(
                idx.Add(txn, Value::ConstRef("breakup" + std::to_string(i)), i, i + 2, 1, 0, 1));
        }
        // add some euids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(
                idx.Add(txn, Value::ConstRef("breakup" + std::to_string(i)), i, i + 2, 1, 0, 1));
        }
        // add some euids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(
                !idx.Add(txn, Value::ConstRef("breakup" + std::to_string(i)), i, i + 2, 1, 0, 1));
        }
        // delete some euids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(
                idx.Delete(txn, Value::ConstRef("breakup" + std::to_string(i)), i, i + 2, 1, 0, 1));
        }
        // output indexed eids
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("amazon story"),
                                               Value::ConstRef("a"));
            int64_t local_vid = 1;
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                local_vid += 2;
                it.Next();
            }
        }
        {
            auto it =
                idx.GetUnmanagedIterator(txn, Value::ConstRef("breakup"), Value::ConstRef("b"));
            int64_t local_vid = 0;
            int64_t local_eid = 1;
            while (it.IsValid()) {
                if (local_vid == 80) local_vid += 10;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                local_vid += 2;
                it.Next();
            }
        }
    }

    // test string keys
    {
        KvTable idx_tab =
            EdgeIndex::OpenTable(txn, store, "string_index_test", FieldType::STRING, false);
        EdgeIndex idx(idx_tab, FieldType::STRING, false);
        // add many euids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("a"), i, i + 2, 1, 0, 1));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("b"), i, i + 2, 1, 0, 1));
        }
        // add some euids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("b"), i, i + 2, 1, 0, 1));
        }
        // add some euids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(!idx.Add(txn, Value::ConstRef("b"), i, i + 2, 1, 0, 1));
        }
        // delete some euids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(idx.Delete(txn, Value::ConstRef("b"), i, i + 2, 1, 0, 1));
        }
        // output indexed eids
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("a"), Value::ConstRef("a"));
            int64_t local_vid = 1;
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                local_vid += 2;
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("b"), Value::ConstRef("b"));
            int64_t local_vid = 0;
            int64_t local_eid = 1;
            while (it.IsValid()) {
                if (local_vid == 80) local_vid += 10;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                local_vid += 2;
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("a"), Value::ConstRef("b"));
            int64_t local_vid = 1;
            int64_t local_eid = 1;
            while (it.IsValid()) {
                if (local_vid > 500) break;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                local_vid += 2;
                it.Next();
            }
            local_vid = 0;
            local_eid = 1;
            while (it.IsValid()) {
                if (local_vid == 80) local_vid += 10;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                local_vid += 2;
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("b"), Value::ConstRef("a"));
            UT_EXPECT_FALSE(it.IsValid());
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("a"), Value::ConstRef("bb"));
            int64_t local_vid = 1;
            int64_t local_eid = 1;
            while (it.IsValid()) {
                if (local_vid > 500) break;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                local_vid += 2;
                it.Next();
            }
            local_vid = 0;
            local_eid = 1;
            while (it.IsValid()) {
                if (local_vid == 80) local_vid += 10;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                local_vid += 2;
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("aa"), Value::ConstRef("bb"));
            int64_t local_vid = 0;
            int64_t local_eid = 1;
            while (it.IsValid()) {
                if (local_vid == 80) local_vid += 10;
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                UT_EXPECT_EQ(it.GetEid(), local_eid);
                local_vid += 2;
                it.Next();
            }
        }
    }
    // test index scan
    {
        // index with non-unique keys
        {
            KvTable idx_tab = EdgeIndex::OpenTable(txn, store, "string_index_non_unqiue",
                                                   FieldType::STRING, false);
            EdgeIndex idx(idx_tab, FieldType::STRING, false);
            // add many vids; should split after some point
            for (int64_t i = 0; i < 500; i += 1) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("1"), i, i + 2, 1, 0, 1));
            }
            for (int64_t i = 500; i < 600; i += 1) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("2"), i, i + 2, 1, 0, 1));
            }
            size_t n = 0;
            for (auto it = idx.GetUnmanagedIterator(txn, Value(), Value()); it.IsValid(); it.Next())
                n++;
            UT_EXPECT_EQ(n, 600);
        }
        // index with non-unique keys
        {
            KvTable idx_tab =
                EdgeIndex::OpenTable(txn, store, "int32_index_unqiue", FieldType::INT32, false);
            EdgeIndex idx(idx_tab, FieldType::INT32, false);
            // add many vids; should split after some point
            for (int32_t i = 0; i < 100; i += 1) {
                for (int j = 1; j <= 100; ++j) {
                    UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(i), i, i + 2, 1, 0, j));
                }
            }
            size_t n = 0;
            for (auto it = idx.GetUnmanagedIterator(txn, Value(), Value()); it.IsValid(); it.Next())
                n++;
            UT_EXPECT_EQ(n, 10000);
        }
    }

    txn.Commit();

    return 0;
}

TEST_F(TestEdgeIndex, EdgeIndex) {
    TestEdgeIndexImpl();
}

TEST_F(TestEdgeIndex, DeleteVertex) {
    AutoCleanDir cleaner("./testdb");
    DBConfig config;
    config.dir = "./testdb";
    LightningGraph db(config);
    db.DropAllData();
    std::vector<FieldSpec> v_fds = {{"name", FieldType::STRING, false},
                                    {"title", FieldType::STRING, false},
                                    {"description", FieldType::STRING, true},
                                    {"type", FieldType::INT8, false}};
    std::vector<FieldSpec> e_fds = {{"name", FieldType::STRING, false},
                                    {"comments", FieldType::STRING, true},
                                    {"weight", FieldType::FLOAT, false}};
    UT_EXPECT_TRUE(db.AddLabel("v1", v_fds, true, "name", {}));
    UT_EXPECT_TRUE(db.AddLabel("e1", e_fds, false, {}, {}));
    // add edge index
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "name", false, false));
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "comments", false, false));
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "weight", false, false));

    Transaction txn = db.CreateWriteTxn();
    std::vector<std::string> v1_properties = {"name", "title", "description", "type"};
    VertexId v_id1 = txn.AddVertex(
        std::string("v1"), v1_properties,
        std::vector<std::string>{"name1", "title1", "desc1", "1"});
    VertexId v_id2 = txn.AddVertex(
        std::string("v1"), v1_properties,
        std::vector<std::string>{"name2", "title2", "desc2", "2"});
    VertexId v_id3 = txn.AddVertex(
        std::string("v1"), v1_properties,
        std::vector<std::string>{"name3", "title3", "desc3", "3"});
    VertexId v_id4 = txn.AddVertex(
        std::string("v1"), v1_properties,
        std::vector<std::string>{"name4", "title4", "desc4", "4"});
    txn.Commit();

    // add edge
    txn = db.CreateWriteTxn();
    std::vector<std::string> e1_properties = {"name", "comments", "weight"};
    EdgeUid e_id1 =
        txn.AddEdge(v_id1, v_id2, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name1", "comments1", "10"});
    EdgeUid e_id2 =
        txn.AddEdge(v_id2, v_id1, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name2", "comments2", "12"});
    EdgeUid e_id3 =
        txn.AddEdge(v_id3, v_id4, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name3", "comments3", "14"});
    EdgeUid e_id4 =
        txn.AddEdge(v_id4, v_id3, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name4", "comments4", "15"});

    EdgeUid e_id5 =
        txn.AddEdge(v_id1, v_id1, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name5", "comments5", "20"});

    EdgeUid e_id6 =
        txn.AddEdge(v_id2, v_id2, std::string("e1"), e1_properties,
                    std::vector<std::string>{"name6", "comments6", "20"});
    txn.Commit();

    txn = db.CreateReadTxn();
    auto counts_befor = txn.countDetail();
    UT_EXPECT_EQ(counts_befor.size(), 2);
    UT_EXPECT_EQ(std::get<2>(counts_befor[0]), 4);
    UT_EXPECT_EQ(std::get<2>(counts_befor[1]), 6);
    txn.Abort();
    db.RefreshCount();
    txn = db.CreateReadTxn();
    auto counts_after = txn.countDetail();
    UT_EXPECT_EQ(counts_befor, counts_after);
    txn.Abort();

    txn = db.CreateWriteTxn();
    size_t n_in = 0;
    size_t n_out = 0;
    UT_EXPECT_TRUE(txn.DeleteVertex(v_id1, &n_in, &n_out));
    UT_EXPECT_EQ(n_in, 2);
    UT_EXPECT_EQ(n_out, 2);
    std::unordered_set<std::string> res = {"name3", "name4", "name6"};
    int count = 0;
    for (auto it = txn.GetEdgeIndexIterator("e1", "name"); it.IsValid();
         it.Next()) {
        count++;
        auto k = it.GetKey();
        UT_EXPECT_TRUE(res.count(k.AsString()));
    }
    UT_EXPECT_EQ(count, 3);
    txn.Commit();
}
