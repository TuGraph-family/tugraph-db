//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\thread_pool.h.
 *
 * \brief   Declares the thread pool class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once
#include <atomic>
#include <condition_variable>
#include <functional>
#include <future>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

#include "fma-common/type_traits.h"

namespace fma_common {

/*!
 * \class   ThreadPool
 *
 * \brief   A thread pool that uses std::future to execute tasks in parallel.
 */
class ThreadPool {
 public:
    struct PriorityTask {
        PriorityTask() {}
        template <typename F>
        PriorityTask(int prio, uint64_t seq_num_, F&& f)
            : priority(prio), seq_num(seq_num_), func(std::forward<F>(f)) {}

        int priority;      // static priority for a class of tasks, lower value
                           // has higher priority
        uint64_t seq_num;  // dynamic priority, lower sequence number
                           // have higher priority
        std::function<void()> func;

        bool operator<(const PriorityTask& rhs) const {
            return priority > rhs.priority || (priority == rhs.priority && seq_num > rhs.seq_num);
        }
    };

 private:
    std::mutex mutex_;
    std::condition_variable cv_;
    std::atomic<bool> stopping_;
    std::priority_queue<PriorityTask> queue_;
    std::vector<std::thread> threads_;

 public:
    DISABLE_COPY(ThreadPool);

    /*!
     * \fn  ThreadPool::ThreadPool(size_t n_threads)
     *
     * \brief   Constructor.
     *
     * \param   n_threads   Number of worker threads.
     */
    explicit ThreadPool(size_t n_threads) : stopping_(false) {
        for (size_t i = 0; i < n_threads; i++) {
            threads_.emplace_back([this]() {
                PriorityTask input;
                while (true) {
                    {
                        std::unique_lock<std::mutex> l(mutex_);
                        cv_.wait(l, [this]() { return !queue_.empty() || stopping_; });
                        if (stopping_ && queue_.empty()) {
                            return;
                        } else {
                            input = std::move(queue_.top());
                            queue_.pop();
                        }
                    }
                    input.func();
                }
            });
        }
    }

    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> l(mutex_);
            stopping_ = true;
            cv_.notify_all();
        }
        for (auto& thread : threads_) {
            thread.join();
        }
    }

    /*!
     * \fn  template<typename Func> void ThreadPool::PushTask(int priority, uint64_t seq_num, Func&&
     * func)
     *
     * \brief   Pushes a task.
     *
     * \tparam  Func    Type of the function.
     *
     * \param           priority    The priority, lower value has higher priority
     * \param           seq_num     The sequence number, lower value has higher priority
     * \param [in,out]  func        The function.
     */
    template <typename Func>
    void PushTask(int priority, uint64_t seq_num, Func&& func) {
        std::lock_guard<std::mutex> l(mutex_);
        queue_.emplace(priority, seq_num, std::forward<Func>(func));
        cv_.notify_one();
    }
};
}  // namespace fma_common
