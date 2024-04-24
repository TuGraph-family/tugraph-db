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

#pragma once

#include <atomic>
#include <chrono>
#include <exception>
#include <mutex>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "fma-common/string_formatter.h"
#include "fma-common/text_parser.h"
#include "fma-common/timed_task.h"
#include "fma-common/utils.h"

#include "core/data_type.h"
#include "core/cache_aligned_vector.h"
#include "core/thread_id.h"

#include "lgraph/lgraph_exceptions.h"

namespace lgraph {

class TaskTracker {
    typedef std::chrono::system_clock::time_point TimePoint;
    typedef std::chrono::system_clock SystemClock;

 public:
    // interface, statistics
    struct Stats {
        double qps;
        double tps;
        double failure_rate;
        size_t n_running;
    };

    enum ErrorCode { SUCCESS = 0, NOTFOUND = 1, FAIL_TO_KILL = 2 };

    // interface, identifies a task
    struct TaskId {
        TaskId() : thread_id(-1), task_id(-1) {}
        TaskId(int thr, int64_t tsk) : thread_id(thr), task_id(tsk) {}

        std::string ToString() const {
            return fma_common::StringFormatter::Format("{}_{}", thread_id, task_id);
        }

        bool FromString(const std::string& str) {
            const char* b = str.data();
            const char* e = str.data() + str.size();
            size_t s1 = fma_common::TextParserUtils::ParseT(b, e, thread_id);
            b += s1;
            if (s1 == 0 || b >= e || *b != '_') return false;
            b++;
            size_t s2 = fma_common::TextParserUtils::ParseT(b, e, task_id);
            return (s2 != 0 && b + s2 == e);
        }

        int thread_id;
        int64_t task_id;
    };

    // interface, describes a task
    struct TaskDesc {
        void SetTaskDesc(int64_t task, const std::shared_ptr<std::string>& des,
                         TimePoint start_time) {
            id.task_id = task;
            if (des) desc = *des;
            time_elpased = (double)std::chrono::duration_cast<std::chrono::microseconds>(
                               SystemClock::now() - start_time)
                               .count() /
                           1000000;
        }

        TaskId id;
        std::string desc;
        double time_elpased;
    };

    template <typename T>
    static void Store(std::atomic<T>& at, const T& d) {
        at.store(d, std::memory_order_release);
    }

    template <typename T>
    static T Load(const std::atomic<T>& at) {
        return at.load(std::memory_order_acquire);
    }

    template <typename T>
    static T Inc(std::atomic<T>& at) {
        return at.fetch_add(1, std::memory_order_acq_rel);
    }

    template <typename T>
    static T FetchClear(std::atomic<T>& at) {
        return at.exchange(0, std::memory_order_acq_rel);
    }

    struct ThreadContext {
     private:
        // current task descriptions
        std::atomic<int64_t> current_task_id;  // set by worker thread to indicate current task id
        std::atomic<int64_t> kill_task_id;     // set by killer thread to kill task
        std::atomic<TimePoint> start_time;
        std::shared_ptr<std::string> task_desc;  // current task description
        std::atomic<bool> task_running;
        bool curr_task_failed = false;
        // statistics
        std::atomic<size_t> requests;    // # requests since last epoch
        std::atomic<size_t> writes;      // # write requests since last epoch
        std::atomic<size_t> long_tasks;  // # long tasks (plugin/cypher) since last epoch
        std::atomic<size_t> failures;    // # failed requests since last epoch

     public:
        ThreadContext()
            : current_task_id(0),
              kill_task_id(0),
              start_time(SystemClock::now()),
              task_desc(nullptr),
              task_running(false),
              curr_task_failed(false),
              requests(0),
              writes(0),
              long_tasks(0),
              failures(0) {}

        void Init() {
            current_task_id = 0;
            kill_task_id = 0;
            start_time = SystemClock::now();
            task_running = false;
            requests = 0;
            writes = 0;
            long_tasks = 0;
            failures = 0;
        }

        int64_t BeginTask(const std::string& desc, bool is_long, bool is_write) {
            std::atomic_store(&task_desc, std::make_shared<std::string>(desc));
            return BeginTask(is_long, is_write);
        }

        int64_t BeginTask(bool is_long, bool is_write) {
            Store(kill_task_id, (int64_t)-1);
            Store(start_time, SystemClock::now());
            Store(task_running, true);
            // by default, mark task as fail
            // we will always remember to mark it as success when we finish, but we might miss it
            // when there is exception
            curr_task_failed = true;
            Inc(requests);
            if (is_write) Inc(writes);
            if (is_long) Inc(long_tasks);
            return Inc(current_task_id);
        }

        void MarkTaskFail() { curr_task_failed = true; }

        void MarkTaskSuccess() { curr_task_failed = false; }

        void EndTask() {
            Store(task_running, false);
            if (curr_task_failed) Inc(failures);
        }

        void EndTask(bool failed) {
            Store(task_running, false);
            if (failed) Inc(failures);
        }

        void CollectStats(size_t& total_requests, size_t& total_writes, size_t& total_long_tasks,
                          size_t& total_failures, size_t& total_running) {
            total_requests += FetchClear(requests);
            total_writes += FetchClear(writes);
            total_long_tasks += FetchClear(long_tasks);
            total_failures += FetchClear(failures);
            total_running += Load(task_running);
        }

        ErrorCode KillTask(int64_t task_id) {
            if (!Load(task_running) || Load(current_task_id) != task_id) {
                return ErrorCode::NOTFOUND;
            }
            Store(kill_task_id, task_id);
            for (size_t i = 0; i < 100; i++) {
                if (!Load(task_running) || Load(current_task_id) != task_id)
                    return ErrorCode::SUCCESS;
                fma_common::SleepUs(10000);
            }
            return ErrorCode::FAIL_TO_KILL;
        }

        bool ShouldKillTask() const {
            return Load(task_running) && Load(kill_task_id) == Load(current_task_id);
        }

        bool GetTaskDesc(TaskDesc& td) const {
            if (!Load(task_running)) return false;
            td.SetTaskDesc(Load(current_task_id),
                           std::atomic_load_explicit(&task_desc, std::memory_order_acquire),
                           Load(start_time));
            return true;
        }

        int64_t GetCurrentTaskId() const { return Load(current_task_id); }
    };

 private:
    // ------------------------
    // data member and private constructor

    // thread contexts
    StaticCacheAlignedVector<ThreadContext, LGRAPH_MAX_THREADS> contexts_;
    // statistics
    std::atomic<double> qps_;
    std::atomic<double> tps_;
    std::atomic<double> failure_rate_;
    std::atomic<size_t> running_;
    std::atomic<size_t> long_tasks_;
    // used by statistics
    std::mutex update_lock_;  // must get this lock before updating stats
    int64_t max_refresh_interval_ms_ = 10 * 1000;
    int64_t min_refresh_interval_ms_ = 1000;
    CacheAlignedObject<std::atomic<TimePoint>>
        last_epoch_time_;  // when were counters last cleared?
    CacheAlignedObject<std::atomic<TimePoint>> last_query_time_;  // when was stats last queried?
    fma_common::TimedTaskScheduler::TaskPtr task_;

    TaskTracker(int64_t max_refresh_interval_ms = 10 * 1000,
                int64_t min_refresh_interval_ms = 500) {
        Reset(max_refresh_interval_ms, min_refresh_interval_ms);
    }

    int64_t MsSinceLastQuery(const TimePoint& now) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                     Load(last_query_time_.data))
            .count();
    }

    int64_t MsSinceLastEpoch(const TimePoint& now) {
        return std::chrono::duration_cast<std::chrono::milliseconds>(now -
                                                                     Load(last_epoch_time_.data))
            .count();
    }

    void RefreshStats() {
        std::lock_guard<std::mutex> l(update_lock_);
        size_t requests = 0;
        size_t writes = 0;
        size_t failures = 0;
        size_t long_tasks = 0;
        size_t running = 0;
        for (size_t i = 0; i < contexts_.size(); i++) {
            auto& ctx = contexts_[i];
            ctx.CollectStats(requests, writes, long_tasks, failures, running);
        }
        TimePoint now = SystemClock::now();
        double seconds = std::max<double>((double)MsSinceLastEpoch(now) / 1000, 0.1);
        Store(last_epoch_time_.data, now);
        Store(qps_, (double)requests / seconds);
        Store(tps_, (double)writes / seconds);
        if (requests == 0)
            Store(failure_rate_, 0.0);
        else
            Store(failure_rate_, (double)failures / requests);
        Store(running_, running);
        Store(long_tasks_, long_tasks);
    }

 public:
    ~TaskTracker() {
        if (task_) task_->Cancel();
    }

    static TaskTracker& GetInstance() {
        static TaskTracker tracker;
        return tracker;
    }

    // get statistics
    // stats are obtained per refresh_interval
    // if a GetStats request comes between interval, it helps refresh the stats
    Stats GetStats() {
        // only refresh if (curr_time - last_query_time_) < refresh_interval
        TimePoint curr_time = SystemClock::now();
        int64_t since_last_query = MsSinceLastQuery(curr_time);
        int64_t since_last_epoch = MsSinceLastEpoch(curr_time);
        if (since_last_query < max_refresh_interval_ms_  // we are frequentyly refreshing
            && since_last_epoch >
                   min_refresh_interval_ms_) {  // leave enough time for acurate statistics
            RefreshStats();
            Store(last_query_time_.data, SystemClock::now());
        } else {
            Store(last_query_time_.data, curr_time);
        }
        // just get current stat
        Stats ret;
        ret.qps = Load(qps_);
        ret.tps = Load(tps_);
        ret.n_running = Load(running_);
        ret.failure_rate = Load(failure_rate_);
        return ret;
    }

    static int64_t MarkTaskBegin(ThreadContext* ctx, const std::string& task_desc, bool is_long,
                                 bool is_write) {
        return ctx->BeginTask(task_desc, is_long, is_write);
    }

    static int64_t MarkTaskBegin(ThreadContext* ctx, bool is_long, bool is_write) {
        return ctx->BeginTask(is_long, is_write);
    }

    void MarkTaskFail() { MarkTaskFail(GetThreadContext()); }

    static void MarkTaskFail(ThreadContext* ctx) { ctx->MarkTaskFail(); }

    void MarkTaskSuccess() { MarkTaskSuccess(GetThreadContext()); }

    static void MarkTaskSuccess(ThreadContext* ctx) { ctx->MarkTaskSuccess(); }

    static void MarkTaskEnd(ThreadContext* ctx) { ctx->EndTask(); }

    /**
     * Called by worker thread, determine if we should kill current task
     *
     * @return  True if it succeeds, false if it fails.
     */
    bool ShouldKillCurrentTask() { return ShouldKillTask(GetThreadContext()); }

    static bool ShouldKillTask(const ThreadContext* ctx) { return ctx->ShouldKillTask(); }

    /**
     * List running tasks
     *
     * @return  A vector of TaskDesc
     */
    std::vector<TaskDesc> ListRunningTasks() {
        std::vector<TaskDesc> ret;
        for (size_t i = 0; i < contexts_.size(); i++) {
            const ThreadContext& ctx = contexts_[i];
            TaskDesc td;
            td.id.thread_id = static_cast<int>(i);
            if (ctx.GetTaskDesc(td)) ret.emplace_back(std::move(td));
        }
        return ret;
    }

    void KillAllTasks() {
        for (size_t i = 0; i < contexts_.size(); i++) {
            ThreadContext& ctx = contexts_[i];
            ctx.KillTask(ctx.GetCurrentTaskId());
        }
    }

    /**
     * Kill a task identified by TaskId
     *
     * @param task  The task.
     */
    ErrorCode KillTask(const TaskId& task) {
        if (task.thread_id >= LGRAPH_MAX_THREADS) {
            return ErrorCode::NOTFOUND;
        }
        return KillTask(&contexts_[task.thread_id], task.task_id);
    }

    static ErrorCode KillTask(ThreadContext* ctx, int64_t task_id) {
        return ctx->KillTask(task_id);
    }

    /**
     * Gets thread context
     *
     * @return  The thread context.
     */
    ThreadContext* GetThreadContext() { return &contexts_[GetMyThreadId()]; }

    TaskId GetCurrentTaskId() const {
        int thr = GetMyThreadId();
        return TaskId(thr, contexts_[thr].GetCurrentTaskId());
    }

    // refresh states, need this for testing, otherwise it is hard to reset
    // TaskTracker, since it is a singleton
    void Reset(int64_t max_refresh_interval_ms = 10 * 1000, int64_t min_refresh_interval_ms = 500) {
        if (task_) task_->Cancel();
        Store(qps_, (double)0);
        Store(tps_, (double)0);
        Store(failure_rate_, 0.0);
        Store(running_, (size_t)0);
        Store(long_tasks_, (size_t)0);
        max_refresh_interval_ms_ = std::max<int64_t>(max_refresh_interval_ms, 100);
        min_refresh_interval_ms_ = min_refresh_interval_ms;
        auto last_time = SystemClock::now() - std::chrono::milliseconds(max_refresh_interval_ms);
        // init last_query_time and last_epoch_time to make sure we get correct stats at the first
        // query
        Store(last_query_time_.data, last_time);
        Store(last_epoch_time_.data, last_time);
        // init thread contexts
        for (size_t i = 0; i < contexts_.size(); i++) {
            ThreadContext& ctx = contexts_[i];
            ctx.Init();
        }
        // start refreshing stats
        task_ = fma_common::TimedTaskScheduler::GetInstance().ScheduleReccurringTask(
            max_refresh_interval_ms_, [this](fma_common::TimedTask*) { RefreshStats(); });
    }
};

class AutoTaskTracker {
    TaskTracker::ThreadContext* thread_context_;
    int64_t task_id_;

 public:
    AutoTaskTracker(const std::string& desc, bool is_long, bool is_write) {
        thread_context_ = TaskTracker::GetInstance().GetThreadContext();
        task_id_ = thread_context_->BeginTask(desc, is_long, is_write);
    }

    AutoTaskTracker(bool is_long, bool is_write) {
        thread_context_ = TaskTracker::GetInstance().GetThreadContext();
        task_id_ = thread_context_->BeginTask(is_long, is_write);
    }

    ~AutoTaskTracker() { thread_context_->EndTask(); }

    bool ShouldKillThisTask() const { return TaskTracker::ShouldKillTask(thread_context_); }

    void Cancel() { TaskTracker::KillTask(thread_context_, task_id_); }

    void MarkSuccess() { thread_context_->MarkTaskSuccess(); }

    void MarkFail() { thread_context_->MarkTaskFail(); }

    TaskTracker::TaskId GetTaskId() const { return TaskTracker::TaskId(GetMyThreadId(), task_id_); }
};

inline void ThrowIfTaskKilled() {
    if (TaskTracker::GetInstance().ShouldKillCurrentTask()) {
        LOG_WARN() << "Task killed";
        THROW_CODE(TaskKilled);
    }
}

class TimeoutTaskKiller {
    fma_common::TimedTaskScheduler::TaskPtr task_;

 public:
    void SetTimeout(double timeout_s) {
        auto& scheduler = fma_common::TimedTaskScheduler::GetInstance();
        lgraph::TaskTracker::TaskId task_id = lgraph::TaskTracker::GetInstance().GetCurrentTaskId();
        task_ = scheduler.RunAfterDuration((size_t)(timeout_s * 1000),
                                           [task_id](fma_common::TimedTask*) {
                                               lgraph::TaskTracker::GetInstance().KillTask(task_id);
                                           });
    }

    ~TimeoutTaskKiller() {
        if (task_) task_->Cancel();
    }
};
}  // namespace lgraph
