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
            int64_t local_eid = 1;
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
            int64_t local_eid = 1;
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

TEST_F(TestEdgeIndex, EdgeIndex) { TestEdgeIndexImpl(); }
