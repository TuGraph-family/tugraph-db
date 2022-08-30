/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "gtest/gtest.h"
#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "core/kv_store.h"
#include "core/mock_kv.h"
#include "core/vertex_index.h"
#include "./ut_utils.h"
using namespace lgraph;
using namespace fma_common;

class TestEdgeIndex : public TuGraphTest {};

int TestVertexIndexImpl() {
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
            VertexIndex::OpenTable(txn, store, "int32_index_test", FieldType::INT32, false);
        VertexIndex idx(idx_tab, FieldType::INT32, false);
        // add many vids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(1), i));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(2), i));
        }
        // add some vids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(2), i));
        }
        // add some vids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(!idx.Add(txn, Value::ConstRef(2), i));
        }
        // delete some vids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(idx.Delete(txn, Value::ConstRef(2), i));
        }
        // test updates
        UT_EXPECT_TRUE(idx.Update(txn, Value::ConstRef(2), Value::ConstRef(1), 0));
        // output indexed vids
        {
            UT_LOG() << "[1, 1]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(1), Value::ConstRef(1));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[2, 2]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(2), Value::ConstRef(2));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[1, 2]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(1), Value::ConstRef(2));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[2, 1]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef(2), Value::ConstRef(1));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
    }
    // test string keys
    {
        KvTable idx_tab =
            VertexIndex::OpenTable(txn, store, "string_index_test", FieldType::STRING, false);
        VertexIndex idx(idx_tab, FieldType::STRING, false);
        // add many vids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("a"), i));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("b"), i));
        }
        // add some vids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("b"), i));
        }
        // add some vids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(!idx.Add(txn, Value::ConstRef("b"), i));
        }
        // delete some vids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(idx.Delete(txn, Value::ConstRef("b"), i));
        }
        // output indexed vids
        {
            UT_LOG() << "[a, a]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("a"), Value::ConstRef("a"));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[b, b]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("b"), Value::ConstRef("b"));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[a, b]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("a"), Value::ConstRef("b"));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[b, a]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("b"), Value::ConstRef("a"));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[a, bb]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("a"), Value::ConstRef("bb"));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
        {
            UT_LOG() << "[aa, bb]\n";
            auto it = idx.GetUnmanagedIterator(txn, Value::ConstRef("aa"), Value::ConstRef("bb"));
            while (it.IsValid()) {
                UT_LOG() << it.GetVid();
                it.Next();
            }
            UT_LOG() << "\n";
            UT_LOG() << "\n";
        }
    }
    // test index scan
    {
        // index with non-unique keys
        {
            KvTable idx_tab = VertexIndex::OpenTable(txn, store, "string_index_non_unqiue",
                                                     FieldType::STRING, false);
            VertexIndex idx(idx_tab, FieldType::STRING, false);
            // add many vids; should split after some point
            for (int64_t i = 0; i < 500; i += 1) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("1"), i));
            }
            for (int64_t i = 500; i < 600; i += 1) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef("2"), i));
            }
            size_t n = 0;
            for (auto it = idx.GetUnmanagedIterator(txn, Value(), Value()); it.IsValid(); it.Next())
                n++;
            UT_EXPECT_EQ(n, 600);
        }
        // index with non-unique keys
        {
            KvTable idx_tab =
                VertexIndex::OpenTable(txn, store, "int32_index_unqiue", FieldType::INT32, false);
            VertexIndex idx(idx_tab, FieldType::INT32, false);
            // add many vids; should split after some point
            for (int32_t i = 0; i < 100; i += 1) {
                UT_EXPECT_TRUE(idx.Add(txn, Value::ConstRef(i), i));
            }
            size_t n = 0;
            for (auto it = idx.GetUnmanagedIterator(txn, Value(), Value()); it.IsValid(); it.Next())
                n++;
            UT_EXPECT_EQ(n, 100);
        }
    }

    txn.Commit();

    return 0;
}

TEST_F(TestEdgeIndex, VertexIndex) {
    TestVertexIndexImpl();
}
