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
#include <rocksdb/db.h>
#include <rocksdb/convenience.h>
#include <rocksdb/utilities/transaction_db.h>
#include <filesystem>
#include "common/logger.h"
namespace fs = std::filesystem;

static std::string testkv = "testkv";
TEST(TxnKV, basic) {
    GTEST_SKIP();
    fs::remove_all(testkv);
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    rocksdb::TransactionDBOptions txn_db_options;
    rocksdb::TransactionDB* db = nullptr;
    auto s = rocksdb::TransactionDB::Open(options, txn_db_options, testkv,&db);
    assert(s.ok());

    rocksdb::WriteOptions wo;
    rocksdb::TransactionOptions to;
    rocksdb::Transaction* txn = db->BeginTransaction(wo, to);
    for (auto i = 0; i < 10; i++) {
        std::string key = "key" + std::to_string(i);
        s = txn->Put(key, key);
        assert(s.ok());
    }
    int count = 0;
    rocksdb::ReadOptions ro;
    auto iter = txn->GetIterator(ro);
    for (iter->Seek("key5"); iter->Valid(); iter->Next()) {
        count++;
        LOG_INFO("key1: {}", iter->key().ToString());
        if (iter->key() == "key6") {
            txn->Delete("key7");
        }
        if (iter->key() == "key8") {
            txn->Delete("key2");
        }
    }
    for (iter->Seek("key0"); iter->Valid(); iter->Next()) {
        count++;
        LOG_INFO("key2: {}", iter->key().ToString());
        //dbtxn->Delete(iter->key());
    }
    /*for (auto i = 0; i < 10; i++) {
        std::string key = "key" + std::to_string(i);
        s = dbtxn->Delete(key);
        assert(s.ok());
    }
    for (iter->SeekToFirst(); iter->Valid(); iter->Next()) {
        LOG_INFO("key2: {}", iter->key().ToString());
    }*/
    delete iter;
    delete txn;
    db->Close();
    delete db;
}

TEST(TxnKV, basic1) {
    GTEST_SKIP();
    fs::remove_all(testkv);
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    rocksdb::TransactionDBOptions txn_db_options;
    rocksdb::TransactionDB* db = nullptr;
    auto s = rocksdb::TransactionDB::Open(options, txn_db_options, testkv,&db);
    assert(s.ok());
    rocksdb::WriteOptions wo;
    rocksdb::TransactionOptions to;
    rocksdb::Transaction* txn = db->BeginTransaction(wo, to);
    for (auto i = 0; i < 10; i++) {
        std::string key = "key" + std::to_string(i);
        s = txn->Put(key, key);
        assert(s.ok());
    }
    rocksdb::ReadOptions ro;
    auto iter = txn->GetIterator(ro);
    for (iter->Seek("key0"); iter->Valid(); iter->Next()) {
        LOG_INFO("key1: {}, val:{}", iter->key().ToString(), iter->value().ToString());
        std::string new_val("new_");
        new_val.append(iter->key().ToString());
        txn->Put(iter->key(), new_val);
        LOG_INFO("key2: {}, val:{}", iter->key().ToString(), iter->value().ToString());
    }
    for (iter->Seek("key0"); iter->Valid(); iter->Next()) {
        LOG_INFO("key3: {}, val:{}", iter->key().ToString(), iter->value().ToString());
    }
    for (iter->Seek("key0"); iter->Valid(); iter->Next()) {
        LOG_INFO("key4: {}, val:{}", iter->key().ToString(), iter->value().ToString());
        txn->Delete(iter->key());
    }
    for (iter->Seek("key0"); iter->Valid(); iter->Next()) {
        LOG_INFO("key5: {}, val:{}", iter->key().ToString(), iter->value().ToString());
    }
    delete iter;
    txn->Commit();
    delete iter;
    db->Close();
    delete db;
}

TEST(TxnKV, basic2) {
    GTEST_SKIP();
    fs::remove_all(testkv);
    rocksdb::Options options;
    options.create_if_missing = true;
    options.create_missing_column_families = true;
    rocksdb::TransactionDBOptions txn_db_options;
    rocksdb::TransactionDB* db = nullptr;
    auto s = rocksdb::TransactionDB::Open(options, txn_db_options, testkv,&db);
    assert(s.ok());
    rocksdb::WriteOptions wo;
    rocksdb::TransactionOptions to;
    rocksdb::Transaction* txn = db->BeginTransaction(wo, to);
    for (auto i = 0; i < 10; i++) {
        std::string key = "key" + std::to_string(i);
        s = txn->Put(key, key);
        assert(s.ok());
    }
    for (auto i = 0; i < 10; i++) {
        std::string key = "key" + std::to_string(i);
        s = txn->Put(key, key);
        assert(s.ok());
    }
    rocksdb::ReadOptions ro;
    auto iter = txn->GetIterator(ro);
    txn->Delete("key0");
    iter->Seek("key0");
    txn->Delete("key1");
    txn->Put("key5", "new_key5");
    txn->Put("key77", "new_key77");
    txn->Delete("key6");
    for (; iter->Valid(); iter->Next()) {
        LOG_INFO("key: {}, val:{}", iter->key().ToString(), iter->value().ToString());
    }
    delete iter;
    txn->Commit();
    delete iter;
    db->Close();
    delete db;
}