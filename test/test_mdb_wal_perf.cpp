/**
* Copyright 2022 AntGroup CO., Ltd.
*^M
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
#include "core/lmdb/lmdb.h"
#include "core/sync_file.h"
#include "core/kv_store.h"
#include "./random_port.h"
#include "./test_tools.h"
#include "./ut_utils.h"

class TestMdbWalPerf : public TuGraphTest {
 protected:
    std::string path_ = "./testkv";
    size_t n_iter_ = 3;
    size_t n_threads_ = 1;
    size_t n_txns_ = 1000;
    size_t n_kv_per_txn_ = 10;
    size_t key_size_ = 8;
    size_t val_size_ = 256;
    bool mdb_async_ = true;
    bool enable_wal_ = true;
    bool enable_batch_ = false;
    size_t batch_time_ms_ = 100;
    size_t rotate_time_ms_ = 60 * 1000;
    bool optimistic_ = false;

    TestMdbWalPerf() {
        fma_common::Configuration config;
        config.Add(path_, "dir", true)
            .Comment("Dir to store data.");
        config.Add(n_iter_, "niter", true)
            .Comment("Number of iterations to run.");
        config.Add(n_threads_, "nthreads", true)
            .Comment("Number of concurrent threads to use.");
        config.Add(n_txns_, "ntxn", true)
            .Comment("Number of transactions to run.");
        config.Add(n_kv_per_txn_, "nkv", true)
            .Comment("Number of KVs per txn.");
        config.Add(key_size_, "ksize", true)
            .Comment("Key size.");
        config.Add(val_size_, "vsize", true)
            .Comment("Value size.");
        config.Add(mdb_async_, "async", true)
            .Comment("MDB asyc.");
        config.Add(enable_wal_, "wal", true)
            .Comment("Enable wal.");
        config.Add(enable_batch_, "batch", true)
            .Comment("Enable batch mode.");
        config.Add(batch_time_ms_, "batch_time", true)
            .Comment("Time between two batches, in milliseconds.");
        config.Add(optimistic_, "optimistic", true)
            .Comment("Enable optimistic transaction.");
        config.Add(rotate_time_ms_, "rotate_time", true)
            .Comment("Time to rotate log.");
        config.ParseAndFinalize(_ut_argc, _ut_argv);
    }

    void SetUp() {
        TuGraphTest::SetUp();
        if (!_ut_run_benchmarks)
            GTEST_SKIP() << "--run_benchmarks not set skipping benchmarks.";
    }

    void TearDown() {
        TuGraphTest::TearDown();
    }
};

TEST_F(TestMdbWalPerf, TestRaw) {
    std::string vstr(val_size_, 'a');
    size_t keyint = 0;
    for (size_t i = 0; i < n_iter_; i++) {
        lgraph::AutoCleanDir _(path_);
        MDB_env* env;
        UT_EXPECT_EQ(mdb_env_create(&env), MDB_SUCCESS);
        int env_flags = 0;
        if (mdb_async_) env_flags |= MDB_NOSYNC;
        UT_EXPECT_EQ(mdb_env_open(env, path_.c_str(), env_flags, 0), MDB_SUCCESS);
        for (size_t t = 0; t < n_txns_; t++) {
            MDB_txn* txn;
            UT_EXPECT_EQ(mdb_txn_begin(env, nullptr, 0, &txn), MDB_SUCCESS);
            for (size_t k = 0; k < n_kv_per_txn_; k++) {
                keyint++;
            }
        }
        mdb_env_close(env);
    }
}

TEST_F(TestMdbWalPerf, KvStore) {
    using namespace lgraph;
    auto GenRandomNewKey = [this](RandomSeed &seed){
        Value key(key_size_);
        uint8_t* p = (uint8_t*)key.Data();
        for (size_t i = 0; i < key.Size(); i++) {
            p[i] = (uint8_t)(rand_r(&seed));
        }
        return key;
    };
    for (size_t i = 0; i < n_iter_; i++) {
        UT_LOG() << "Running with " << n_threads_ << " threads, "
                 << n_threads_*n_txns_ << " transaction,"
                 << n_threads_*n_txns_*n_kv_per_txn_ << " kvs";
        AutoCleanDir _("./testkv");
        auto store = std::make_unique<LMDBKvStore>("./testkv", 1 << 30, enable_wal_,
                                                   true, rotate_time_ms_, batch_time_ms_);
        auto txn = store->CreateWriteTxn();
        auto table = store->OpenTable(*txn, "default", true, ComparatorDesc::DefaultComparator());
        txn->Commit();
        std::string vstr(val_size_, 'a');
        Value value = Value::ConstRef(vstr);
        double t1 = fma_common::GetTime();
        std::vector<std::thread> threads;
        for (size_t thr = 0; thr < n_threads_; thr++) {
            threads.emplace_back([this, thr, &store, &table, &GenRandomNewKey, &value]() {
                RandomSeed seed = thr;
                Value key = GenRandomNewKey(seed);
                for (size_t t = 0; t < n_txns_; t++) {
                    auto txn = store->CreateWriteTxn(optimistic_);
                    for (size_t k = 0; k < n_kv_per_txn_; k++) {
                        table->SetValue(*txn, key, value);
                    }
                    txn->Commit();
                }
            });
        }
        for (auto&t : threads) t.join();
        double t2 = fma_common::GetTime();
        double duration = t2 - t1;
        size_t nk = n_threads_ * n_txns_ * n_kv_per_txn_;
        UT_LOG() << "Finished in "
                << duration << " seconds at "
                << (double)nk / duration << " kv/s and "
                << (double)n_threads_ * n_txns_ / duration << " txn/s";
    }
}
