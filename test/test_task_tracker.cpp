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

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "gtest/gtest.h"

#include "core/task_tracker.h"
#include "./random_port.h"
#include "./ut_utils.h"

class TestTaskTracker : public TuGraphTest {};

class Semanphore {
    size_t curr_value_ = 0;
    std::mutex m_;
    std::condition_variable cond_;

 public:
    Semanphore() : curr_value_(0) {}

    void WaitTillValueBecomes(size_t value) {
        std::unique_lock<std::mutex> l(m_);
        while (curr_value_ < value) cond_.wait(l);
    }

    void Inc() {
        std::unique_lock<std::mutex> l(m_);
        curr_value_++;
        cond_.notify_all();
    }
};

TEST_F(TestTaskTracker, TaskTracker) {
    using namespace fma_common;
    using namespace lgraph;

    size_t n_threads = 20;
    size_t n_task_per_thread = 1000;
    size_t n_seconds = 10;
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
    config.Add(n_seconds, "seconds", true).SetMin(1).Comment("Number of seconds to check stats");
    config.ExitAfterHelp(true);
    config.ParseAndFinalize(argc, argv);
    {
        TaskTracker::GetInstance().Reset(1000, 500);
        TimedTaskScheduler::GetInstance().CancelAll();
        std::vector<std::thread> thrs;
        Semanphore sem1, sem2;
        for (size_t i = 0; i < n_threads; i++) {
            thrs.emplace_back([i, n_threads, &sem1, &sem2]() {
                UT_LOG() << "beg: " << i << "get thread id " << GetMyThreadId();
                sem1.Inc();
                sem1.WaitTillValueBecomes(n_threads + 1);  // wait for master
                AutoTaskTracker tt(std::to_string(i), true, true);
                UT_LOG() << "mid: " << i << "has thread id " << GetMyThreadId();
                sem2.Inc();
                sem2.WaitTillValueBecomes(n_threads + 1);
                UT_LOG() << "end: " << i << "has thread id " << GetMyThreadId();
            });
        }
        TaskTracker::Stats stats = TaskTracker::GetInstance().GetStats();
        sem1.WaitTillValueBecomes(n_threads);
        // first try, everything is 0
        UT_EXPECT_EQ(stats.n_running, 0);
        UT_EXPECT_EQ(stats.qps, 0);
        UT_EXPECT_EQ(stats.tps, 0);
        UT_EXPECT_EQ(stats.failure_rate, 0);
        // now start execution
        sem1.Inc();                            // start worker execution
        fma_common::SleepS(0.6);               // wait out min_refresh_interval
        sem2.WaitTillValueBecomes(n_threads);  // wait till all tasks have started
        stats = TaskTracker::GetInstance().GetStats();
        UT_EXPECT_EQ(stats.n_running, n_threads);
        UT_LOG() << "qps: " << stats.qps;
        UT_LOG() << "tps: " << stats.tps;
        UT_LOG() << "failure_rate: " << stats.failure_rate;
        sem2.Inc();  // finish tasks
        // wait till end
        for (auto& thr : thrs) thr.join();
        fma_common::SleepS(0.6);  // wait out min_refresh_interval
        stats = TaskTracker::GetInstance().GetStats();
        UT_EXPECT_EQ(stats.n_running, 0);
    }

    {
        double t1 = GetTime();
        std::vector<std::thread> thrs;
        std::vector<int64_t> sums(n_threads);
        for (size_t i = 0; i < n_threads; i++) {
            thrs.emplace_back([i, n_task_per_thread, &sums]() {
                int64_t t = 0;
                RandomSeed seed(0);
                for (size_t i = 0; i < n_task_per_thread; i++) {
                    AutoTaskTracker tt("task", true, true);
                    t += rand_r(&seed);
                }
                sums[i] = t;
            });
        }
        auto tasks = TaskTracker::GetInstance().ListRunningTasks();
        for (auto& t : tasks) {
            UT_LOG() << "Task(" << t.id.ToString() << ", ts=" << t.time_elpased << ")";
            UT_EXPECT_EQ(t.desc, "task");
        }
        for (auto& t : thrs) t.join();
        double t2 = GetTime();
        int64_t s = 0;
        for (auto& sum : sums) s += sum;
        UT_LOG() << s;
        UT_LOG() << "Finished at " << (double)n_threads * n_task_per_thread / (t2 - t1) << " OPS";
    }
    {
        std::vector<std::thread> thrs;
        std::vector<std::atomic<bool>> interrupted(n_threads);
        std::vector<std::atomic<int>> thrd_ids(n_threads);
        for (auto& i : interrupted) i = false;
        Semanphore tasks_registered, start_tasks, task1_finished, register_task2, task2_registered,
            start_task2;
        TaskTracker::GetInstance().Reset();
        for (size_t i = 0; i < n_threads; i++) {
            thrs.emplace_back([n_threads, i, &interrupted, &thrd_ids, &tasks_registered,
                               &start_tasks, &task1_finished, &register_task2,
                               &task2_registered]() {
                {
                    AutoTaskTracker t("task", true, true);
                    thrd_ids[i] = GetMyThreadId();
                    tasks_registered.Inc();
                    UT_LOG() << i << " running task";
                    start_tasks.WaitTillValueBecomes(1);
                    for (size_t j = 0; j < 10; j++) {
                        fma_common::SleepS(0.1);
                        if (TaskTracker::GetInstance().ShouldKillCurrentTask()) {
                            interrupted[i] = true;
                            UT_LOG() << i << " interrupted";
                            break;
                        }
                    }
                    UT_LOG() << i << " finish task";
                }
                task1_finished.Inc();
                register_task2.WaitTillValueBecomes(1);
                {
                    AutoTaskTracker t("task2", true, false);
                    UT_LOG() << i << " running task2";
                    task2_registered.Inc();
                    for (size_t j = 0; j < 10; j++) {
                        fma_common::SleepS(0.1);
                        if (TaskTracker::GetInstance().ShouldKillCurrentTask()) {
                            interrupted[i] = true;
                            break;
                        }
                    }
                    UT_LOG() << i << " finish task2";
                }
            });
        }
        tasks_registered.WaitTillValueBecomes(n_threads);
        auto tasks = TaskTracker::GetInstance().ListRunningTasks();
        UT_EXPECT_EQ(tasks.size(), n_threads);
        for (auto& t : tasks) {
            UT_EXPECT_EQ(t.desc, "task");
            UT_EXPECT_EQ(t.id.task_id, 1);
        }
        UT_EXPECT_EQ(TaskTracker::GetInstance().KillTask(tasks[3].id), TaskTracker::FAIL_TO_KILL);
        start_tasks.Inc();
        size_t thrd_idx = 0;
        for (; thrd_idx < thrd_ids.size(); thrd_idx++) {
            if (thrd_ids[thrd_idx] == tasks[3].id.thread_id) {
                break;
            }
        }
        task1_finished.WaitTillValueBecomes(n_threads);
        for (size_t i = 0; i < interrupted.size(); i++) {
            bool v = interrupted[i];
            UT_EXPECT_EQ(v, i == thrd_idx);
        }
        std::fill(interrupted.begin(), interrupted.end(), false);
        UT_EXPECT_EQ(TaskTracker::GetInstance().KillTask(TaskTracker::TaskId(0, 2)),
                     TaskTracker::NOTFOUND);  // task id incorret, should have no effect
        register_task2.Inc();
        fma_common::SleepS(0.1);
        task2_registered.WaitTillValueBecomes(n_threads);
        tasks = TaskTracker::GetInstance().ListRunningTasks();
        UT_EXPECT_EQ(tasks.size(), n_threads);
        for (size_t i = 0; i < tasks.size(); i++) {
            auto& t = tasks[i];
            UT_EXPECT_EQ(t.id.task_id, 2);
            UT_EXPECT_TRUE(t.time_elpased >= 0.08 && t.time_elpased <= 0.12);
            UT_EXPECT_EQ(t.desc, "task2");
        }
        UT_EXPECT_EQ(tasks.size(), n_threads);
        for (auto& t : thrs) t.join();
        for (size_t i = 0; i < interrupted.size(); i++) {
            bool v = interrupted[i];
            UT_EXPECT_EQ(v, false);
        }
    }
    {
        std::vector<std::thread> thrs;
        std::vector<std::atomic<int>> interrupted(n_threads);
        for (auto& i : interrupted) i = 0;
        std::mutex m;
        std::condition_variable cond;
        bool start = false;
        TaskTracker::GetInstance().Reset();
        AutoTaskTracker tt("master task", false, true);
        const auto& ctx = TaskTracker::GetInstance().GetThreadContext();
        for (size_t i = 0; i < n_threads; i++) {
            thrs.emplace_back([n_threads, i, &interrupted, &m, &cond, &start, &ctx]() {
                UT_LOG() << GetMyThreadId() << " running task";
                {
                    std::unique_lock<std::mutex> l(m);
                    while (!start) cond.wait(l);
                }
                for (size_t j = 0; j < 10; j++) {
                    fma_common::SleepS(0.1);
                    if (TaskTracker::GetInstance().ShouldKillTask(ctx)) {
                        interrupted[i] = 1;
                        UT_LOG() << i << " interrupted";
                        return;
                    }
                }
                UT_LOG() << i << " finish task";
            });
        }
        auto tasks = TaskTracker::GetInstance().ListRunningTasks();
        UT_EXPECT_EQ(tasks.size(), 1);
        UT_EXPECT_EQ(TaskTracker::GetInstance().KillTask(tasks[0].id), TaskTracker::FAIL_TO_KILL);
        UT_EXPECT_EQ(TaskTracker::GetInstance().KillTask(TaskTracker::TaskId(1024, 0)),
                     TaskTracker::NOTFOUND);
        UT_LOG() << "Killing task " << tasks[0].id.ToString();
        {
            std::lock_guard<std::mutex> l(m);
            start = true;
            cond.notify_all();
        }
        for (auto& t : thrs) t.join();
        for (size_t i = 0; i < interrupted.size(); i++) {
            bool v = interrupted[i];
            UT_EXPECT_EQ(v, true);
        }
    }
    {
        TaskTracker::GetInstance().Reset(1000, 500);
        double time_per_task = 0.1;
        size_t n_tasks = (size_t)(n_seconds / time_per_task);
        std::vector<std::thread> thrs;
        for (size_t i = 0; i < 8; i++) {
            thrs.emplace_back([=]() {
                for (size_t j = 0; j < n_tasks; j++) {
                    AutoTaskTracker tt(j % 10 < 2, j % 10 < 5);
                    fma_common::SleepS(time_per_task);
                    if (j % 10 < 1)
                        tt.MarkFail();
                    else
                        tt.MarkSuccess();
                }
            });
        }
        double t1 = GetTime();
        while (true) {
            double t2 = GetTime();
            if (t2 - t1 >= (double)n_seconds) break;
            TaskTracker::Stats stats = TaskTracker::GetInstance().GetStats();
            UT_LOG() << "running=" << stats.n_running << ", qps=" << stats.qps
                      << ", tps=" << stats.tps << ", failure_rate=" << stats.failure_rate;
            fma_common::SleepS(0.3);
        }
        for (auto& thr : thrs) thr.join();
    }
}
