//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <chrono>
#include <condition_variable>
#include <map>
#include <mutex>
#include <thread>

#include "tools/lgraph_log.h"
#include "fma-common/type_traits.h"

namespace fma_common {
typedef std::chrono::time_point<std::chrono::steady_clock> TimePoint;
typedef std::chrono::milliseconds MilliSecond;

class TimedTask {
 protected:
    friend class TimedTaskScheduler;
    mutable std::mutex lock_;
    TimePoint tp_;
    std::function<void(TimedTask*)> func_;
    bool executed_ = false;

    virtual void Run(const std::shared_ptr<TimedTask>& self) {
        std::lock_guard<std::mutex> l(lock_);
        if (func_) func_(this);
        executed_ = true;
    }

 public:
    virtual ~TimedTask() {}

    template <typename FuncT>
    TimedTask(const TimePoint& tp, FuncT&& func) : tp_(tp), func_(std::forward<FuncT>(func)) {}

    /** Cancels this object */
    virtual void Cancel() {
        std::lock_guard<std::mutex> l(lock_);
        func_ = nullptr;
    }

    const TimePoint& ScheduledTimeNoLock() const { return tp_; }

    /**
     * The next scheduled time. This can change for RecurringTask.
     *
     * @return  A TimePoint.
     */
    TimePoint ScheduledTime() const {
        std::lock_guard<std::mutex> l(lock_);
        return tp_;
    }

    /**
     * Whether the task has been executed, this may not be accurate for derived class,
     * e.g., RecurringTask.
     *
     * @return  True if task has been executed, otherwise false.
     */
    bool Executed() const {
        std::lock_guard<std::mutex> l(lock_);
        return executed_;
    }
};

class TimedTaskScheduler;
//-----------------------------------
// Recurring task runs at a constant interval
class RecurringTask : public TimedTask {
 protected:
    friend class TimedTaskScheduler;
    TimedTaskScheduler* scheduler_;
    MilliSecond interval_;

 public:
    template <typename FuncT>
    RecurringTask(TimedTaskScheduler* scheduler, MilliSecond interval, FuncT&& func)
        : TimedTask(std::chrono::steady_clock::now(), std::forward<FuncT>(func)),
          scheduler_(scheduler),
          interval_(interval) {}

 protected:
    inline void Run(const std::shared_ptr<TimedTask>& self) override;
};

class TimedTaskScheduler {
 public:
    typedef std::shared_ptr<TimedTask> TaskPtr;

 private:
    struct TaskPtrCmp {
        bool operator()(const TaskPtr& lhs, const TaskPtr& rhs) const {
            return lhs->ScheduledTimeNoLock() > rhs->ScheduledTimeNoLock();
        }
    };

    std::vector<std::thread> workers_;
    std::mutex lock_;
    std::condition_variable cond_;
    std::priority_queue<TaskPtr, std::vector<TaskPtr>, TaskPtrCmp> tasks_;
    size_t n_running_ = 0;
    bool exit_flag_ = false;

    DISABLE_COPY(TimedTaskScheduler);

 public:
    static TimedTaskScheduler& GetInstance() {
        static TimedTaskScheduler scheduler;
        return scheduler;
    }

    TimedTaskScheduler() {
        workers_.emplace_back([this]() { TaskRunnerThread(); });
    }

    ~TimedTaskScheduler() {
        {
            std::lock_guard<std::mutex> l(lock_);
            exit_flag_ = true;
        }
        CancelAll();
        cond_.notify_all();
        for (auto& w : workers_) w.join();
    }

    /** Wait till all tasks have been run */
    void WaitTillClear() {
        std::unique_lock<std::mutex> l(lock_);
        while (n_running_ || !tasks_.empty()) cond_.wait(l);
    }

    /**
     * Schedules a task. Typically used by recurring tasks.
     *
     * @param [in,out] task The task.
     */
    void ScheduleTask(const std::shared_ptr<TimedTask>& task) {
        std::lock_guard<std::mutex> l(lock_);
        if (exit_flag_) return;
        tasks_.push(task);
        cond_.notify_all();
        // if too many tasks are executing, we should spawn more threads to avoid deadlock
        if (n_running_ >= workers_.size()) workers_.emplace_back([this]() { TaskRunnerThread(); });
    }

    /**
     * Executes func at time_point tp
     *
     * @tparam  FuncT   Type of the function t.
     * @param           tp      The TP.
     * @param [in,out]  func    The function.
     *
     * @return  A TaskPtr.
     */
    template <typename FuncT>
    TaskPtr RunAtTimePoint(const TimePoint& tp, FuncT&& func) {
        TaskPtr task = std::make_shared<TimedTask>(tp, std::forward<FuncT>(func));
        ScheduleTask(task);
        return task;
    }

    /**
     * Executes func after a time duration specified in milliseconds.
     *
     * @tparam  FuncT   Type of the function t.
     * @param           milliseconds    The duration in milliseconds.
     * @param [in,out]  func            The function.
     *
     * @return  A TaskPtr.
     */
    template <typename FuncT>
    TaskPtr RunAfterDuration(const size_t milliseconds, FuncT&& func) {
        TaskPtr task = std::make_shared<TimedTask>(
            std::chrono::steady_clock::now() + MilliSecond(milliseconds),
            std::forward<FuncT>(func));
        ScheduleTask(task);
        return task;
    }

    /**
     * Executes func now and run it every interval_milliseconds
     *
     * @tparam  FuncT   Type of the function t.
     * @param           interval_milliseconds   The interval in milliseconds.
     * @param [in,out]  func                    The function.
     *
     * @return  A TaskPtr.
     */
    template <typename FuncT>
    TaskPtr ScheduleReccurringTask(const size_t interval_milliseconds, FuncT&& func) {
        TaskPtr task = std::make_shared<RecurringTask>(this, MilliSecond(interval_milliseconds),
                                                       std::forward<FuncT>(func));
        ScheduleTask(task);
        return task;
    }

    void CancelAll() {
        std::lock_guard<std::mutex> l(lock_);
        while (!tasks_.empty()) tasks_.pop();
        cond_.notify_all();
    }

 protected:
    void TaskRunnerThread() {
        std::unique_lock<std::mutex> l(lock_);
        while (true) {
            while (tasks_.empty() && !exit_flag_) cond_.wait(l);
            // check exit flag
            if (exit_flag_) return;
            // peek first task
            TaskPtr first_task = tasks_.top();
            TimePoint now = std::chrono::steady_clock::now();
            if (first_task->ScheduledTimeNoLock() <= now) {
                // run task
                tasks_.pop();
                n_running_++;
                // need to release lock here because task->Run() may schedule another task here,
                // which requires locking.
                l.unlock();
                try {
                    first_task->Run(first_task);
                } catch (std::exception& e) {
                    LOG_WARN() << "Error executing timed task: " << e.what();
                }
                l.lock();
                n_running_--;
                // notify in case someone is waiting for clear
                cond_.notify_all();
            } else {
                // sleep until next tp or woken up by new tasks
                auto diff = first_task->ScheduledTimeNoLock() - now;
                cond_.wait_for(l, diff);
            }
        }
    }
};

inline void RecurringTask::Run(const std::shared_ptr<TimedTask>& self) {
    // this is executed by the worker thread only, when the task is not in priority queue,
    // so it is ok to modify tp_ which is used to sort tasks
    // It is also ok to schedule this task for next run because it is already popped from
    // priority queue and the lock on task scheduler is released when running tasks.
    std::unique_lock<std::mutex> l(lock_);
    if (!func_) return;
    auto func = func_;
    l.unlock();
    try {
        func(this);
    } catch (std::exception& e) {
        LOG_WARN() << "Error executing recurring task: " << e.what();
    }
    l.lock();
    executed_ = true;
    // schedule next occurrance
    tp_ += interval_;
    scheduler_->ScheduleTask(self);
}

class AutoCancelTimedTask {
    TimedTaskScheduler::TaskPtr task_;
    DISABLE_COPY(AutoCancelTimedTask);

 public:
    AutoCancelTimedTask() {}

    explicit AutoCancelTimedTask(const TimedTaskScheduler::TaskPtr& task) : task_(task) {}

    ~AutoCancelTimedTask() {
        if (task_) task_->Cancel();
    }

    void AssignTask(const TimedTaskScheduler::TaskPtr& task) { task_ = task; }

    void Cancel() { task_->Cancel(); }
};
}  // namespace fma_common
