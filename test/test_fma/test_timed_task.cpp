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

#include "fma-common/logger.h"
#include "fma-common/timed_task.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

FMA_SET_TEST_PARAMS(TimedTask, "");

FMA_UNIT_TEST(TimedTask) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    using namespace fma_common;

    {
        TimedTaskScheduler scheduler;
        std::atomic<int> n(0);
        auto task = scheduler.ScheduleReccurringTask(100, [&n](TimedTask *) {
            FMA_LOG() << "recurring " << n;
            n++;
        });
        while (n < 10) SleepS(0);
        task->Cancel();
        scheduler.WaitTillClear();
    }
    {
        TimedTaskScheduler scheduler;
        for (size_t i = 10; i > 0; i--) {
            scheduler.RunAfterDuration(i * 100, [i](TimedTask *) { FMA_LOG() << i; });
        }
        scheduler.WaitTillClear();
        for (size_t i = 10; i > 0; i--) {
            scheduler.RunAfterDuration(i * 100, [i](TimedTask *) { FMA_LOG() << i; });
        }
    }
    {
        TimedTaskScheduler scheduler;
        std::vector<std::thread> thrs;
        std::vector<double> times(11, 0);
        for (int i = 10; i >= 0; i--) {
            thrs.emplace_back([&scheduler, i, &times]() {
                auto task = scheduler.RunAfterDuration(
                    i * 100, [i, &times](TimedTask *) { times[i] = GetTime(); });
            });
        }
        for (auto &thr : thrs) thr.join();
        scheduler.WaitTillClear();
        for (size_t i = 1; i <= 10; i++) {
            FMA_UT_CHECK_GT(times[i], times[i - 1]);
        }

        std::atomic<int> n(0);
        {
            AutoCancelTimedTask guard(scheduler.ScheduleReccurringTask(100, [&n](TimedTask *) {
                FMA_LOG() << "recurring " << n;
                n++;
            }));
            fma_common::SleepS(1);
        }
        FMA_UT_ASSERT(n >= 9 && n <= 11);
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
