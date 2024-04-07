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

#include <future>

#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "fma-common/string_util.h"
#include "fma-common/fma_stream.h"
#include "gtest/gtest.h"
#include "core/kv_store.h"

#include "./test_tools.h"
#include "./ut_utils.h"
using namespace lgraph;
using namespace fma_common;

class TestKvStore : public TuGraphTest {};

void Dump(KvTable& table, KvTransaction& txn,
                       const std::function<void(const Value& key, void* context)>& begin_key,
                       const std::function<void(const Value& value, void* context)>& dump_value,
                       const std::function<void(const Value& key, void* context)>& finish_key,
                       void* context) {
    auto it = table.GetIterator(txn);
    it->GotoFirstKey();
    while (it->IsValid()) {
        Value key = it->GetKey();
        begin_key(key, context);
        dump_value(it->GetValue(), context);
        finish_key(key, context);
        it->Next();
    }
}

void Print(KvTable& table, KvTransaction& txn,
                        const std::function<std::string(const Value& key)>& print_key,
                        const std::function<std::string(const Value& key)>& print_value) {
    std::string line;
    Dump(table, txn,
        [print_key](const Value& k, void* l) {
            std::string& line = *(std::string*)l;
            line.clear();
            fma_common::StringFormatter::Append(line, "[{}] : ", print_key(k));
        },
        [print_value](const Value& v, void* l) {
            std::string& line = *(std::string*)l;
            fma_common::StringFormatter::Append(line, " [{}]", print_value(v));
        },
        [](const Value& k, void* l) {
            std::string& line = *(std::string*)l;
            std::cout << line;
        },
        &line);
}

static void DumpTable(KvTable& tab, KvTransaction& txn,
                      const std::function<std::string(const Value&)>& print_key,
                      const std::function<std::string(const Value&)>& print_value) {
    auto it = tab.GetIterator(txn);
    it->GotoFirstKey();
    while (it->IsValid()) {
        std::string line;
        line = line + "[" + print_key(it->GetKey()) + "]: {" + print_value(it->GetValue());
        line += "}";
        UT_LOG() << line;
        it->Next();
    }
}

static void InsertNameAge(const std::string& name, int age, KvTable& tab, KvTransaction& txn) {
    tab.SetValue(txn, Value::ConstRef(name), Value::ConstRef(age));
}

static void CleanStore(KvStore& store) {
    auto txn = store.CreateWriteTxn();
    store.DropAll(*txn);
    txn->Commit();
}

TEST_F(TestKvStore, KvStore) {
    size_t n_subdb = 10;
    size_t db_size = 30;
    Configuration config;
    config.Add(n_subdb, "num_db", true).Comment("Number of concurrent dbs to open");
    config.Add(db_size, "db_size", true).Comment("Size of DB, as power of 2");
    config.ParseAndFinalize(_ut_argc, _ut_argv);
    {
        UT_LOG() << "Testing open table with abort";
        AutoCleanDir cleaner("./testkv");
        auto store = std::make_unique<LMDBKvStore>("./testkv");
        auto txn = store->CreateWriteTxn();
        auto tbl = store->OpenTable(*txn, "table", true, ComparatorDesc::DefaultComparator());
        txn->Abort();
        txn = store->CreateWriteTxn();
        tbl = store->OpenTable(*txn, "table", true, ComparatorDesc::DefaultComparator());
        auto it = tbl->GetIterator(*txn);
        it->GotoFirstKey();
        UT_EXPECT_TRUE(!it->IsValid());
    }
#if (!LGRAPH_USE_MOCK_KV)
    {
        UT_LOG() << "Testing last op id";
        AutoCleanDir dir1("./testkv");
        {
            UT_LOG() << "\tTesting with newly created kv";
            // this must come first, otherwise the LastOpId will change
            auto store = std::make_unique<LMDBKvStore>("./testkv");
            auto txn = store->CreateReadTxn();
            UT_EXPECT_EQ(txn->TxnId(), 0);
            UT_EXPECT_EQ(txn->LastOpId(), -1);
            txn->Abort();
            txn = store->CreateWriteTxn();
            UT_EXPECT_EQ(txn->LastOpId(), -1);
            LMDBKvStore::SetLastOpIdOfAllStores(100);
            // need to write something, otherwise the txn won't be written
            auto tbl = store->OpenTable(*txn, "tmp", true, ComparatorDesc::DefaultComparator());
            txn->Commit();
            txn = store->CreateReadTxn();
            UT_EXPECT_EQ(txn->TxnId(), 1);
            UT_EXPECT_EQ(txn->LastOpId(), 100);
            txn->Abort();
            txn = store->CreateWriteTxn();
            UT_EXPECT_EQ(txn->LastOpId(), 100);
            txn->Abort();
        }
        {
            UT_LOG() << "\tTesting with an existing kv";
            auto store = std::make_unique<LMDBKvStore>("./testkv");
            auto txn = store->CreateReadTxn();
            UT_EXPECT_EQ(txn->LastOpId(), 100);
            txn->Abort();
            txn = store->CreateWriteTxn();
            UT_EXPECT_EQ(txn->LastOpId(), 100);
            txn->Abort();
        }
    }
#endif
    {
        // Open multiple stores
        std::vector<AutoCleanDir> cleaners;
        std::vector<std::unique_ptr<KvStore>> kvs;
        for (size_t i = 0; i < n_subdb; i++) {
            std::string dir = fma_common::StringFormatter::Format("./testkv{}", i);
            cleaners.emplace_back(dir);
            kvs.emplace_back(new LMDBKvStore(dir, (size_t)1 << db_size));
        }
        std::vector<std::unique_ptr<KvTransaction>> txns;
        for (auto& s : kvs) {
            txns.emplace_back(s->CreateWriteTxn());
        }
        for (size_t i = 0; i < txns.size(); i++) {
            kvs[i]->DropAll(*txns[i]);
        }
        for (auto& txn : txns) txn->Commit();
    }

    // TODO(botu.wzy)
    /*UT_LOG() << "Testing KvException";
    {
        // test KvException
        KvException e(0, Value::ConstRef("This is a key").MakeMdbVal(),
                      Value::ConstRef(0xf0f0f0).MakeMdbVal());
        UT_LOG() << "KvException test: " << e.what();
    }*/

    // kvstore mkdir fail(/dev/null)
    UT_LOG() << "Testing opening non-existing DB";
#if _WIN32
    const std::string no_permission_dir = "NUL";
#else
    const std::string no_permission_dir = "/dev/null";
#endif
    UT_EXPECT_THROW(std::make_unique<LMDBKvStore>(no_permission_dir, (size_t)1 << db_size, true),
        lgraph_api::LgraphException);

#if (!LGRAPH_USE_MOCK_KV)
    try {
        auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << db_size, true);
        {
            CleanStore(*store);
            UT_LOG() << "Testing iterator refresh";
            auto txn = store->CreateWriteTxn();
            auto tb1 = store->OpenTable(*txn, "tb1", true, ComparatorDesc::DefaultComparator());
            txn->Commit();
            txn = store->CreateWriteTxn();
            {
                tb1->AddKV(*txn, Value::ConstRef("k1"), Value::ConstRef("v1"));
                auto it = tb1->GetIterator(*txn, Value::ConstRef("k1"));
                UT_EXPECT_EQ(it->GetValue().AsString(), "v1");
            }
            txn->Abort();
            txn = store->CreateWriteTxn();
            {
                tb1->AddKV(*txn, Value::ConstRef("k1"), Value::ConstRef("v1"));
                auto it = tb1->GetIterator(*txn);
                it->GotoFirstKey();
                UT_EXPECT_TRUE(!it->UnderlyingPointerModified());
                // overwriting key modifies it
                tb1->SetValue(*txn, Value::ConstRef("k1"), Value::ConstRef("vs2"));
                UT_EXPECT_TRUE(it->UnderlyingPointerModified());
                UT_EXPECT_TRUE(it->RefreshAfterModify());
                // insert behind it should not affect it
                tb1->AddKV(*txn, Value::ConstRef("k2"), Value::ConstRef("v2"));
                /*UT_EXPECT_TRUE(!it.UnderlyingPointerModified());*/
                std::string buf(400, 'x');
                for (int i = 0; i < 10; i++) {
                    /*if (i < 9) UT_EXPECT_TRUE(!it.UnderlyingPointerModified());*/
                    std::string key = "x" + std::to_string(i);
                    tb1->AddKV(*txn, Value::ConstRef(key), Value::ConstRef(buf));
                }
                // page split will modify it
                UT_EXPECT_TRUE(it->UnderlyingPointerModified());
                it->RefreshAfterModify();
                UT_EXPECT_TRUE(!it->UnderlyingPointerModified());
                UT_EXPECT_EQ(it->GetValue().AsString(), "vs2");
                // overwriting with huge value should also work
                std::string huge_value(4098, 0);
                tb1->SetValue(*txn, Value::ConstRef("k1"), Value::ConstRef(huge_value));
                UT_EXPECT_TRUE(it->UnderlyingPointerModified());
                it->RefreshAfterModify();
                UT_EXPECT_TRUE(!it->UnderlyingPointerModified());
                UT_EXPECT_EQ(it->GetValue().AsString(), huge_value);
                // shrinking the value
                tb1->SetValue(*txn, Value::ConstRef("k1"), Value::ConstRef("v2"));
                UT_EXPECT_TRUE(it->UnderlyingPointerModified());
                it->RefreshAfterModify();
                UT_EXPECT_EQ(it->GetValue().AsString(), "v2");
                // setting values after it should be fine, as long as we don't cause a page
                // split
                tb1->SetValue(*txn, Value::ConstRef("k2"), Value::ConstRef("22"));
                UT_EXPECT_TRUE(!it->UnderlyingPointerModified());
                tb1->SetValue(*txn, Value::ConstRef("k2"), Value::ConstRef("vv22"));
                /*UT_EXPECT_TRUE(!it.UnderlyingPointerModified());*/
                UT_EXPECT_EQ(it->GetValue().AsString(), "v2");
                // delete will cause it to point to k2
                tb1->DeleteKey(*txn, Value::ConstRef("k1"));
                UT_EXPECT_TRUE(it->UnderlyingPointerModified());
                it->RefreshAfterModify();
                UT_EXPECT_EQ(it->GetKey().AsString(), "k2");
                UT_EXPECT_EQ(it->GetValue().AsString(), "vv22");
                // modifying x0
                tb1->SetValue(*txn, Value::ConstRef("x0"), Value::ConstRef("xxx"));
                /*UT_EXPECT_TRUE(!it.UnderlyingPointerModified());*/
                UT_EXPECT_EQ(it->GetValue().AsString(), "vv22");
                // forcing a page split by setting large values to x0 and x1
                huge_value.resize(1024);
                for (int i = 0; i < 3; i++) {
                    std::string key = "x" + std::to_string(i);
                    tb1->SetValue(*txn, Value::ConstRef(key), Value::ConstRef(huge_value));
                }
                UT_EXPECT_TRUE(it->UnderlyingPointerModified());
                it->RefreshAfterModify();
                UT_EXPECT_EQ(it->GetKey().AsString(), "k2");
                UT_EXPECT_EQ(it->GetValue().AsString(), "vv22");
                // deleting
                it->GotoKey(Value::ConstRef("x5"));
                UT_EXPECT_TRUE(it->IsValid());
                for (int i = 10; i > 5; i--) {
                    std::string key = "x" + std::to_string(i);
                    tb1->DeleteKey(*txn, Value::ConstRef(key));
                    if (it->UnderlyingPointerModified()) {
                        UT_LOG() << "\t iterator modified at " << i;
                        it->RefreshAfterModify();
                    }
                }
            }
            txn->Abort();
            txn = store->CreateWriteTxn();
            {
                UT_EXPECT_TRUE(tb1->AddKV(*txn, Value::ConstRef("key2"),
                                          Value::ConstRef("value2")));
                auto it2 = tb1->GetIterator(*txn);
                it2->GotoFirstKey();
                UT_EXPECT_EQ(it2->GetKey().AsString(), "key2");
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value2");
                std::string v2(4096, 'v');
                UT_EXPECT_TRUE(
                    tb1->SetValue(*txn, Value::ConstRef("key2"), Value::ConstRef(v2), true));
                UT_EXPECT_TRUE(it2->UnderlyingPointerModified());
                UT_EXPECT_TRUE(it2->RefreshAfterModify());
                UT_EXPECT_TRUE(!it2->UnderlyingPointerModified());
                UT_EXPECT_EQ(it2->GetValue().AsString(), v2);
                v2 = "value2";
                UT_EXPECT_TRUE(
                    tb1->SetValue(*txn, Value::ConstRef("key2"), Value::ConstRef(v2), true));
                UT_EXPECT_TRUE(it2->UnderlyingPointerModified());
                UT_EXPECT_TRUE(it2->RefreshAfterModify());
                UT_EXPECT_EQ(it2->GetValue().AsString(), v2);

                UT_LOG() << "    testing page-split-merge-caused iterator refresh";
                // insert before
                for (int i = 0; i < 1000; i++) {
                    std::string key = "a" + std::to_string(i);
                    UT_EXPECT_TRUE(tb1->AddKV(*txn, Value::ConstRef(key),
                                              Value::ConstRef("value")));
                }
                UT_EXPECT_TRUE(it2->RefreshAfterModify());
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value2");
                UT_EXPECT_TRUE(it2->Prev());
                UT_EXPECT_EQ(it2->GetKey().AsString(), "a999");
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value");
                // insert after
                for (int i = 0; i < 1000; i++) {
                    std::string key = "x" + std::to_string(i);
                    UT_EXPECT_TRUE(tb1->AddKV(*txn, Value::ConstRef(key),
                                              Value::ConstRef("value")));
                }
                UT_EXPECT_TRUE(it2->RefreshAfterModify());
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value");
                UT_EXPECT_TRUE(it2->Next());
                UT_EXPECT_EQ(it2->GetKey().AsString(), "key2");
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value2");

                auto it3 = tb1->GetIterator(*txn, Value::ConstRef("a999"));
                // delete before it
                size_t i = 0;
                auto it = tb1->GetIterator(*txn);
                for (it->GotoFirstKey(); it->IsValid();) {
                    i++;
                    if (it->GetKey().AsString()[0] == 'a') {
                        it->DeleteKey();
                    } else {
                        break;
                    }
                }
                // deleted iterator will point to next key
                UT_EXPECT_TRUE(it3->RefreshAfterModify());
                UT_EXPECT_EQ(it3->GetKey().AsString(), "key2");
                UT_EXPECT_EQ(it3->GetValue().AsString(), "value2");
                // repositioned iterator still points to the same key
                UT_EXPECT_TRUE(it2->RefreshAfterModify());
                UT_EXPECT_EQ(it2->GetKey().AsString(), "key2");
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value2");
                UT_EXPECT_TRUE(it2->Next());
                UT_EXPECT_EQ(it2->GetKey().AsString(), "x0");
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value");
                it2->Prev();
                UT_EXPECT_EQ(it2->GetKey().AsString(), "key2");
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value2");
                // now it2=it3
                it2->SetValue(Value::ConstRef("new_value"));
                // updated iterator still point to the same key
                UT_EXPECT_TRUE(it3->RefreshAfterModify());
                UT_EXPECT_EQ(it3->GetKey().AsString(), "key2");
                UT_EXPECT_EQ(it3->GetValue().AsString(), "new_value");
            }
            txn->Commit();
        }
        try {
            CleanStore(*store);
            UT_LOG() << "    testing delete-caused iterator refresh";
            auto txn = store->CreateWriteTxn();
            {
                auto tb1 = store->OpenTable(*txn, "tb1",
                                            true, ComparatorDesc::DefaultComparator());
                UT_EXPECT_TRUE(tb1->AddKV(*txn, Value::ConstRef("key1"),
                                          Value::ConstRef("value1")));
                auto it1 = tb1->GetIterator(*txn);
                it1->GotoFirstKey();
                UT_EXPECT_EQ(it1->GetKey().AsString(), "key1");
                auto it2 = tb1->GetIterator(*txn);
                it2->GotoFirstKey();
                UT_EXPECT_EQ(it2->GetValue().AsString(), "value1");
                it2->DeleteKey();
                UT_EXPECT_TRUE(!it2->IsValid());
                it1->RefreshAfterModify();
                UT_EXPECT_TRUE(!it1->IsValid());
            }
            txn->Commit();
        } catch (std::exception& e) {
            UT_ERR() << e.what();
        }
        {
            CleanStore(*store);
            UT_LOG() << "    testing insert-caused iterator refresh";
            auto txn = store->CreateWriteTxn();
            {
                auto tb1 = store->OpenTable(*txn, "tb1",
                                            true, ComparatorDesc::DefaultComparator());
                UT_EXPECT_TRUE(tb1->AddKV(*txn, Value::ConstRef("key2"),
                                          Value::ConstRef("value2")));
                auto it2 = tb1->GetIterator(*txn);
                it2->GotoFirstKey();
                UT_EXPECT_EQ(it2->GetKey().AsString(), "key2");
                UT_EXPECT_TRUE(tb1->AddKV(*txn, Value::ConstRef("key1"),
                                          Value::ConstRef("value1")));
                auto it1 = tb1->GetIterator(*txn);
                it1->GotoFirstKey();
                UT_EXPECT_EQ(it1->GetValue().AsString(), "value1");
                UT_EXPECT_TRUE(it2->RefreshAfterModify());
                UT_EXPECT_EQ(it2->GetKey().AsString(), "key2");
                it2->DeleteKey();
                UT_EXPECT_TRUE(it1->RefreshAfterModify());
                UT_EXPECT_TRUE(it1->IsValid());
                UT_EXPECT_TRUE(!it2->IsValid());
                UT_EXPECT_EQ(tb1->GetKeyCount(*txn), 1);
                auto it = tb1->GetIterator(*txn);
                it->GotoFirstKey();
                UT_EXPECT_EQ(it1->GetValue().AsString(), "value1");
                UT_EXPECT_EQ(it->GetValue().AsString(), "value1");
            }
            txn->Commit();
        }
    } catch (std::exception& e) {
        UT_LOG() << e.what();
        UT_EXPECT_TRUE(false);
    }
#endif

    try {
        auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << db_size, true);
        // Start test, see if we already has a db
        auto txn = store->CreateWriteTxn();
        std::vector<std::string> tables = store->ListAllTables(*txn);
        UT_LOG() << "Cleaning testkv directory";
        if (!tables.empty()) {
            store->DropAll(*txn);
        }
        txn->Commit();

        UT_LOG() << "Testing Insert";
        // now create two tables
        txn = store->CreateWriteTxn();
        auto name_age =
            store->OpenTable(*txn, "name_age", true, ComparatorDesc::DefaultComparator());
        {
            auto it = name_age->GetIterator(*txn);
            it->GotoFirstKey();
            UT_EXPECT_TRUE(!it->IsValid());
        }
        InsertNameAge("allice", 11, *name_age, *txn);
        InsertNameAge("bob", 11, *name_age, *txn);
        InsertNameAge("cindy", 9, *name_age, *txn);
        InsertNameAge("denial", 10, *name_age, *txn);
        InsertNameAge("ella", 4, *name_age, *txn);
        InsertNameAge("fiona", 4, *name_age, *txn);
        UT_LOG() << "";
        UT_LOG() << "Now the name-age table contains the following:";
        DumpTable(
            *name_age, *txn, [](const Value& v) { return v.AsString(); },
            [](const Value& v) { return ToString(v.AsType<int>()); });
        txn->Commit();

        // Testing read
        UT_LOG() << "";
        UT_LOG() << "Checking consistency of the tables...";
        txn = store->CreateReadTxn();
        UT_EXPECT_TRUE(name_age->HasKey(*txn, Value::ConstRef("ella")));
        UT_EXPECT_TRUE(!name_age->HasKey(*txn, Value::ConstRef("nobody")));
        UT_EXPECT_THROW(name_age->HasKey(*txn, Value::ConstRef("")), lgraph_api::LgraphException);
        UT_EXPECT_THROW(name_age->GetValue(*txn, Value::ConstRef("")), lgraph_api::LgraphException);
        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("allice")).AsType<int>(), 11);
        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("nobody")).Size(), 0);

        try {
            name_age->SetValue(*txn, Value::ConstRef("ella"), Value::ConstRef(9));
            FMA_ASSERT(false);
        } catch (lgraph_api::LgraphException& e) {
            // TODO(botu.wzy): check lmdb error code
            // UT_EXPECT_EQ(e.code(), EACCES);
            UT_LOG() << "Expected exception: Trying to write in read transaction yields an "
                        "exception "
                     << " and error message: " << e.what();
        }

        // Testing write
        UT_LOG() << "";
        UT_LOG() << "Testing modifications...";
        txn = store->CreateWriteTxn();
        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("ella"), Value::ConstRef(9)));
        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("george"), Value::ConstRef(5)));
        // key exist and overwrite=false, then this should return false
        UT_EXPECT_TRUE(
            !name_age->SetValue(*txn, Value::ConstRef("george"), Value::ConstRef(15), false));

        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("hank"), Value::ConstRef(6)));
        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("ivy"), Value::ConstRef(7)));
        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("hank")).AsType<int>(), 6);
        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("ivy")).AsType<int>(), 7);

        UT_EXPECT_TRUE(name_age->DeleteKey(*txn, Value::ConstRef("hank")));
        UT_EXPECT_TRUE(!name_age->HasKey(*txn, Value::ConstRef("hank")));
        UT_EXPECT_TRUE(!name_age->DeleteKey(*txn, Value::ConstRef("noone")));
        UT_EXPECT_TRUE(name_age->DeleteKey(*txn, Value::ConstRef("ivy")));
        UT_EXPECT_TRUE(!name_age->HasKey(*txn, Value::ConstRef("ivy")));
        UT_EXPECT_TRUE(!name_age->DeleteKey(*txn, Value::ConstRef("ivy")));
        UT_EXPECT_THROW(name_age->DeleteKey(*txn, Value::ConstRef("")),
                        lgraph_api::LgraphException);
        UT_LOG() << "Now name-age table becomes:";
        DumpTable(
            *name_age, *txn, [](const Value& v) { return v.AsString(); },
            [](const Value& v) { return ToString(v.AsType<int>()); });
        txn->Commit();

        // Testing iterators
        UT_LOG() << "";
        UT_LOG() << "Testing iterators...";
        txn = store->CreateWriteTxn();
        {
            auto it = name_age->GetIterator(*txn);
            it->GotoFirstKey();
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("allice"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(), 11);
            for (int i = 0; i < 4; i++) {
                UT_EXPECT_TRUE(it->Next());
            }
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("ella"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(), 9);
            it->DeleteKey();
            UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef("big_ella"), Value::ConstRef(9)));
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("big_ella"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(),
                         9);  // now the iterator points to "big_ella"
            UT_EXPECT_TRUE(!it->AddKeyValue(Value::ConstRef("big_ella"), Value::ConstRef(9)));
            UT_EXPECT_TRUE(it->Next());
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("bob"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(), 11);
            it->DeleteKey();
            it = name_age->GetIterator(*txn, Value::ConstRef("denial"));
            it->DeleteKey();

            UT_LOG() << "Now name-age table becomes:";
            DumpTable(
                *name_age, *txn, [](const Value& v) { return v.AsString(); },
                [](const Value& v) { return ToString(v.AsType<int>()); });
        }
        txn->Commit();

        // test SetFixedSizeValue
        {
            txn = store->CreateWriteTxn();
            {
                auto tb2 =
                    store->OpenTable(*txn, "setv", true, ComparatorDesc::DefaultComparator());
                auto it = tb2->GetIterator(*txn);
                it->GotoFirstKey();
                UT_EXPECT_TRUE(!it->IsValid());
                std::string v(1000, 1);
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(1), Value::ConstRef(v), false));
                UT_EXPECT_TRUE(it->IsValid());
                std::string v2(2000, 1);
                it->SetValue(Value::ConstRef(v2));
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 1);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(2),
                    Value::ConstRef(v2), false));
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(3),
                    Value::ConstRef(v2), false));
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(4),
                    Value::ConstRef(v2), false));
                UT_EXPECT_TRUE(it->IsValid());
                it->Prev();
                it->Prev();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 2);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                it->DeleteKey();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 3);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                it->DeleteKey();
                it->DeleteKey();
                UT_EXPECT_TRUE(!it->IsValid());
                it->Prev();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 1);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                UT_EXPECT_TRUE(tb2->SetValue(*txn, Value::ConstRef<int>(1), Value::ConstRef(v)));
                it->RefreshAfterModify();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 1);
                UT_EXPECT_EQ(it->GetValue().AsString(), v);
            }
            txn->Commit();
        }

        {
            UT_LOG() << "Testing transaction fork...";
            txn = store->CreateReadTxn();
            {
                auto table =
                    store->OpenTable(*txn, "name_age", false, ComparatorDesc::DefaultComparator());
                auto it = table->GetIterator(*txn);
                it->GotoFirstKey();
                size_t n = 0;
                while (it->IsValid()) {
                    n++;
                    it->Next();
                }
                table->GetClosestIterator(*txn, Value::ConstRef("name_age"));
                UT_LOG() << "master thread read " << n << " keys.";
                std::vector<std::thread> threads;
                static const int nt = 10;
                std::vector<size_t> n_keys(nt, 0);
                for (int i = 0; i < nt; i++) {
                    threads.emplace_back([&txn, &n_keys, &table, i]() {
                        auto ntxn = txn->Fork();
                        auto it = table->GetIterator(*ntxn);
                        it->GotoFirstKey();
                        size_t n = 0;
                        while (it->IsValid()) {
                            n++;
                            it->Next();
                        }
                        n_keys[i] = n;
                        UT_LOG() << "thread " << i << " read " << n << " keys.";
                    });
                }
                for (auto& t : threads) t.join();
                for (int i = 0; i < nt; i++) UT_EXPECT_EQ(n, n_keys[i]);
            }
            txn->Commit();
        }
        size_t size;
        store->WarmUp(&size);
#if (!LGRAPH_USE_MOCK_KV)
        UT_EXPECT_EQ(size, 1054);
#endif
    } catch (std::exception& e) {
        LOG_FATAL() << e.what();
    }

#if (!LGRAPH_USE_MOCK_KV)
    // try
    {
        UT_LOG() << "Testing optimistic write transactions";
        auto store = std::make_unique<LMDBKvStore>("./testkv", (size_t)1 << db_size, true);
        // Start test, see if we already has a db
        UT_LOG() << "Initializing";
        // the first transaction must open all tables and commit successfully
        auto txn = store->CreateWriteTxn();
        auto tables = store->ListAllTables(*txn);
        for (auto table : tables) {
            store->OpenTable(*txn, table, false, ComparatorDesc::DefaultComparator());
            UT_LOG() << "Opened table " << table;
        }
        txn->Commit();
        UT_LOG() << "Cleaning testkv directory with an optimistic write transaction";
        txn = store->CreateWriteTxn(true);
        tables = store->ListAllTables(*txn);
        if (!tables.empty()) {
            UT_EXPECT_ANY_THROW(store->DropAll(*txn));
        }
        txn->Abort();
        UT_LOG() << "Cleaning testkv directory with an exclusive write transaction";
        txn = store->CreateWriteTxn();
        tables = store->ListAllTables(*txn);
        if (!tables.empty()) {
            store->DropAll(*txn);
        }
        txn->Commit();

        UT_LOG() << "Creating table";
        txn = store->CreateWriteTxn();
        auto name_age =
            store->OpenTable(*txn, "name_age", true, ComparatorDesc::DefaultComparator());
        txn->Commit();

        UT_LOG() << "Testing Insert";
        // now create two tables
        txn = store->CreateWriteTxn(true);
        {
            auto it = name_age->GetIterator(*txn);
            it->GotoFirstKey();
            UT_EXPECT_TRUE(!it->IsValid());
        }
        InsertNameAge("allice", 11, *name_age, *txn);
        InsertNameAge("bob", 11, *name_age, *txn);
        InsertNameAge("cindy", 9, *name_age, *txn);
        InsertNameAge("denial", 10, *name_age, *txn);
        InsertNameAge("ella", 4, *name_age, *txn);
        InsertNameAge("fiona", 4, *name_age, *txn);
        UT_LOG() << "";
        UT_LOG() << "Now the name-age table contains the following:";
        DumpTable(
            *name_age, *txn, [](const Value& v) { return v.AsString(); },
            [](const Value& v) { return ToString(v.AsType<int>()); });
        Print(*name_age,
            *txn, [](const Value& v) { return v.AsString(); },
            [](const Value& v) { return ToString(v.AsType<int>()); });
        txn->Commit();
        // Testing read
        UT_LOG() << "";
        UT_LOG() << "Checking consistency of the tables...";
        txn = store->CreateReadTxn();
        UT_EXPECT_TRUE(name_age->HasKey(*txn, Value::ConstRef("ella")));
        UT_EXPECT_TRUE(!name_age->HasKey(*txn, Value::ConstRef("nobody")));
        UT_EXPECT_THROW(name_age->HasKey(*txn, Value::ConstRef("")), lgraph_api::LgraphException);
        UT_EXPECT_THROW(name_age->GetValue(*txn, Value::ConstRef("")), lgraph_api::LgraphException);

        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("allice")).AsType<int>(), 11);
        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("nobody")).Size(), 0);

        try {
            name_age->SetValue(*txn, Value::ConstRef("ella"), Value::ConstRef(9));
        } catch (lgraph_api::LgraphException& e) {
            // UT_EXPECT_EQ(e.code(), EACCES);
            // TODO(botu.wzy) check lmdb error code
            UT_LOG() << "Expected exception: Trying to write in read transaction yields an "
                        "exception "
                     << " and error message: " << e.what();
        }
        /* abort should not matter here, since when we assign txn again, the old one will be
         *  aborted automatically.
         */
        txn->Abort();  // abort must be called here, otherwise MDB_BAD_RSLOT would trigger

        // Testing write
        UT_LOG() << "";
        UT_LOG() << "Testing modifications...";
        txn = store->CreateWriteTxn(true);
        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("ella"), Value::ConstRef(9)));
        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("george"), Value::ConstRef(5)));
        // key exist and overwrite=false, then this should return false
        UT_EXPECT_TRUE(
            !name_age->SetValue(*txn, Value::ConstRef("george"), Value::ConstRef(15), false));

        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("hank"), Value::ConstRef(6)));
        UT_EXPECT_TRUE(name_age->SetValue(*txn, Value::ConstRef("ivy"), Value::ConstRef(7)));
        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("hank")).AsType<int>(), 6);
        UT_EXPECT_EQ(name_age->GetValue(*txn, Value::ConstRef("ivy")).AsType<int>(), 7);

        UT_EXPECT_TRUE(name_age->DeleteKey(*txn, Value::ConstRef("hank")));
        UT_EXPECT_TRUE(!name_age->HasKey(*txn, Value::ConstRef("hank")));
        UT_EXPECT_TRUE(!name_age->DeleteKey(*txn, Value::ConstRef("noone")));
        UT_EXPECT_TRUE(name_age->DeleteKey(*txn, Value::ConstRef("ivy")));
        UT_EXPECT_TRUE(!name_age->HasKey(*txn, Value::ConstRef("ivy")));
        UT_EXPECT_TRUE(!name_age->DeleteKey(*txn, Value::ConstRef("ivy")));
        UT_EXPECT_THROW(name_age->DeleteKey(*txn, Value::ConstRef("")),
                        lgraph_api::LgraphException);

        UT_LOG() << "All passed!";
        UT_LOG() << "Now name-age table becomes:";
        DumpTable(
            *name_age, *txn, [](const Value& v) { return v.AsString(); },
            [](const Value& v) { return ToString(v.AsType<int>()); });
        txn->Commit();

        // Testing iterators
        UT_LOG() << "";
        UT_LOG() << "Testing iterators...";
        txn = store->CreateWriteTxn(true);
        {
            auto it = name_age->GetIterator(*txn);
            it->GotoFirstKey();
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("allice"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(), 11);
            for (int i = 0; i < 4; i++) {
                UT_EXPECT_TRUE(it->Next());
            }
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("ella"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(), 9);
            it->DeleteKey();
            UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef("big_ella"), Value::ConstRef(9)));
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("big_ella"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(),
                         9);  // now the iterator points to "big_ella"
            UT_EXPECT_TRUE(!it->AddKeyValue(Value::ConstRef("big_ella"), Value::ConstRef(9)));
            UT_EXPECT_TRUE(it->Next());
            UT_EXPECT_EQ(it->GetKey().AsString(), std::string("bob"));
            UT_EXPECT_EQ(it->GetValue().AsType<int>(), 11);
            it->DeleteKey();
            it = name_age->GetIterator(*txn, Value::ConstRef("denial"));
            it->DeleteKey();

            UT_LOG() << "Now name-age table becomes:";
            DumpTable(
                *name_age, *txn, [](const Value& v) { return v.AsString(); },
                [](const Value& v) { return ToString(v.AsType<int>()); });
        }
        txn->Commit();

        // test SetFixedSizeValue
        {
            txn = store->CreateWriteTxn();
            {
                auto tb2 =
                    store->OpenTable(*txn, "setv", true, ComparatorDesc::DefaultComparator());
                auto it = tb2->GetIterator(*txn);
                it->GotoFirstKey();
                UT_EXPECT_TRUE(!it->IsValid());
                std::string v(1000, 1);
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(1), Value::ConstRef(v), false));
                UT_EXPECT_TRUE(it->IsValid());
                std::string v2(2000, 1);
                it->SetValue(Value::ConstRef(v2));
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 1);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(2),
                    Value::ConstRef(v2), false));
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(3),
                    Value::ConstRef(v2), false));
                UT_EXPECT_TRUE(it->AddKeyValue(Value::ConstRef<int>(4),
                    Value::ConstRef(v2), false));
                UT_EXPECT_TRUE(it->IsValid());
                it->Prev();
                it->Prev();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 2);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                it->DeleteKey();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 3);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                it->DeleteKey();
                it->DeleteKey();
                UT_EXPECT_TRUE(!it->IsValid());
                it->Prev();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 1);
                UT_EXPECT_EQ(it->GetValue().AsString(), v2);
                UT_EXPECT_TRUE(tb2->SetValue(*txn, Value::ConstRef<int>(1), Value::ConstRef(v)));
                it->RefreshAfterModify();
                UT_EXPECT_TRUE(it->IsValid());
                UT_EXPECT_EQ(it->GetKey().AsType<int>(), 1);
                UT_EXPECT_EQ(it->GetValue().AsString(), v);
            }
            txn->Commit();
        }

        {
            UT_LOG() << "Testing transaction fork...";
            txn = store->CreateReadTxn();
            {
                auto table =
                    store->OpenTable(*txn, "name_age", false, ComparatorDesc::DefaultComparator());
                auto it = table->GetIterator(*txn);
                it->GotoFirstKey();
                size_t n = 0;
                while (it->IsValid()) {
                    n++;
                    it->Next();
                }
                table->GetClosestIterator(*txn, Value::ConstRef("name_age"));
                UT_LOG() << "master thread read " << n << " keys.";
                std::vector<std::thread> threads;
                static const int nt = 10;
                std::vector<size_t> n_keys(nt, 0);
                for (int i = 0; i < nt; i++) {
                    threads.emplace_back([&txn, &n_keys, &table, i]() {
                        auto ntxn = txn->Fork();
                        auto it = table->GetIterator(*ntxn);
                        it->GotoFirstKey();
                        size_t n = 0;
                        while (it->IsValid()) {
                            n++;
                            it->Next();
                        }
                        n_keys[i] = n;
                        UT_LOG() << "thread " << i << " read " << n << " keys.";
                    });
                }
                for (auto& t : threads) t.join();
                for (int i = 0; i < nt; i++) UT_EXPECT_EQ(n, n_keys[i]);
            }
            txn->Commit();
        }
        size_t size;
        store->WarmUp(&size);
        UT_EXPECT_EQ(size, 1054);
    }
    {
        UT_LOG() << "Testing optimistic txn with non-conflicting writes";
        AutoCleanDir _("./testkv");
        auto store = std::make_unique<LMDBKvStore>("./testkv");
        auto txn = store->CreateWriteTxn();
        auto table = store->OpenTable(*txn, "mw", true, ComparatorDesc::DefaultComparator());
        txn->Commit();

        static const int nt = 10;
        {
            std::vector<std::thread> threads;
            std::vector<int8_t> results(nt, 0);
            for (int i = 0; i < nt; i++) {
                threads.emplace_back([&store, &table, &results, i]() {
                    auto txn = store->CreateWriteTxn(true);
                    UT_EXPECT_TRUE(
                        table->AddKV(*txn, Value::ConstRef<int>(i), Value::ConstRef<int>(0)));
                    txn->Commit();
                    results[i] = 1;
                });
            }
            for (auto& t : threads) t.join();
            for (int i = 0; i < nt; i++) UT_EXPECT_EQ(results[i], 1);
            UT_LOG() << "all transactions succeeded";
        }
    }
    {
        UT_LOG() << "Testing optimistic txn with conflicting writes";
        AutoCleanDir _("./testkv");
        auto store = std::make_unique<LMDBKvStore>("./testkv");
        auto txn = store->CreateWriteTxn();
        auto table = store->OpenTable(*txn, "mw", true, ComparatorDesc::DefaultComparator());
        txn->Commit();

        size_t nt = 10;
        std::vector<std::thread> threads;
        std::vector<int8_t> results(nt, 0);
        Barrier barrier(nt);
        for (int i = 0; i < nt; i++) {
            threads.emplace_back([&store, &table, &results, i, &barrier]() {
                auto txn = store->CreateWriteTxn(true);
                UT_EXPECT_TRUE(
                    table->SetValue(*txn, Value::ConstRef<int>(0), Value::ConstRef<int>(1), false));
                try {
                    UT_LOG() << "transaction " << i << " waiting";
                    barrier.Wait();
                    txn->Commit();
                    results[i] = 1;
                    UT_LOG() << "transaction " << i << " succeeded";
                } catch (std::exception& e) {
                    UT_LOG() << e.what();
                    results[i] = -1;
                    UT_LOG() << "transaction " << i << " failed";
                }
            });
        }
        for (auto& t : threads) t.join();
        size_t sum = 0;
        for (int i = 0; i < nt; i++) sum += (results[i] == 1) ? 1 : 0;
        UT_LOG() << sum << " transactions succeeded while others failed";
        UT_EXPECT_EQ(sum, 1);
    }
    {
        UT_LOG() << "Testing optimistic txn: first committed txn wins";
        AutoCleanDir _("./testkv");
        auto store = std::make_unique<LMDBKvStore>("./testkv");
        auto txn = store->CreateWriteTxn();
        auto table = store->OpenTable(*txn, "mw", true, ComparatorDesc::DefaultComparator());
        txn->Commit();
        Barrier barrier1(2);
        Barrier barrier2(2);
        std::thread child_thread([&store, &table, &barrier1, &barrier2]() {
            auto txn = store->CreateWriteTxn(true);
            UT_EXPECT_TRUE(table->SetValue(*txn, Value::ConstRef<int>(0), Value::ConstRef<int>(2)));
            barrier1.Wait();
            barrier2.Wait();
            UT_EXPECT_ANY_THROW(txn->Commit());
            UT_LOG() << "optimistic txn failed to commit due to conflicts";
        });
        txn = store->CreateWriteTxn();
        UT_EXPECT_TRUE(table->SetValue(*txn, Value::ConstRef<int>(0), Value::ConstRef<int>(2)));
        barrier1.Wait();
        txn->Commit();
        barrier2.Wait();
        child_thread.join();
    }
#endif
}
