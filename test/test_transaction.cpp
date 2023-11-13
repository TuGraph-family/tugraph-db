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
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "./test_tools.h"
#include "./ut_utils.h"

using namespace lgraph;

class TestTransaction : public TuGraphTest {};

TEST_F(TestTransaction, TestConcurrentVertexAdd) {
    size_t n_threads = 2;
    fma_common::Configuration config;
    config.Add(n_threads, "nt", true).Comment("Number of concurrent threads").SetMin(1);
    config.ParseAndFinalize(_ut_argc, _ut_argv);

    const std::string& dir = "./testdb";
    AutoCleanDir _(dir);
    lgraph::DBConfig conf;
    conf.dir = dir;
    LightningGraph db(conf);
    db.AddLabel("v",
        std::vector<lgraph::FieldSpec>{lgraph::FieldSpec("id", lgraph::FieldType::INT64, false)},
        true, lgraph::VertexOptions("id"));
    Barrier bar(n_threads);
    std::atomic<size_t> n_success(0);
    std::atomic<size_t> n_fail(0);
    std::vector<std::thread> threads;
    for (size_t i = 0; i < n_threads; i++) {
        threads.emplace_back([&, i]() {
            auto txn = db.CreateWriteTxn(true);
            txn.AddVertex(std::string("v"), std::vector<std::string>{"id"},
                          std::vector<std::string>{std::to_string(i)});
            bar.Wait();
            try {
                txn.Commit();
                n_success++;
            } catch (lgraph_api::TxnConflictError&) {
                n_fail++;
            }
        });
    }
    for (auto& t : threads) t.join();
    UT_EXPECT_EQ(n_success.load(), 1);
    UT_EXPECT_EQ(n_fail.load(), n_threads - 1);
    {
        auto txn = db.CreateReadTxn();
        UT_EXPECT_EQ(txn.GetLooseNumVertex(), 1);
    }
}

TEST_F(TestTransaction, Transaction) {
#define CAC(txn)  \
    txn.Commit(); \
    txn.Abort();  \
    txn.Commit();

    using namespace fma_common;
    using namespace lgraph;

    size_t nt = 10;

    DBConfig config;
    config.dir = "./testdb";
    LightningGraph db(config);
    {
        UT_LOG() << "Testing constructor and assignment";
        {
            UT_LOG() << "Case 1";
            auto txn = db.CreateWriteTxn();
            auto txn2 = std::move(txn);  // move constructor of write txn
            CAC(txn);
            CAC(txn2);
        }
        {
            UT_LOG() << "Case 2";
            auto txn3 = db.CreateReadTxn();
            auto txn4 = std::move(txn3);  // move constructor of read txn
            CAC(txn3);
            CAC(txn4);
        }
        {
            UT_LOG() << "Case 3";
            auto txn = db.CreateWriteTxn(true);  // optimistic write
            auto txn2 = std::move(txn);
            CAC(txn);
            CAC(txn2);
        }
        {
            UT_LOG() << "Case 4";
            auto txn3 = db.CreateReadTxn();
            auto txn4 = std::move(txn3);  // move constructor of read txn
            CAC(txn3);
            CAC(txn4);
        }
        {
            UT_LOG() << "Case 5";
            auto txn = db.CreateWriteTxn(false);
            auto txn2 = std::move(txn);
            CAC(txn);
            CAC(txn2);
        }
        {
            UT_LOG() << "Case 6";
            auto txn = db.CreateWriteTxn(true);  // optimistic write
            auto txn2 = std::move(txn);
            CAC(txn);
            CAC(txn2);
        }
    }
    {
        UT_LOG() << "Testing co-existence of write transactions";
        {
            auto t0 = fma_common::GetTime();
            std::vector<std::thread> thrds;
            std::atomic<size_t> np(0);
            for (size_t i = 0; i < nt; i++) {
                thrds.emplace_back([&db, &np]() {
                    auto txn = db.CreateWriteTxn();
                    size_t i = np.fetch_add(1);
                    UT_EXPECT_EQ(i, 0);
                    SleepS(0.1);
                    np.fetch_sub(1);
                });
            }
            for (auto& thr : thrds) thr.join();
            auto t1 = fma_common::GetTime();
            UT_LOG() << t1 - t0;
        }
        {
            auto t0 = fma_common::GetTime();
            std::vector<std::thread> thrds;
            std::atomic<size_t> np(0);
            for (size_t i = 0; i < nt; i++) {
                thrds.emplace_back([&db, &np]() {
                    auto txn = db.CreateWriteTxn(false);
                    size_t i = np.fetch_add(1);
                    UT_EXPECT_EQ(i, 0);
                    SleepS(0.1);
                    np.fetch_sub(1);
                });
            }
            for (auto& thr : thrds) thr.join();
            auto t1 = fma_common::GetTime();
            UT_LOG() << t1 - t0;
        }
        {
            auto t0 = fma_common::GetTime();
            std::vector<std::thread> thrds;
            std::atomic<size_t> np(0);
            for (size_t i = 0; i < nt; i++) {
                thrds.emplace_back(
                    [&db, &np](int ti) {
                        auto txn = db.CreateWriteTxn(true);
                        size_t i = np.fetch_add(1);
                        // UT_EXPECT_EQ(i, 0);
                        UT_LOG() << ti << " got " << i;
                        SleepS(0.1);
                        np.fetch_sub(1);
                    },
                    static_cast<int>(i));
            }
            for (auto& thr : thrds) thr.join();
            auto t1 = fma_common::GetTime();
            UT_LOG() << t1 - t0;
        }
    }
    {
        UT_LOG() << "Testing co-existence of read transactions";
        auto txn = db.CreateWriteTxn();
        std::vector<std::thread> thrds;
        std::atomic<size_t> np(0);
        for (size_t i = 0; i < nt; i++) {
            thrds.emplace_back([&db, &np]() {
                auto txn = db.CreateReadTxn();
                np.fetch_add(1);
                UT_LOG() << "num read txns: " << np;
                SleepS(0.1);
                np.fetch_sub(1);
            });
        }
        for (auto& thr : thrds) thr.join();
    }
    {
        UT_LOG() << "Checking that no nested transaction is allowed";
        {
            auto txn = db.CreateWriteTxn();
            UT_EXPECT_THROW(db.CreateReadTxn(), std::exception);
            UT_EXPECT_THROW(db.CreateWriteTxn(), std::exception);
        }
        {
            auto txn = db.CreateWriteTxn(false);
            UT_EXPECT_THROW(db.CreateReadTxn(), std::exception);
            UT_EXPECT_THROW(db.CreateWriteTxn(), std::exception);
        }
        {
            auto txn = db.CreateReadTxn();
            UT_EXPECT_THROW(db.CreateReadTxn(), std::exception);
            UT_EXPECT_THROW(db.CreateWriteTxn(), std::exception);
        }
    }
}
