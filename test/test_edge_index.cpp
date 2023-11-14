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
    auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << 30, true);
    // Start test, see if we already has a db
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
    txn->Commit();

    // now create the index table
    txn = store->CreateWriteTxn();
    // test integer keys
    {
        auto idx_tab =
            EdgeIndex::OpenTable(*txn, *store, "int32_index_test", FieldType::INT32,
                                 lgraph::IndexType::NonuniqueIndex);
        EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
        // add many eids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), {i, i + 2, 1, i, j}));
            }
        }
        for (int64_t i = 100; i < 600; i += 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(2), {i, i + 2, 1, i, j}));
            }
        }
        // add some eids
        for (int64_t i = 98; i >= 0; i -= 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(2), {i, i + 2, 1, i, j}));
            }
        }
        // add some euids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(!idx.Add(*txn, Value::ConstRef(2), {i, i + 2, 1, i, j}));
            }
        }
        auto tit = idx.GetUnmanagedIterator(*txn, Value::ConstRef(2), Value::ConstRef(2));
        UT_EXPECT_TRUE(tit.IsValid());
        // delete some euids
        for (int64_t i = 88; i >= 80; i -= 2) {
            for (int j = 1; j <= 100; ++j) {
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(2), {i, i + 2, 1, i, j}));
            }
        }
        tit.RefreshContentIfKvIteratorModified();
        UT_EXPECT_TRUE(tit.IsValid());
        // test updates
        UT_EXPECT_TRUE(idx.Update(*txn, Value::ConstRef(2), Value::ConstRef(1), {0, 2, 1, 0, 1}));
        // output indexed euids
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(1), Value::ConstRef(1));
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
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(2), Value::ConstRef(2));
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
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(1), Value::ConstRef(2));
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
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(2), Value::ConstRef(1));
            UT_EXPECT_FALSE(it.IsValid());
        }
    }
    // test Fullkeys
    {
        auto idx_tab =
            EdgeIndex::OpenTable(*txn, *store, "stringss_index_test", FieldType::STRING,
                                 lgraph::IndexType::PairUniqueIndex);
        EdgeIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::PairUniqueIndex);
        // add many euids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(
                idx.Add(*txn, Value::ConstRef("amazon story" +
                        std::to_string(i)), {i, i + 2, 1, 0, 1}));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(
                idx.Add(*txn, Value::ConstRef("breakup" + std::to_string(i)), {i, i + 2, 1, 0, 1}));
        }
        // add some euids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(
                idx.Add(*txn, Value::ConstRef("breakup" + std::to_string(i)), {i, i + 2, 1, 0, 1}));
        }
        // add some euids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(!idx.Add(*txn, Value::ConstRef("breakup" + std::to_string(i)),
                                    {i, i + 2, 1, 0, 1}));
        }
        // delete some euids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(
                idx.Delete(*txn, Value::ConstRef("breakup" + std::to_string(i)),
                           {i, i + 2, 1, 0, 1}));
        }
        // output indexed eids
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("amazon story"),
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
                idx.GetUnmanagedIterator(*txn, Value::ConstRef("breakup"), Value::ConstRef("b"));
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
        auto idx_tab =
            EdgeIndex::OpenTable(*txn, *store, "string_index_test", FieldType::STRING,
                                 lgraph::IndexType::NonuniqueIndex);
        EdgeIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
        // add many euids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("a"), {i, i + 2, 1, 0, 1}));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("b"), {i, i + 2, 1, 0, 1}));
        }
        // add some euids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("b"), {i, i + 2, 1, 0, 1}));
        }
        // add some euids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(!idx.Add(*txn, Value::ConstRef("b"), {i, i + 2, 1, 0, 1}));
        }
        // delete some euids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef("b"), {i, i + 2, 1, 0, 1}));
        }
        // output indexed eids
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("a"), Value::ConstRef("a"));
            int64_t local_vid = 1;
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetSrcVid(), local_vid);
                local_vid += 2;
                it.Next();
            }
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("b"), Value::ConstRef("b"));
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
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("a"), Value::ConstRef("b"));
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
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("b"), Value::ConstRef("a"));
            UT_EXPECT_FALSE(it.IsValid());
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("a"), Value::ConstRef("bb"));
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
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("aa"), Value::ConstRef("bb"));
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
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "string_index_non_unqiue",
                                         FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
            // add many vids; should split after some point
            for (int64_t i = 0; i < 500; i += 1) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("1"), {i, i + 2, 1, 0, 1}));
            }
            for (int64_t i = 500; i < 600; i += 1) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("2"), {i, i + 2, 1, 0, 1}));
            }
            size_t n = 0;
            for (auto it = idx.GetUnmanagedIterator(*txn, Value(), Value());
                 it.IsValid(); it.Next())
                n++;
            UT_EXPECT_EQ(n, 600);
        }
        // index with non-unique keys
        {
            auto idx_tab =
                EdgeIndex::OpenTable(*txn, *store, "int32_index_unqiue",
                                     FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            // add many vids; should split after some point
            for (int32_t i = 0; i < 100; i += 1) {
                for (int j = 1; j <= 100; ++j) {
                    UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(i), {i, i + 2, 1, 0, j}));
                }
            }
            size_t n = 0;
            for (auto it = idx.GetUnmanagedIterator(*txn, Value(), Value());
                 it.IsValid(); it.Next())
                n++;
            UT_EXPECT_EQ(n, 10000);
        }
    }

    txn->Commit();

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
                                    {"weight", FieldType::INT32, false}};

    UT_EXPECT_TRUE(db.AddLabel("v1", v_fds, true, VertexOptions("name")));
    UT_EXPECT_TRUE(db.AddLabel("e1", e_fds, false, EdgeOptions()));

    // add edge index
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "name", lgraph::IndexType::NonuniqueIndex, false));
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "comments", lgraph::IndexType::NonuniqueIndex, false));
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "weight", lgraph::IndexType::NonuniqueIndex, false));

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
    txn.AddEdge(v_id1, v_id2, std::string("e1"), e1_properties,
                std::vector<std::string>{"name1", "comments1", "10"});
    txn.AddEdge(v_id2, v_id1, std::string("e1"), e1_properties,
                std::vector<std::string>{"name2", "comments2", "12"});
    txn.AddEdge(v_id3, v_id4, std::string("e1"), e1_properties,
                std::vector<std::string>{"name3", "comments3", "14"});
    txn.AddEdge(v_id4, v_id3, std::string("e1"), e1_properties,
                std::vector<std::string>{"name4", "comments4", "15"});
    txn.AddEdge(v_id1, v_id1, std::string("e1"), e1_properties,
                std::vector<std::string>{"name5", "comments5", "20"});
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

int TestEdgeIndexValue() {
    {
        auto fetch_n_bytes = [](const char* p, size_t nbytes) {
            int64_t val = 0;
            memcpy(&val, p, nbytes);
            return val;
        };
        // test EdgeIndexValue
        EdgeIndexValue eiv;
        for (size_t i = 1; i <= 100; ++i) {
            eiv.InsertEUid(EdgeUid(i, i + 1, 1, i, i));
            UT_EXPECT_EQ(eiv.GetEUidCount(), i);
            bool found = false;
            eiv.SearchEUid(EdgeUid(i, i + 1, 1, i, i), found);

            UT_EXPECT_TRUE(found);
        }
        UT_EXPECT_TRUE(eiv.TooLarge());
        Value v = eiv.CreateKey(Value());
        UT_EXPECT_EQ(v.Size(), 24);
        UT_EXPECT_EQ(fetch_n_bytes(v.Data(), 5), 100);
        UT_EXPECT_EQ(fetch_n_bytes(v.Data() + 5, 5), 101);
        UT_EXPECT_EQ(fetch_n_bytes(v.Data() + 10, 2), 1);
        UT_EXPECT_EQ(fetch_n_bytes(v.Data() + 12, 8), 100);
        UT_EXPECT_EQ(fetch_n_bytes(v.Data() + 20, 4), 100);

        for (size_t i = 0; i < 100; ++i) {
            VertexId src = eiv.GetNthSrcVid(i);
            UT_EXPECT_EQ(src, i + 1);
            VertexId dst = eiv.GetNthDstVid(i);
            UT_EXPECT_EQ(dst, i + 2);
            LabelId lid = eiv.GetNthLid(i);
            UT_EXPECT_EQ(lid, 1);
            TemporalId tid = eiv.GetNthTid(i);
            UT_EXPECT_EQ(tid, i + 1);
            EdgeId eid = eiv.GetNthEid(i);
            UT_EXPECT_EQ(eid, i + 1);
        }

        EdgeIndexValue right = eiv.SplitRightHalf();
        UT_EXPECT_EQ(right.GetEUidCount(), 50);
        UT_EXPECT_EQ(eiv.GetEUidCount(), 50);

        for (size_t i = 1; i < 50; ++i) {
            uint8_t ret = eiv.DeleteEUidIfExist(EdgeUid(i, i + 1, 1, i, i));
            UT_EXPECT_EQ(ret, 1);
        }
        UT_EXPECT_EQ(eiv.DeleteEUidIfExist({50, 51, 1, 50, 50}), 2);
        UT_EXPECT_EQ(eiv.GetEUidCount(), 0);
        EdgeIndexValue giv;
        for (size_t i = 1; i <= 100; ++i) {
            giv.InsertEUid(EdgeUid(i, i + 1, 1, i, i));
            UT_EXPECT_EQ(giv.GetEUidCount(), i);
            bool found = false;
            giv.SearchEUid(EdgeUid(i, i + 1, 1, i, i), found);
            UT_EXPECT_TRUE(found);
        }

        Value g = giv.CreateKey(Value());
        UT_EXPECT_EQ(g.Size(), 24);
        UT_EXPECT_EQ(fetch_n_bytes(v.Data(), 5), 100);
        UT_EXPECT_EQ(fetch_n_bytes(v.Data() + 5, 5), 101);

        UT_EXPECT_TRUE(giv.TooLarge());
        for (size_t i = 0; i < 100; ++i) {
            LabelId lid = giv.GetNthLid(i);
            UT_EXPECT_EQ(lid, 1);
            TemporalId tid = giv.GetNthTid(i);
            UT_EXPECT_EQ(tid, i + 1);
            EdgeId eid = giv.GetNthEid(i);
            UT_EXPECT_EQ(eid, i + 1);
        }

        EdgeIndexValue gright = giv.SplitRightHalf();
        UT_EXPECT_EQ(gright.GetEUidCount(), 50);
        UT_EXPECT_EQ(giv.GetEUidCount(), 50);

        for (size_t i = 1; i < 50; ++i) {
            uint8_t ret = giv.DeleteEUidIfExist(EdgeUid(i, i + 1, 1, i, i));
            UT_EXPECT_EQ(ret, 1);
        }
        UT_EXPECT_EQ(giv.DeleteEUidIfExist({50, 51, 1, 50, 50}), 2);
        UT_EXPECT_EQ(giv.GetEUidCount(), 0);
    }
    return 0;
}  // end of TestEdgeIndexValue

int TestEdgeIndexCRUD() {
    auto fetch_n_bytes = [](const char* p, size_t nbytes) {
        int64_t val = 0;
        memcpy(&val, p, nbytes);
        return val;
    };
    auto int_unique_global_to_string = [&](const char* p, size_t s) {
        int32_t key = fetch_n_bytes(p, s);
        std::string ret = std::to_string(key);
        return ret;
    };
    auto int_normal_to_string = [&](const char* p, size_t s) {
        int64_t key = fetch_n_bytes(p, s - 24);
        std::string ret = std::to_string(key);
        int64_t src_vid = fetch_n_bytes(p + s - 24, 5);
        ret.append(std::to_string(src_vid));
        int64_t dst_vid = fetch_n_bytes(p + s - 19, 5);
        ret.append(std::to_string(dst_vid));
        int64_t lid = fetch_n_bytes(p + s - 14, 2);
        ret.append(std::to_string(lid));
        int64_t tid = fetch_n_bytes(p + s - 12, 8);
        ret.append(std::to_string(tid));
        int64_t eid = fetch_n_bytes(p + s - 4, 4);
        ret.append(std::to_string(eid));
        return ret;
    };
    auto int_pair_unique_to_string = [&] (const char* p, size_t s) {
        int64_t key = fetch_n_bytes(p, s - 10);
        std::string ret = std::to_string(key);
        int64_t src_vid = fetch_n_bytes(p + s - 10, 5);
        ret.append(std::to_string(src_vid));
        int64_t dst_vid = fetch_n_bytes(p + s - 5, 5);
        ret.append(std::to_string(dst_vid));
        return ret;
    };
    {
        // test EdgeIndex
        auto store = std::make_unique<LMDBKvStore>("./testdb", (size_t)1 << 30, true);
        // Start test, see if we already has a db
        auto txn = store->CreateWriteTxn();
        store->DropAll(*txn);
        txn->Commit();
        txn = store->CreateWriteTxn();

        // test integer unique global key
        {
            auto idx_tab =
                EdgeIndex::OpenTable(*txn, *store, "UniqueGlobalInt32", FieldType::INT32,
                                     lgraph::IndexType::GlobalUniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32,
                          lgraph::IndexType::GlobalUniqueIndex);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), {0, 0, 0, 0, 0}));
            std::string eidx;
            idx.Dump(*txn, int_unique_global_to_string, eidx);
            UT_EXPECT_TRUE(eidx.substr(0, 1) == "1");
        }

        // test integer non-unique global key
        {
            auto idx_tab =
                EdgeIndex::OpenTable(*txn, *store, "NonUniqueGlobalInt32",
                                     FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), {0, 0, 0, 0, 0}));
            std::string eidx;
            idx.Dump(*txn, int_normal_to_string, eidx);
            UT_EXPECT_TRUE(eidx.substr(0, 6) == "100000");
        }

        // test integer unique non-global key
        {
            auto idx_tab =
                EdgeIndex::OpenTable(*txn, *store, "UniqueNonGlobalInt32",
                                     FieldType::INT32, lgraph::IndexType::PairUniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::PairUniqueIndex);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), {3, 2, 0, 0, 0}));
            std::string eidx;
            idx.Dump(*txn, int_pair_unique_to_string, eidx);
            UT_EXPECT_TRUE(eidx.substr(0, 3) == "123");
        }

        // test integer non-unique non-global key
        {
            auto idx_tab =
                EdgeIndex::OpenTable(*txn, *store, "NonUniqueNonGlobalInt32",
                                     FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), {9, 8, 7, 6, 5}));
            std::string eidx;
            idx.Dump(*txn, int_normal_to_string, eidx);
            UT_EXPECT_TRUE(eidx.substr(0, 6) == "189765");
        }
        // test unique global int32_t crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "UniqueGlobalAdd",
                                     FieldType::INT32, lgraph::IndexType::GlobalUniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32,
                          lgraph::IndexType::GlobalUniqueIndex);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(i), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(*txn, Value::ConstRef(i),
                                                                    Value::ConstRef(i));
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), i);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), 1);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }
            EdgeIndexIterator it_all = idx.GetUnmanagedIterator(*txn, Value::ConstRef(0),
                                                                Value::ConstRef(200));
            size_t vaild_keys = 0;
            while (it_all.IsValid()) {
                ++vaild_keys, it_all.Next();
            }
            UT_EXPECT_EQ(vaild_keys, 100);

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(i), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(*txn, Value::ConstRef(i),
                                                                    Value::ConstRef(i));
                UT_EXPECT_FALSE(it_del.IsValid());
            }

            EdgeIndexIterator it_none = idx.GetUnmanagedIterator(*txn, Value::ConstRef(0),
                                                                 Value::ConstRef(200));
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(all_keys, 0);
        }
        // test unique global string crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "UniqueGlobalAddString",
                                         FieldType::STRING, lgraph::IndexType::GlobalUniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::STRING,
                          lgraph::IndexType::GlobalUniqueIndex);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                std::string key("test_key" + std::to_string(i));
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(*txn, Value::ConstRef(key),
                                                                    Value::ConstRef(key));
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), 1);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }
            EdgeIndexIterator it_all = idx.GetUnmanagedIterator(*txn,
                                      Value::ConstRef("test_key0"), Value());
            size_t vaild_keys = 0;
            while (it_all.IsValid()) {
                ++vaild_keys;
                it_all.Next();
            }
            UT_EXPECT_EQ(vaild_keys, 100);

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                std::string key("test_key" + std::to_string(i));
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(*txn,
                                           Value::ConstRef(key), Value::ConstRef(key));
                UT_EXPECT_FALSE(it_del.IsValid());
            }

            EdgeIndexIterator it_none = idx.GetUnmanagedIterator(*txn,
                                        Value::ConstRef("test_key0"), Value());
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(all_keys, 0);
        }
        // test unique non-global int32 crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "UniqueNonGlobalAddInt32",
                                         FieldType::INT32, lgraph::IndexType::PairUniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::PairUniqueIndex);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(*txn,
                                           Value::ConstRef(1), Value::ConstRef(1), i , i + 1);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), 1);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), 1);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }
            EdgeIndexIterator it_range =
                idx.GetUnmanagedIterator(*txn, Value::ConstRef(1), Value::ConstRef(1));
            size_t range_count = 0;
            while (it_range.IsValid()) {
                UT_EXPECT_EQ(it_range.GetSrcVid(), range_count);
                UT_EXPECT_EQ(it_range.GetDstVid(), range_count + 1);
                UT_EXPECT_EQ(it_range.GetLabelId(), 1);
                UT_EXPECT_EQ(it_range.GetTemporalId(), range_count);
                UT_EXPECT_EQ(it_range.GetEid(), range_count);
                ++range_count;
                it_range.Next();
            }
            UT_EXPECT_EQ(range_count, 100);

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(1), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(*txn, Value::ConstRef(1),
                                                                    Value::ConstRef(1), i, i + 1);
                if (i != 99) {
                    UT_EXPECT_TRUE(it_del.IsValid());
                    UT_EXPECT_EQ(it_del.GetKey().AsType<int32_t>(), 1);
                    UT_EXPECT_NE(it_del.GetSrcVid(), i);
                    UT_EXPECT_NE(it_del.GetDstVid(), i + 1);
                    UT_EXPECT_EQ(it_del.GetLabelId(), 1);
                    UT_EXPECT_NE(it_del.GetTemporalId(), i);
                    UT_EXPECT_NE(it_del.GetEid(), i);
                } else {
                    UT_EXPECT_FALSE(it_del.IsValid());
                }
            }

            EdgeIndexIterator it_none =
                idx.GetUnmanagedIterator(*txn, Value::ConstRef(1), Value());
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(all_keys, 0);
        }
        //  test unique non-global string crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "UniqueNonGlobalAddString",
                                          FieldType::STRING, lgraph::IndexType::PairUniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::STRING,
                          lgraph::IndexType::PairUniqueIndex);
            std::string key("test_key");
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i , i + 1);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), 1);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }
            EdgeIndexIterator it_range = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(key), Value::ConstRef(key));
            size_t range_count = 0;
            while (it_range.IsValid()) {
                UT_EXPECT_EQ(it_range.GetSrcVid(), range_count);
                UT_EXPECT_EQ(it_range.GetDstVid(), range_count + 1);
                UT_EXPECT_EQ(it_range.GetLabelId(), 1);
                UT_EXPECT_EQ(it_range.GetTemporalId(), range_count);
                UT_EXPECT_EQ(it_range.GetEid(), range_count);
                ++range_count;
                it_range.Next();
            }
            UT_EXPECT_EQ(range_count, 100);

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, 1, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i, i + 1);
                if (i != 99) {
                    UT_EXPECT_TRUE(it_del.IsValid());
                    UT_EXPECT_EQ(it_del.GetKey().AsString(), key);
                    UT_EXPECT_NE(it_del.GetSrcVid(), i);
                    UT_EXPECT_NE(it_del.GetDstVid(), i + 1);
                    UT_EXPECT_EQ(it_del.GetLabelId(), 1);
                    UT_EXPECT_NE(it_del.GetTemporalId(), i);
                    UT_EXPECT_NE(it_del.GetEid(), i);
                } else {
                    UT_EXPECT_FALSE(it_del.IsValid());
                }
            }

            EdgeIndexIterator it_none = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(key), Value());
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(all_keys, 0);
        }

        // test non-unique global int32 crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "NonUniqueGlobalAddInt32",
                                         FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::NonuniqueIndex);

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(1), Value::ConstRef(1), i , i + 1, i, i, i);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), 1);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }
            EdgeIndexIterator it_range =
                idx.GetUnmanagedIterator(*txn, Value::ConstRef(1), Value::ConstRef(1));
            size_t range_count = 0;
            while (it_range.IsValid()) {
                UT_EXPECT_EQ(it_range.GetSrcVid(), range_count);
                UT_EXPECT_EQ(it_range.GetDstVid(), range_count + 1);
                UT_EXPECT_EQ(it_range.GetLabelId(), range_count);
                UT_EXPECT_EQ(it_range.GetTemporalId(), range_count);
                UT_EXPECT_EQ(it_range.GetEid(), range_count);
                ++range_count;
                it_range.Next();
            }
            UT_EXPECT_EQ(range_count, 100);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(1), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(1), Value::ConstRef(1), i, i + 1, i, i, i);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_del.IsValid()) {
                    if (it_del.GetSrcVid() == i) find_src = true;
                    if (it_del.GetDstVid() == i + 1) find_dst = true;
                    if (it_del.GetLabelId() == i) find_lid = true;
                    if (it_del.GetTemporalId() == i) find_tid = true;
                    if (it_del.GetEid() == i) find_eid = true;
                    it_del.Next();
                }
                UT_EXPECT_FALSE(find_src);
                UT_EXPECT_FALSE(find_dst);
                UT_EXPECT_FALSE(find_lid);
                UT_EXPECT_FALSE(find_tid);
                UT_EXPECT_FALSE(find_eid);
            }
            EdgeIndexIterator it_none = idx.GetUnmanagedIterator(*txn, Value::ConstRef(0), Value());
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(i), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(i), Value::ConstRef(i));
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), i);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), i);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(i), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(i), Value::ConstRef(i));
                UT_EXPECT_FALSE(it_del.IsValid());
            }
            EdgeIndexIterator it_del = idx.GetUnmanagedIterator(*txn, Value::ConstRef(0), Value());
            size_t del_keys = 0;
            while (it_del.IsValid()) {
                ++del_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(del_keys, 0);
        }

        // test non-unique global string crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "NonUniqueGlobalAddString",
                                         FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key");
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i , i + 1, i, i, i);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }
            EdgeIndexIterator it_range = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef("test_key"), Value::ConstRef("test_key"));
            size_t range_count = 0;
            while (it_range.IsValid()) {
                UT_EXPECT_EQ(it_range.GetSrcVid(), range_count);
                UT_EXPECT_EQ(it_range.GetDstVid(), range_count + 1);
                UT_EXPECT_EQ(it_range.GetLabelId(), range_count);
                UT_EXPECT_EQ(it_range.GetTemporalId(), range_count);
                UT_EXPECT_EQ(it_range.GetEid(), range_count);
                ++range_count;
                it_range.Next();
            }
            UT_EXPECT_EQ(range_count, 100);
            for (int32_t i = 0; i < 100; ++i) {
                std::string key("test_key");
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i, i + 1, i, i, i);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_del.IsValid()) {
                    if (it_del.GetSrcVid() == i) find_src = true;
                    if (it_del.GetDstVid() == i + 1) find_dst = true;
                    if (it_del.GetLabelId() == i) find_lid = true;
                    if (it_del.GetTemporalId() == i) find_tid = true;
                    if (it_del.GetEid() == i) find_eid = true;
                    it_del.Next();
                }
                UT_EXPECT_FALSE(find_src);
                UT_EXPECT_FALSE(find_dst);
                UT_EXPECT_FALSE(find_lid);
                UT_EXPECT_FALSE(find_tid);
                UT_EXPECT_FALSE(find_eid);
            }
            EdgeIndexIterator it_none = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef("test_key"), Value());
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(all_keys, 0);

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key" + std::to_string(i));
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key));
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), i);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key" + std::to_string(i));
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key));
                UT_EXPECT_FALSE(it_del.IsValid());
            }
        }

        // test non-unique non-global int32 crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "NonUniqueNONGlobalAddInt32",
                                          FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(1), Value::ConstRef(1), i , i + 1, i, i, i);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), 1);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(1), Value::ConstRef(1), i , i + 1);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), 1);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(1), Value::ConstRef(1));
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), 1);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }

            EdgeIndexIterator it_range = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(1), Value::ConstRef(1));
            size_t range_count = 0;
            while (it_range.IsValid()) {
                UT_EXPECT_EQ(it_range.GetSrcVid(), range_count);
                UT_EXPECT_EQ(it_range.GetDstVid(), range_count + 1);
                UT_EXPECT_EQ(it_range.GetLabelId(), range_count);
                UT_EXPECT_EQ(it_range.GetTemporalId(), range_count);
                UT_EXPECT_EQ(it_range.GetEid(), range_count);
                ++range_count;
                it_range.Next();
            }
            UT_EXPECT_EQ(range_count, 100);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(1), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(1), Value::ConstRef(1), i, i + 1, i, i, i);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_del.IsValid()) {
                    if (it_del.GetSrcVid() == i) find_src = true;
                    if (it_del.GetDstVid() == i + 1) find_dst = true;
                    if (it_del.GetLabelId() == i) find_lid = true;
                    if (it_del.GetTemporalId() == i) find_tid = true;
                    if (it_del.GetEid() == i) find_eid = true;
                    it_del.Next();
                }
                UT_EXPECT_FALSE(find_src);
                UT_EXPECT_FALSE(find_dst);
                UT_EXPECT_FALSE(find_lid);
                UT_EXPECT_FALSE(find_tid);
                UT_EXPECT_FALSE(find_eid);
            }
            EdgeIndexIterator it_none = idx.GetUnmanagedIterator(*txn, Value::ConstRef(0), Value());
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(all_keys, 0);

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(i), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(i), Value::ConstRef(i));
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsType<int32_t>(), i);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), i);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(i), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(i), Value::ConstRef(i));
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_del.IsValid()) {
                    if (it_del.GetSrcVid() == i) find_src = true;
                    if (it_del.GetDstVid() == i + 1) find_dst = true;
                    if (it_del.GetLabelId() == i) find_lid = true;
                    if (it_del.GetTemporalId() == i) find_tid = true;
                    if (it_del.GetEid() == i) find_eid = true;
                    it_del.Next();
                }
                UT_EXPECT_FALSE(find_src);
                UT_EXPECT_FALSE(find_dst);
                UT_EXPECT_FALSE(find_lid);
                UT_EXPECT_FALSE(find_tid);
                UT_EXPECT_FALSE(find_eid);
            }
            EdgeIndexIterator it_del_all = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(0), Value());
            size_t del_keys = 0;
            while (it_del_all.IsValid()) {
                ++del_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(del_keys, 0);
        }

        // test non-unique non-global string crd
        {
            auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "NonUniqueNONGlobalAddString",
                                          FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
            EdgeIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
            for (int32_t i = 0; i < 100; ++i) {
                std::string key("test_key");
                EdgeUid edgeUid(i, i + 1, i, i, i);
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i , i + 1, i, i, i);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key");
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i , i + 1);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }

            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key");
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key));
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_add.IsValid()) {
                    if (it_add.GetSrcVid() == i) find_src = true;
                    if (it_add.GetDstVid() == i + 1) find_dst = true;
                    if (it_add.GetLabelId() == i) find_lid = true;
                    if (it_add.GetTemporalId() == i) find_tid = true;
                    if (it_add.GetEid() == i) find_eid = true;
                    it_add.Next();
                }
                UT_EXPECT_TRUE(find_src);
                UT_EXPECT_TRUE(find_dst);
                UT_EXPECT_TRUE(find_lid);
                UT_EXPECT_TRUE(find_tid);
                UT_EXPECT_TRUE(find_eid);
            }

            EdgeIndexIterator it_range = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef("test_key"), Value::ConstRef("test_key"));
            size_t range_count = 0;
            while (it_range.IsValid()) {
                UT_EXPECT_EQ(it_range.GetSrcVid(), range_count);
                UT_EXPECT_EQ(it_range.GetDstVid(), range_count + 1);
                UT_EXPECT_EQ(it_range.GetLabelId(), range_count);
                UT_EXPECT_EQ(it_range.GetTemporalId(), range_count);
                UT_EXPECT_EQ(it_range.GetEid(), range_count);
                ++range_count;
                it_range.Next();
            }
            UT_EXPECT_EQ(range_count, 100);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key");
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i, i + 1, i, i, i);
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_del.IsValid()) {
                    if (it_del.GetSrcVid() == i) find_src = true;
                    if (it_del.GetDstVid() == i + 1) find_dst = true;
                    if (it_del.GetLabelId() == i) find_lid = true;
                    if (it_del.GetTemporalId() == i) find_tid = true;
                    if (it_del.GetEid() == i) find_eid = true;
                    it_del.Next();
                }
                UT_EXPECT_FALSE(find_src);
                UT_EXPECT_FALSE(find_dst);
                UT_EXPECT_FALSE(find_lid);
                UT_EXPECT_FALSE(find_tid);
                UT_EXPECT_FALSE(find_eid);
            }
            EdgeIndexIterator it_none = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef("test_key"), Value());
            size_t all_keys = 0;
            while (it_none.IsValid()) {
                ++all_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(all_keys, 0);
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key" + std::to_string(i));
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key), i, i + 1);
                UT_EXPECT_TRUE(it_add.IsValid());
                UT_EXPECT_EQ(it_add.GetKey().AsString(), key);
                UT_EXPECT_EQ(it_add.GetSrcVid(), i);
                UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
                UT_EXPECT_EQ(it_add.GetLabelId(), i);
                UT_EXPECT_EQ(it_add.GetTemporalId(), i);
                UT_EXPECT_EQ(it_add.GetEid(), i);
            }
            for (int32_t i = 0; i < 100; ++i) {
                EdgeUid edgeUid(i, i + 1, i, i, i);
                std::string key("test_key" + std::to_string(i));
                UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
                EdgeIndexIterator it_del = idx.GetUnmanagedIterator(
                    *txn, Value::ConstRef(key), Value::ConstRef(key));
                bool find_src = false, find_dst = false, find_lid = false,
                     find_tid = false, find_eid = false;
                while (!find_src && it_del.IsValid()) {
                    if (it_del.GetSrcVid() == i) find_src = true;
                    if (it_del.GetDstVid() == i + 1) find_dst = true;
                    if (it_del.GetLabelId() == i) find_lid = true;
                    if (it_del.GetTemporalId() == i) find_tid = true;
                    if (it_del.GetEid() == i) find_eid = true;
                    it_del.Next();
                }
                UT_EXPECT_FALSE(find_src);
                UT_EXPECT_FALSE(find_dst);
                UT_EXPECT_FALSE(find_lid);
                UT_EXPECT_FALSE(find_tid);
                UT_EXPECT_FALSE(find_eid);
            }
            EdgeIndexIterator it_del_all = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef("test_key"), Value());
            size_t del_keys = 0;
            while (it_del_all.IsValid()) {
                ++del_keys;
                it_none.Next();
            }
            UT_EXPECT_EQ(del_keys, 0);
        }
        txn->Commit();
    }
    return 0;
}  // end of TestEdgeIndexCRUD

int CURDEdgeWithTooLongKey() {
    auto bytes_count = [](int i) {
        if (i == 0) return (size_t)1;
        size_t count = 0;
        while (i) {
            i /= 10;
            ++count;
        }
        return count;
    };

    // test EdgeIndex
    auto store = std::make_unique<LMDBKvStore>("./testdb", (size_t)1 << 30, true);
    // Start test, see if we already has a db
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
    txn->Commit();
    txn = store->CreateWriteTxn();
    // NonuniqueIndex edge index
    {
        auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "LongKeyWithNonUnique", FieldType::STRING,
                                            lgraph::IndexType::NonuniqueIndex);
        EdgeIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
        for (int32_t i = 0; i < 100; ++i) {
            std::string key(lgraph::_detail::MAX_KEY_SIZE + 100, 'a');
            EdgeUid edgeUid(i, i + 1, i, i, i);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
            EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(key), Value::ConstRef(key), i, i + 1, i, i, i);
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(),
                         std::string(lgraph::_detail::MAX_KEY_SIZE - 24, 'a'));
            bool find_src = false, find_dst = false, find_lid = false, find_tid = false,
                 find_eid = false;
            while (!find_src && it_add.IsValid()) {
                if (it_add.GetSrcVid() == i) find_src = true;
                if (it_add.GetDstVid() == i + 1) find_dst = true;
                if (it_add.GetLabelId() == i) find_lid = true;
                if (it_add.GetTemporalId() == i) find_tid = true;
                if (it_add.GetEid() == i) find_eid = true;
                it_add.Next();
            }
            UT_EXPECT_TRUE(find_src);
            UT_EXPECT_TRUE(find_dst);
            UT_EXPECT_TRUE(find_lid);
            UT_EXPECT_TRUE(find_tid);
            UT_EXPECT_TRUE(find_eid);
        }

        for (int32_t i = 0; i < 100; ++i) {
            std::string ok(lgraph::_detail::MAX_KEY_SIZE + 100, 'a');
            std::string nk(lgraph::_detail::MAX_KEY_SIZE + 100, 'b');
            EdgeUid edgeUid(i, i + 1, i, i, i);
            UT_EXPECT_TRUE(idx.Update(*txn, Value::ConstRef(ok), Value::ConstRef(nk), edgeUid));
            EdgeIndexIterator it_add = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(nk), Value::ConstRef(nk), i, i + 1, i, i, i);
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(),
                         std::string(lgraph::_detail::MAX_KEY_SIZE - 24, 'b'));
            bool find_src = false, find_dst = false, find_lid = false, find_tid = false,
                 find_eid = false;
            while (!find_src && it_add.IsValid()) {
                if (it_add.GetSrcVid() == i) find_src = true;
                if (it_add.GetDstVid() == i + 1) find_dst = true;
                if (it_add.GetLabelId() == i) find_lid = true;
                if (it_add.GetTemporalId() == i) find_tid = true;
                if (it_add.GetEid() == i) find_eid = true;
                it_add.Next();
            }
            UT_EXPECT_TRUE(find_src);
            UT_EXPECT_TRUE(find_dst);
            UT_EXPECT_TRUE(find_lid);
            UT_EXPECT_TRUE(find_tid);
            UT_EXPECT_TRUE(find_eid);
        }

        for (int32_t i = 0; i < 100; ++i) {
            EdgeUid edgeUid(i, i + 1, i, i, i);
            std::string key(lgraph::_detail::MAX_KEY_SIZE + 100, 'b');
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
            EdgeIndexIterator it_del =
                idx.GetUnmanagedIterator(*txn, Value::ConstRef(key), Value::ConstRef(key));
            bool find_src = false, find_dst = false, find_lid = false, find_tid = false,
                 find_eid = false;
            while (!find_src && it_del.IsValid()) {
                if (it_del.GetSrcVid() == i) find_src = true;
                if (it_del.GetDstVid() == i + 1) find_dst = true;
                if (it_del.GetLabelId() == i) find_lid = true;
                if (it_del.GetTemporalId() == i) find_tid = true;
                if (it_del.GetEid() == i) find_eid = true;
                it_del.Next();
            }
            UT_EXPECT_FALSE(find_src);
            UT_EXPECT_FALSE(find_dst);
            UT_EXPECT_FALSE(find_lid);
            UT_EXPECT_FALSE(find_tid);
            UT_EXPECT_FALSE(find_eid);
        }
    }

    // GlobalUniqueIndex edge index
    {
        auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "LongKeyWithGlobal",
                                  FieldType::STRING, lgraph::IndexType::GlobalUniqueIndex);
        EdgeIndex idx(std::move(idx_tab), FieldType::STRING,
                      lgraph::IndexType::GlobalUniqueIndex);
        std::string str(lgraph::_detail::MAX_KEY_SIZE + 100, 'a');
        std::string tmp(lgraph::_detail::MAX_KEY_SIZE + 100, 'b');
        for (int32_t i = 0; i < 100; ++i) {
            EdgeUid edgeUid(i, i + 1, 1, i, i);
            std::string key = std::to_string(i).append(str);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
            EdgeIndexIterator it_add = idx.GetUnmanagedIterator(*txn, Value::ConstRef(key),
                                                                Value::ConstRef(key));
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(), std::to_string(i) +
                         std::string(lgraph::_detail::MAX_KEY_SIZE - bytes_count(i), 'a'));
            UT_EXPECT_EQ(it_add.GetSrcVid(), i);
            UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
            UT_EXPECT_EQ(it_add.GetLabelId(), 1);
            UT_EXPECT_EQ(it_add.GetTemporalId(), i);
            UT_EXPECT_EQ(it_add.GetEid(), i);
        }
        EdgeIndexIterator it_all = idx.GetUnmanagedIterator(*txn, Value(), Value());
        size_t vaild_keys = 0;
        while (it_all.IsValid()) {
            ++vaild_keys;
            it_all.Next();
        }
        UT_EXPECT_EQ(vaild_keys, 100);

        for (int32_t i = 0; i < 100; ++i) {
            EdgeUid edgeUid(i, i + 1, 1, i, i);
            std::string ok = std::to_string(i).append(str);
            std::string nk = std::to_string(i).append(tmp);
            UT_EXPECT_TRUE(idx.Update(*txn, Value::ConstRef(ok), Value::ConstRef(nk), edgeUid));
            EdgeIndexIterator it_add = idx.GetUnmanagedIterator(*txn,
                                            Value::ConstRef(nk), Value::ConstRef(nk));
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(), std::to_string(i) +
                      std::string(lgraph::_detail::MAX_KEY_SIZE - bytes_count(i), 'b'));
            UT_EXPECT_EQ(it_add.GetSrcVid(), i);
            UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
            UT_EXPECT_EQ(it_add.GetLabelId(), 1);
            UT_EXPECT_EQ(it_add.GetTemporalId(), i);
            UT_EXPECT_EQ(it_add.GetEid(), i);
        }

        for (int32_t i = 0; i < 100; ++i) {
            EdgeUid edgeUid(i, i + 1, 1, i, i);
            std::string key = std::to_string(i).append(tmp);
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
            EdgeIndexIterator it_del = idx.GetUnmanagedIterator(*txn,
                                        Value::ConstRef(key), Value::ConstRef(key));
            UT_EXPECT_FALSE(it_del.IsValid());
        }

        EdgeIndexIterator it_none = idx.GetUnmanagedIterator(*txn, Value(), Value());
        size_t all_keys = 0;
        while (it_none.IsValid()) {
            ++all_keys;
            it_none.Next();
        }
        UT_EXPECT_EQ(all_keys, 0);
    }

    // GlobalUniqueIndex edge index
    {
        auto idx_tab = EdgeIndex::OpenTable(*txn, *store, "LongKeyWithPair",
                                            FieldType::STRING, lgraph::IndexType::PairUniqueIndex);
        EdgeIndex idx(std::move(idx_tab), FieldType::STRING,
                      lgraph::IndexType::PairUniqueIndex);
        std::string str(lgraph::_detail::MAX_KEY_SIZE + 100, 'a');
        std::string tmp(lgraph::_detail::MAX_KEY_SIZE + 100, 'b');
        for (int32_t i = 0; i < 100; ++i) {
            EdgeUid edgeUid(i, i + 1, 1, i, i);
            std::string key = std::to_string(i).append(str);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), edgeUid));
            EdgeIndexIterator it_add = idx.GetUnmanagedIterator(*txn, Value::ConstRef(key),
                                                                Value::ConstRef(key));
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(), std::to_string(i) +
                        std::string(lgraph::_detail::MAX_KEY_SIZE - bytes_count(i) - 10, 'a'));
            UT_EXPECT_EQ(it_add.GetSrcVid(), i);
            UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
            UT_EXPECT_EQ(it_add.GetLabelId(), 1);
            UT_EXPECT_EQ(it_add.GetTemporalId(), i);
            UT_EXPECT_EQ(it_add.GetEid(), i);
        }
        EdgeIndexIterator it_all = idx.GetUnmanagedIterator(*txn, Value(), Value());
        size_t vaild_keys = 0;
        while (it_all.IsValid()) {
            ++vaild_keys;
            it_all.Next();
        }
        UT_EXPECT_EQ(vaild_keys, 100);

        for (int32_t i = 0; i < 100; ++i) {
            EdgeUid edgeUid(i, i + 1, 1, i, i);
            std::string ok = std::to_string(i).append(str);
            std::string nk = std::to_string(i).append(tmp);
            UT_EXPECT_TRUE(idx.Update(*txn, Value::ConstRef(ok), Value::ConstRef(nk), edgeUid));
            EdgeIndexIterator it_add = idx.GetUnmanagedIterator(*txn,
                                            Value::ConstRef(nk), Value::ConstRef(nk));
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(), std::to_string(i) +
                         std::string(lgraph::_detail::MAX_KEY_SIZE - bytes_count(i) - 10, 'b'));
            UT_EXPECT_EQ(it_add.GetSrcVid(), i);
            UT_EXPECT_EQ(it_add.GetDstVid(), i + 1);
            UT_EXPECT_EQ(it_add.GetLabelId(), 1);
            UT_EXPECT_EQ(it_add.GetTemporalId(), i);
            UT_EXPECT_EQ(it_add.GetEid(), i);
        }

        for (int32_t i = 0; i < 100; ++i) {
            EdgeUid edgeUid(i, i + 1, 1, i, i);
            std::string key = std::to_string(i).append(tmp);
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), edgeUid));
            EdgeIndexIterator it_del = idx.GetUnmanagedIterator(*txn,
                                       Value::ConstRef(key), Value::ConstRef(key));
            UT_EXPECT_FALSE(it_del.IsValid());
        }

        EdgeIndexIterator it_none = idx.GetUnmanagedIterator(*txn, Value(), Value());
        size_t all_keys = 0;
        while (it_none.IsValid()) {
            ++all_keys;
            it_none.Next();
        }
        UT_EXPECT_EQ(all_keys, 0);
    }
    return 0;
}

TEST_F(TestEdgeIndex, TestEdgeIndexCRUD) {
    TestEdgeIndexValue();
    TestEdgeIndexCRUD();
    CURDEdgeWithTooLongKey();
}
