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
#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "core/kv_store.h"
#include "core/mock_kv.h"
#include "core/vertex_index.h"
#include "core/lightning_graph.h"
#include "./ut_utils.h"
using namespace lgraph;
using namespace fma_common;

class TestVertexIndex : public TuGraphTest {};

int TestVertexIndexImpl() {
    auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << 30, true);
    // Start test, see if we already has a db
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
    txn->Commit();

    // now create the index table
    txn = store->CreateWriteTxn();
    int cnt = 0;
    // test integer keys
    {
        auto fetch_n_bytes = [](const char* p, size_t nbytes) {
            int64_t val = 0;
            memcpy(&val, p, nbytes);
            return val;
        };
        auto int_non_unique_to_string = [&](const char* p, size_t s) {
            int32_t key = fetch_n_bytes(p, s - 5);
            std::string ret = std::to_string(key);
            return ret;
        };
        auto idx_tab =
            VertexIndex::OpenTable(*txn, *store, "int32_index_test", FieldType::INT32,
                                   lgraph::IndexType::NonuniqueIndex);
        VertexIndex idx(std::move(idx_tab), FieldType::INT32, lgraph::IndexType::NonuniqueIndex);
        // add many vids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(1), i));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(2), i));
        }
        // add some vids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(2), i));
        }
        // add some vids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(!idx.Add(*txn, Value::ConstRef(2), i));
        }
        // delete some vids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(2), i));
        }
        // test updates
        UT_EXPECT_TRUE(idx.Update(*txn, Value::ConstRef(2), Value::ConstRef(1), 0));
        idx.Dump(*txn.get(), int_non_unique_to_string);
        // output indexed vids
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(1), Value::ConstRef(1));
            UT_EXPECT_EQ(it.GetVid(), 0);
            cnt = 1;
            it.Next();
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                it.Next();
                cnt += 2;
                if (cnt == 80) cnt += 10;
            }
            cnt = 2;
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(2), Value::ConstRef(2));
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                cnt += 2;
                it.Next();
                if (cnt == 80) cnt += 10;
            }
            {
                auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(1), Value::ConstRef(2));
                UT_EXPECT_EQ(it.GetVid(), 0);
                cnt = 1;
                it.Next();
                while (it.IsValid()) {
                    UT_EXPECT_EQ(it.GetVid(), cnt);
                    it.Next();
                    cnt += 2;
                    if (cnt == 80) cnt += 10;
                    if (cnt == 501) cnt = 2;
                }
                cnt = 1;
            }
            {
                auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef(2), Value::ConstRef(1));
                while (it.IsValid()) {
                    UT_EXPECT_EQ(it.GetVid(), cnt);
                    it.Next();
                    cnt += 2;
                }
                cnt = 1;
            }
        }
    }
    // test string keys
    {
        auto idx_tab =
            VertexIndex::OpenTable(*txn, *store, "string_index_test", FieldType::STRING,
                                   lgraph::IndexType::NonuniqueIndex);
        VertexIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
        // add many vids; should split after some point
        for (int64_t i = 1; i < 500; i += 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("a"), i));
        }
        for (int64_t i = 100; i < 600; i += 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("b"), i));
        }
        // add some vids
        for (int64_t i = 98; i >= 0; i -= 2) {
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("b"), i));
        }
        // add some vids which already exist
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(!idx.Add(*txn, Value::ConstRef("b"), i));
        }
        // delete some vids
        for (int64_t i = 88; i >= 80; i -= 2) {
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef("b"), i));
        }
        // output indexed vids
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("a"), Value::ConstRef("a"));
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                it.Next();
                cnt += 2;
            }
            cnt = 0;
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("b"), Value::ConstRef("b"));
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                it.Next();
                cnt += 2;
                if (cnt == 80) cnt += 10;
            }
            cnt = 1;
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("a"), Value::ConstRef("b"));
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                it.Next();
                cnt += 2;
                if (cnt == 80) cnt += 10;
                if (cnt == 501) cnt = 0;
            }
            cnt = 0;
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("b"), Value::ConstRef("a"));
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                it.Next();
                cnt += 2;
            }
            cnt = 1;
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("a"), Value::ConstRef("bb"));
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                it.Next();
                cnt += 2;
                if (cnt == 80) cnt += 10;
                if (cnt == 501) cnt = 0;
            }
            cnt = 0;
        }
        {
            auto it = idx.GetUnmanagedIterator(*txn, Value::ConstRef("aa"), Value::ConstRef("bb"));
            while (it.IsValid()) {
                UT_EXPECT_EQ(it.GetVid(), cnt);
                it.Next();
                cnt += 2;
                if (cnt == 80) cnt += 10;
            }
        }
    }
    // test index scan
    {
        // index with non-unique keys
        {
            auto idx_tab = VertexIndex::OpenTable(*txn, *store, "string_index_non_unqiue",
                                           FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
            VertexIndex idx(std::move(idx_tab), FieldType::STRING,
                            lgraph::IndexType::NonuniqueIndex);
            // add many vids; should split after some point
            for (int64_t i = 0; i < 500; i += 1) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("1"), i));
            }
            for (int64_t i = 500; i < 600; i += 1) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef("2"), i));
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
                VertexIndex::OpenTable(*txn, *store, "int32_index_unqiue", FieldType::INT32,
                                       lgraph::IndexType::NonuniqueIndex);
            VertexIndex idx(std::move(idx_tab), FieldType::INT32,
                            lgraph::IndexType::NonuniqueIndex);
            // add many vids; should split after some point
            for (int32_t i = 0; i < 100; i += 1) {
                UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(i), i));
            }
            size_t n = 0;
            for (auto it = idx.GetUnmanagedIterator(*txn, Value(), Value());
                 it.IsValid(); it.Next())
                n++;
            UT_EXPECT_EQ(n, 100);
        }
    }

    txn->Commit();

    return 0;
}

int CURDVertexWithTooLongKey() {
    // test EdgeIndex
    auto store = std::make_unique<LMDBKvStore>("./testdb", (size_t)1 << 30, true);
    // Start test, see if we already has a db
    auto txn = store->CreateWriteTxn();
    store->DropAll(*txn);
    txn->Commit();
    txn = store->CreateWriteTxn();
    // NonuniqueIndex vertex index
    {
        auto idx_tab = VertexIndex::OpenTable(*txn, *store, "LongKeyWithNonUnique",
                                              FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
        VertexIndex idx(std::move(idx_tab), FieldType::STRING, lgraph::IndexType::NonuniqueIndex);
        for (int32_t i = 0; i < 100; ++i) {
            std::string key(lgraph::_detail::MAX_KEY_SIZE + 100, 'a');
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), i));
            VertexIndexIterator it_add = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(key), Value::ConstRef(key), i);
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(),
                         std::string(lgraph::_detail::MAX_KEY_SIZE - 5, 'a'));
            bool find_vid = false;
            while (!find_vid && it_add.IsValid()) {
                if (it_add.GetVid() == i) find_vid = true;
                it_add.Next();
            }
            UT_EXPECT_TRUE(find_vid);
        }

        for (int32_t i = 0; i < 100; ++i) {
            std::string ok(lgraph::_detail::MAX_KEY_SIZE + 100, 'a');
            std::string nk(lgraph::_detail::MAX_KEY_SIZE + 100, 'b');
            UT_EXPECT_TRUE(idx.Update(*txn, Value::ConstRef(ok), Value::ConstRef(nk), i));
            VertexIndexIterator it_add = idx.GetUnmanagedIterator(
                *txn, Value::ConstRef(nk), Value::ConstRef(nk), i);
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(),
                         std::string(lgraph::_detail::MAX_KEY_SIZE - 5, 'b'));
            bool find_vid = false;
            while (!find_vid && it_add.IsValid()) {
                if (it_add.GetVid() == i) find_vid = true;
                it_add.Next();
            }
            UT_EXPECT_TRUE(find_vid);
        }

        for (int32_t i = 0; i < 100; ++i) {
            std::string key(lgraph::_detail::MAX_KEY_SIZE + 100, 'b');
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), i));
            VertexIndexIterator it_del =
                idx.GetUnmanagedIterator(*txn, Value::ConstRef(key), Value::ConstRef(key));
            bool find_vid = false;
            while (!find_vid && it_del.IsValid()) {
                if (it_del.GetVid() == i) find_vid = true;
                it_del.Next();
            }
            UT_EXPECT_FALSE(find_vid);
        }
    }

    // GlobalUniqueIndex vertex index
    {
    auto bytes_count = [](int i) {
        if (i == 0) return (size_t)1;
        size_t count = 0;
        while (i) {
            i /= 10;
            ++count;
        }
        return count;
    };
        auto idx_tab = VertexIndex::OpenTable(*txn, *store, "LongKeyWithGlobal",
                                 FieldType::STRING, lgraph::IndexType::GlobalUniqueIndex);
        VertexIndex idx(std::move(idx_tab), FieldType::STRING,
                      lgraph::IndexType::GlobalUniqueIndex);
        std::string str(lgraph::_detail::MAX_KEY_SIZE + 100, 'a');
        std::string tmp(lgraph::_detail::MAX_KEY_SIZE + 100, 'b');
        for (int32_t i = 0; i < 100; ++i) {
            std::string key = std::to_string(i).append(str);
            UT_EXPECT_TRUE(idx.Add(*txn, Value::ConstRef(key), i));
            VertexIndexIterator it_add = idx.GetUnmanagedIterator(*txn, Value::ConstRef(key),
                                                                Value::ConstRef(key));
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(), std::to_string(i) +
                         std::string(lgraph::_detail::MAX_KEY_SIZE - bytes_count(i), 'a'));
            UT_EXPECT_EQ(it_add.GetVid(), i);
        }
        VertexIndexIterator it_all = idx.GetUnmanagedIterator(*txn, Value(), Value());
        size_t vaild_keys = 0;
        while (it_all.IsValid()) {
            ++vaild_keys;
            it_all.Next();
        }
        UT_EXPECT_EQ(vaild_keys, 100);

        for (int32_t i = 0; i < 100; ++i) {
            std::string ok = std::to_string(i).append(str);
            std::string nk = std::to_string(i).append(tmp);
            UT_EXPECT_TRUE(idx.Update(*txn, Value::ConstRef(ok), Value::ConstRef(nk), i));
            VertexIndexIterator it_add = idx.GetUnmanagedIterator(*txn,
                                             Value::ConstRef(nk), Value::ConstRef(nk));
            UT_EXPECT_TRUE(it_add.IsValid());
            UT_EXPECT_EQ(it_add.GetKey().AsString(), std::to_string(i) +
                                  std::string(lgraph::_detail::MAX_KEY_SIZE - bytes_count(i), 'b'));
            UT_EXPECT_EQ(it_add.GetVid(), i);
        }
        for (int32_t i = 0; i < 100; ++i) {
            std::string key = std::to_string(i).append(tmp);
            UT_EXPECT_TRUE(idx.Delete(*txn, Value::ConstRef(key), i));
            VertexIndexIterator it_del = idx.GetUnmanagedIterator(*txn,
                                                   Value::ConstRef(key), Value::ConstRef(key));
            UT_EXPECT_FALSE(it_del.IsValid());
        }

        VertexIndexIterator it_none = idx.GetUnmanagedIterator(*txn, Value(), Value());
        size_t all_keys = 0;
        while (it_none.IsValid()) {
            ++all_keys;
            it_none.Next();
        }
        UT_EXPECT_EQ(all_keys, 0);
    }
    return 0;
}
TEST_F(TestVertexIndex, VertexIndex) {
    TestVertexIndexImpl();
    CURDVertexWithTooLongKey();
}

TEST_F(TestVertexIndex, addIndexDetach) {
    AutoCleanDir cleaner("./testdb");
    DBConfig config;
    config.dir = "./testdb";
    LightningGraph db(config);
    db.DropAllData();
    std::vector<FieldSpec> v_fds = {{"id", FieldType::INT32, false},
                                    {"int32", FieldType::INT32, false},
                                    {"string", FieldType::STRING, false},
                                    {"float", FieldType::FLOAT, true}};

    std::vector<FieldSpec> e_fds = {{"int32", FieldType::INT32, false},
                                    {"string", FieldType::STRING, false},
                                    {"float", FieldType::FLOAT, true}};

    VertexOptions vo("id");
    vo.detach_property = true;
    UT_EXPECT_TRUE(db.AddLabel("v1", v_fds, true, vo));
    EdgeOptions eo;
    eo.detach_property = true;
    UT_EXPECT_TRUE(db.AddLabel("e1", e_fds, false, EdgeOptions()));

    Transaction txn = db.CreateWriteTxn();
    std::vector<std::string> v1_properties = {"id", "int32", "string", "float"};
    for (int32_t i = 0; i < 100000; i++) {
        auto str_fd = std::to_string(i);
        str_fd = std::string(5 - str_fd.size(), '0') + str_fd;
        txn.AddVertex(
            std::string("v1"), v1_properties,
            std::vector<FieldData>{FieldData(int32_t(i)), FieldData(int32_t(i)),
                                   FieldData(str_fd), FieldData(float(i))});
    }
    std::vector<std::string> e1_properties = {"int32", "string", "float"};
    for (int32_t i = 0; i < 100000; i++) {
        auto str_fd = std::to_string(i);
        str_fd = std::string(5 - str_fd.size(), '0') + str_fd;
        txn.AddEdge(i, (i+1)%100000, std::string("e1"), e1_properties,
                    std::vector<FieldData>{FieldData(int32_t(i)),
                                           FieldData(str_fd), FieldData(float(i))});
    }
    txn.Commit();
    UT_EXPECT_TRUE(db.BlockingAddIndex("v1", "int32", lgraph::IndexType::NonuniqueIndex, true));
    UT_EXPECT_TRUE(db.BlockingAddIndex("v1", "string", lgraph::IndexType::NonuniqueIndex, true));
    UT_EXPECT_TRUE(db.BlockingAddIndex("v1", "float", lgraph::IndexType::NonuniqueIndex, true));

    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "int32", lgraph::IndexType::NonuniqueIndex, false));
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "string", lgraph::IndexType::NonuniqueIndex, false));
    UT_EXPECT_TRUE(db.BlockingAddIndex("e1", "float", lgraph::IndexType::NonuniqueIndex, false));

    txn = db.CreateReadTxn();
    // vertex
    {
        int32_t i = 0;
        for (auto iter = txn.GetVertexIndexIterator("v1", "int32"); iter.IsValid(); iter.Next()) {
            UT_EXPECT_TRUE(iter.GetKeyData() == FieldData::Int32(i));
            UT_EXPECT_TRUE(iter.GetVid() == i);
            i++;
        }
    }
    {
        int32_t i = 0;
        for (auto iter = txn.GetVertexIndexIterator("v1", "string"); iter.IsValid(); iter.Next()) {
            auto str_fd = std::to_string(i);
            str_fd = std::string(5 - str_fd.size(), '0') + str_fd;
            UT_EXPECT_TRUE(iter.GetKeyData() == FieldData::String(str_fd));
            UT_EXPECT_TRUE(iter.GetVid() == i);
            i++;
        }
    }
    {
        int32_t i = 0;
        for (auto iter = txn.GetVertexIndexIterator("v1", "float"); iter.IsValid(); iter.Next()) {
            UT_EXPECT_TRUE(iter.GetKeyData() == FieldData::Float(i));
            UT_EXPECT_TRUE(iter.GetVid() == i);
            i++;
        }
    }

    // edge
    {
        int32_t i = 0;
        for (auto iter = txn.GetEdgeIndexIterator("e1", "int32"); iter.IsValid(); iter.Next()) {
            UT_EXPECT_EQ(iter.GetKeyData(), FieldData::Int32(i));
            UT_EXPECT_EQ(iter.GetSrcVid(), i);
            UT_EXPECT_EQ(iter.GetDstVid(), (i+1)%100000);
            i++;
        }
    }
    {
        int32_t i = 0;
        for (auto iter = txn.GetEdgeIndexIterator("e1", "string"); iter.IsValid(); iter.Next()) {
            auto str_fd = std::to_string(i);
            str_fd = std::string(5 - str_fd.size(), '0') + str_fd;
            UT_EXPECT_EQ(iter.GetKeyData(), FieldData::String(str_fd));
            UT_EXPECT_EQ(iter.GetSrcVid(), i);
            UT_EXPECT_EQ(iter.GetDstVid(), (i+1)%100000);
            i++;
        }
    }
    {
        int32_t i = 0;
        for (auto iter = txn.GetEdgeIndexIterator("e1", "float"); iter.IsValid(); iter.Next()) {
            UT_EXPECT_EQ(iter.GetKeyData(), FieldData::Float(i));
            UT_EXPECT_EQ(iter.GetSrcVid(), i);
            UT_EXPECT_EQ(iter.GetDstVid(), (i+1)%100000);
            i++;
        }
    }
    txn.Abort();
}
