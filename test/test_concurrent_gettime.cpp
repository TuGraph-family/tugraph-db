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

#include <atomic>
#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "./ut_utils.h"
#include "gtest/gtest.h"

class TestConcurrentGetTime : public TuGraphTest {};

TEST_F(TestConcurrentGetTime, ConcurrentGetTime) {
    using namespace std;
    using namespace fma_common;
    size_t n_threads = 40;
    size_t n_task_per_thread = 1000;

    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(n_threads, "threads", true)
        .SetMin(1)
        .SetMax(1024)
        .Comment("Number of concurrent threads");
    config.Add(n_task_per_thread, "tasks_per_thread", true)
        .SetMin(1)
        .Comment("Number of tasks per thread");
    config.ExitAfterHelp(true);
    config.ParseAndFinalize(argc, argv);

    UT_EXPECT_GE(n_threads, 1);
    UT_EXPECT_LE(n_threads, 1024);
    UT_EXPECT_GE(n_task_per_thread, 1);

    double t1 = GetTime();
    typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
    std::vector<std::thread> thrs;
    std::vector<TimePoint> sums(n_threads);
    for (size_t i = 0; i < n_threads; i++) {
        thrs.emplace_back([i, n_task_per_thread, &sums]() {
            TimePoint t;
            for (size_t i = 0; i < n_task_per_thread; i++) {
                t += std::chrono::steady_clock::now().time_since_epoch();
            }
            sums[i] = t;
        });
    }
    for (auto& t : thrs) t.join();
    double t2 = GetTime();
    TimePoint s;
    for (auto& sum : sums) s += sum.time_since_epoch();
    UT_LOG() << s.time_since_epoch().count();
    UT_LOG() << "Finished at " << (double)n_threads * n_task_per_thread / (t2 - t1) << " OPS";
}
