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
#include <chrono>
#include <iostream>
#include <string>

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/thread_pool.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

FMA_SET_TEST_PARAMS(ThreadPool, "", "--nTasks 10 --nThreads 12 --nIter 100",
                    "--nTasks 100 --nThreads 7 --sleepTime 0.01 --nIter 30");

FMA_UNIT_TEST(ThreadPool) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    int n_tasks = 10;
    int n_threads = 3;
    double sleep_time = 0.2;
    int n_iter = 10;
    Configuration config;
    config.Add(n_tasks, "nTasks", true).Comment("Number of elements to push into the queue");
    config.Add(n_threads, "nThreads", true).Comment("Number of threads to use.");
    config.Add(sleep_time, "sleepTime", true).Comment("Time to sleep for each task");
    config.Add(n_iter, "nIter", true).Comment("Number of iterations to run in performance test");
    config.Parse(argc, argv);
    config.Finalize();

    FMA_LOG() << "Using " << n_threads << " threads to process " << n_tasks << " elements";

    std::vector<int> seq(n_tasks, 0);
    std::atomic<int> curr(0);
    double t1 = GetTime();
    {
        ThreadPool pool(n_threads);
        for (int i = 0; i < n_tasks; i++) {
            pool.PushTask(0, n_tasks - i, [&curr, &seq, i, sleep_time]() {
                int my_seq = curr++;
                seq[i] = my_seq;
                SleepS(sleep_time);
                FMA_LOG() << i << ", seqNo " << my_seq << " sleeping for " << sleep_time << "s";
            });
        }
        while (curr != n_tasks) SleepUs(1);
    }
    double t2 = GetTime();
    double et = t2 - t1;
    FMA_UT_ASSERT(et < 1.0  // if time is very small, then timer can be off
               || (et < (n_tasks / n_threads + 2) * sleep_time * 1.1 &&
                   et > (n_tasks / n_threads) * sleep_time * 0.9))
        << "Time used to execute is siganificantly off: " << et;
    // now test performace
    size_t total_tasks = n_iter * n_tasks;
    t1 = GetTime();
    {
        std::atomic<size_t> n_run(0);
        ThreadPool pool(n_threads);
        for (int i = 0; i < n_iter; i++) {
            for (int j = 0; j < n_tasks; j++) {
                int myid = i * n_tasks + j;
                pool.PushTask(0, myid, [&n_run, myid]() { n_run++; });
            }
        }
        double long_time = std::max<double>((double)total_tasks / 5000, 1.0);
        while (n_run < total_tasks) {
            double et = GetTime() - t1;
            if (et > long_time) {
                FMA_ERR() << "Performance test is taking too long to execute: " << et
                          << "s, aborting";
            }
            SleepUs(1);
        }
    }
    t2 = GetTime();
    FMA_LOG() << "Processed " << total_tasks << " tasks at " << (double)total_tasks / (t2 - t1)
              << "tps";

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
