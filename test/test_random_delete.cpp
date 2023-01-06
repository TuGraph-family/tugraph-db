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

#include "fma-common/utils.h"
#include "fma-common/configuration.h"
#include "gtest/gtest.h"

#include "core/defs.h"
#include "lgraph/lgraph.h"
#include "./random_port.h"
#include "./ut_utils.h"

class TestRandomDelete : public TuGraphTest {};

TEST_F(TestRandomDelete, RandomDelete) {
    using namespace fma_common;
    using namespace lgraph_api;

    std::string dir = "./db";
    int percent = 10;
    int batch_size = 2000;
    int n_threads = 1;
    int random_seed = 0;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(dir, "d,dir", true).Comment("DB directory");
    config.Add(percent, "p,percent", true)
        .Comment("Percent of vertex to delete")
        .SetMin(0)
        .SetMax(100);
    config.Add(batch_size, "b,batch", true).Comment("Batch size").SetMin(1);
    config.Add(n_threads, "n,thread", true).Comment("Number of threads").SetMin(1).SetMax(128);
    config.Add(random_seed, "s,seed", true).Comment("Random seed to use");
    config.ExitAfterHelp(true);
    config.ParseAndFinalize(argc, argv);
    UT_LOG() << "Deleting " << percent << " percent vertices from db " << dir;

    srand(random_seed);
    lgraph_api::Galaxy galaxy(dir,
                              lgraph::_detail::DEFAULT_ADMIN_NAME,
                              lgraph::_detail::DEFAULT_ADMIN_PASS,
                              false, true);
    GraphDB db = galaxy.OpenGraph("default");
    std::vector<std::thread> thrs;
    thrs.reserve(n_threads);
    size_t nv = db.EstimateNumVertices();
    size_t block_size = (nv + n_threads - 1) / n_threads;
    RandomSeed seed(0);
    for (size_t ti = 0; ti < n_threads; ti++) {
        size_t start = ti * block_size;
        size_t end = start + block_size;
        thrs.emplace_back([&, ti, start, end]() {
            auto txn = db.CreateWriteTxn(true);
            size_t current_batch = 0;
            for (size_t i = start; i < end; i++) {
                if (rand_r(&seed) % 100 >= percent) continue;
                int64_t vid = i;
                auto it = txn.GetVertexIterator(vid);
                if (it.IsValid()) {
                    it.Delete();
                    current_batch++;
                    if (current_batch >= batch_size) {
                        current_batch = 0;
                        txn.Commit();
                        txn = db.CreateWriteTxn(true);
                        UT_LOG() << "Committed a batch of size " << batch_size;
                    }
                }
            }
            txn.Commit();
        });
    }
    for (auto& thr : thrs) thr.join();
}
