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

#include <condition_variable>
#include <mutex>
#include "fma-common/configuration.h"
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "core/transaction.h"
#include "./ut_utils.h"

class TestTxnFork : public TuGraphTest {};

TEST_F(TestTxnFork, Fork) {
    using namespace lgraph;
    using namespace fma_common;

    size_t n_txn = 10;
    size_t nv_per_txn = 100;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(n_txn, "n", true).Comment("Number of forked transactions.");
    config.ParseAndFinalize(argc, argv);
    DBConfig db_config;
    db_config.dir = "./testdb";
    LightningGraph db(db_config);
    db.DropAllData();
    db.AddLabel("v", {FieldSpec("id", FieldType::STRING, false)}, true, VertexOptions("id"));
    {
        auto txn = db.CreateWriteTxn();
        txn.AddVertex(std::string("v"), std::vector<std::string>({"id"}),
                      std::vector<std::string>({"first"}));
        txn.Commit();
    }
    size_t s = 0;
    for (size_t i = 0; i < n_txn; i++) {
        Transaction ro_txn = db.CreateReadTxn();
        // fork transaction to another thread and try to read
        bool forked = false;
        std::mutex mu;
        std::condition_variable cv;
        std::thread reader([&]() {
            auto txn = db.ForkTxn(ro_txn);
            {
                std::lock_guard<std::mutex> l(mu);
                forked = true;
            }
            cv.notify_all();
            for (auto it = txn.GetVertexIterator(); it.IsValid(); it.Next()) {
                s++;
            }
            UT_LOG() << "s=" << s;
        });
        {
            std::unique_lock<std::mutex> l(mu);
            while (!forked) cv.wait(l);
        }
        ro_txn.Abort();
        auto txn = db.CreateWriteTxn();
        for (size_t j = 0; j < nv_per_txn; j++) {
            txn.AddVertex(std::string("v"), std::vector<std::string>({"id"}),
                          std::vector<std::string>({std::to_string(i) + std::to_string(j)}));
        }
        txn.Commit();
        reader.join();
    }
}
