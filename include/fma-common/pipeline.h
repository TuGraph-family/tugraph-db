//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file   fma-common\pipeline.h.
 *
 * \brief   Declares the pipeline stage class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once
#ifdef _MSC_VER
#pragma warning(disable : 4503)
#endif

#include <algorithm>
#include <condition_variable>
#include <functional>
#include <limits>
#include <list>
#include <mutex>
#include <queue>
#include <type_traits>
#include <vector>

#include "fma-common/bounded_queue.h"
#include "fma-common/thread_pool.h"
#include "fma-common/utils.h"

namespace fma_common {

/*!
 * \class   PipelineStage
 *
 * \brief   We sometimes want to organize logic into a pipeline, in which
 *          some stages are sequential, some are parallel. For example,
 *          we may want to use one thread to read a file, then sent the
 *          content as data blocks to multiple threads for parallel parsing,
 *          then finally use one thread to process the data one by one.
 *          The ordering of the data blocks may or may not be important.
 *
 *          The PipelineStage template class implements a stage in the
 *          pipeline, which reads an input of type T1 and outputs a data
 *          of type T2. Multiple threads can process different parts in
 *          parallel, and the user can choose whether to keep the output
 *          order of the data as the input order.
 *
 *          Multiple PipelineStage can be connected to form a pipeline.
 *
 * \tparam  T1  Input type.
 * \tparam  T2  Output type. Can be void.
 */
template <typename T1, typename T2>
class PipelineStage : public DataConsumer<T1> {
 public:
    using FuncT = std::function<T2(T1&& t)>;
    using InputT = T1;
    using OutputT = T2;

 private:
    using UniqueLock = std::unique_lock<std::mutex>;
    using LockGuard = std::lock_guard<std::mutex>;
    typedef DUMMY_TYPE_IF_VOID(T2) OutputInstanceT;

    /*!
     * \enum    State of a task.
     *
     * \brief   When a task is added to the pipeline, it is NEW.
     *          When a thread starts processing the task, it is RUNNING.
     *          When the thread has finished processing it, it becomes
     *          FINISHED and can be removed from the pipeline.
     */
    enum State { NEW_TASK = 0, RUNNING = 1, FINISHED = 2 };

    struct TaskInstance {
        TaskInstance() = default;
        TaskInstance(const T1& t, const State& s) : input(t), state(s) {}
        TaskInstance(T1&& t, const State& s) : input(std::move(t)), state(s) {}
        T1 input;
        OutputInstanceT output;
        State state;
    };

    size_t max_active_tasks_ = 1;
    size_t input_buffer_size_ = std::numeric_limits<size_t>::max();
    FuncT func_;
    int priority_ = 0;
    uint64_t seq_num_ = 0;
    std::list<TaskInstance> input_buffer_;
    typedef typename std::list<TaskInstance>::iterator TaskIterator;
    std::mutex mutex_;
    std::condition_variable cv_ok_to_push_;
    std::condition_variable cv_stage_clear_;
    ThreadPool* thread_pool_;
    bool own_pool_ = false;
    DataConsumer<T2>* next_stage_ = nullptr;
    bool kill_switch_ = false;
    bool out_of_order_ = false;

 public:
    DISABLE_COPY(PipelineStage);

    /*!
     * \fn  PipelineStage::PipelineStage(const FuncT& func, ThreadPool* pool = nullptr, int priority
     * = 0, size_t max_active_tasks = 1, size_t input_buffer_size =
     * std::numeric_limits<size_t>::max(), bool out_of_order = std::is_void<T2>::value,
     * DataConsumer<T2>* next_stage = nullptr)
     *
     * \brief   Constructs a PipelineStage. Defines the function to process a T1 and
     *          produce a T2.
     *
     *          If a thread pool is given, the stage will use threads in the thread
     *          pool to execute the tasks. Otherwise, it creates a new thread pool.
     *
     *          Piority should be carefully designed when sharing thread pool in
     *          multiple stages to avoid deadlock. So it is usually a better choice
     *          to use seperate thread pools. Note that lower priority value means
     *          higher priority.
     *
     * \param  func                The function to process a T1 and produce a T2.
     *                             type: T2 func(const T1&)
     * \param  pool                (Optional) If non-null, the thread pool to use.
     * \param  priority            (Optional) The priority, lower value means higher
     *                             priority.
     * \param  max_active_tasks    (Optional) The maximum number of tasks that can run
     *                             in parallel.
     * \param  input_buffer_size   (Optional) Size of the input buffer. When consumer
     *                             cannot catch up with the producers, the buffer can
     *                             be filled up quickly. Limiting the input buffer will
     *                             stop the producer from pushing more tasks in.
     * \param  out_of_order        (Optional) Whether the tasks should retire in the
     *                             same order as they are pushed.
     * \param  next_stage          (Optional) If non-null, the next stage.
     */
    PipelineStage(const FuncT& func, ThreadPool* pool = nullptr, int priority = 0,
                  size_t max_active_tasks = 1,
                  size_t input_buffer_size = std::numeric_limits<size_t>::max(),
                  bool out_of_order = std::is_void<T2>::value,
                  DataConsumer<T2>* next_stage = nullptr)
        : max_active_tasks_(std::max<size_t>(max_active_tasks, 1)),
          input_buffer_size_(input_buffer_size),
          func_(func),
          priority_(priority),
          next_stage_(next_stage),
          out_of_order_(out_of_order) {
        if (pool == nullptr) {
            thread_pool_ = new ThreadPool(max_active_tasks);
            own_pool_ = true;
        } else {
            thread_pool_ = pool;
            own_pool_ = false;
        }
    }

    PipelineStage(PipelineStage&&) = delete;

    virtual ~PipelineStage() { Stop(); }

    /*!
     * \fn  void PipelineStage::SetNextStage(DataConsumer<T2>* next_stage)
     *
     * \brief   Sets the next stage. Output will be pushed to the next stage
     *          when a task finishes. SetNextStage() must be called before
     *          pushing any input.
     *
     * \param [in,out]  next_stage  The next stage.
     */
    void SetNextStage(DataConsumer<T2>* next_stage) {
        LockGuard l(mutex_);
        next_stage_ = next_stage;
    }

    /*!
     * \fn  bool PipelineStage::Push(T1&& t) override
     *
     * \brief   Pushes an input into the stage. Blocks the caller until the
     *          data is successfully pushed.
     *
     * \param [in,out]  t   Rvalue reference to the data to be pushed.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Push(T1&& t) override {
        UniqueLock l(mutex_);
        while (input_buffer_.size() >= input_buffer_size_ && !kill_switch_) {
            cv_ok_to_push_.wait(l);
        }
        if (kill_switch_) {
            LOG_WARN() << "Killing pipeline stage when there is ongoing push";
            return false;
        }
        input_buffer_.emplace_back(std::forward<T1>(t), State::NEW_TASK);
        CheckQueueNolock(input_buffer_.end());
        return true;
    }

    /*!
     * \fn  bool PipelineStage::Push(const T1& t) override
     *
     * \brief   Pushes an input into the stage. Blocks the caller until the
     *          data is successfully pushed.
     *
     * \param   t   The data to push.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Push(const T1& t) override { return Push(std::move(T1(t))); }

    /*!
     * \fn  void PipelineStage::WaitTillClear()
     *
     * \brief   Wait until all the input have been successfully processed
     *          and the output pushed to the next stage.
     */
    void WaitTillClear() {
        UniqueLock l(mutex_);
        if (kill_switch_) return;
        while (!input_buffer_.empty()) {
            cv_stage_clear_.wait(l);
        }
    }

    /**
     * Query if all the tasks have been finished.
     *
     * \return  True if clear, false if not.
     */
    bool IsClear() {
        LockGuard l(mutex_);
        return input_buffer_.empty();
    }

    /*!
     * \fn  void PipelineStage::Stop()
     *
     * \brief   Forcely stops this stage, no matter whether there is anything
     *          left to process. To make a safe exit, call WaitTillClear()
     *          before calling Stop().
     */
    void Stop() {
        {
            LockGuard l(mutex_);
            kill_switch_ = true;
            cv_ok_to_push_.notify_all();
        }
        if (own_pool_) {
            delete thread_pool_;
            thread_pool_ = nullptr;
        }
    }

 private:
    ENABLE_IF_NOT_VOID(T2, void) RunTask(const TaskIterator& t) {
        t->output = func_(std::move(t->input));
        {
            LockGuard l(mutex_);
            t->state = State::FINISHED;
            CheckQueueNolock(t);
        }
    }

    ENABLE_IF_VOID(T2, void) RunTask(const TaskIterator& t) {
        func_(std::move(t->input));
        {
            LockGuard l(mutex_);
            t->state = State::FINISHED;
            CheckQueueNolock(t);
        }
    }

    ENABLE_IF_NOT_VOID(T2, void) OutputToNextStage(const TaskIterator& t) {
        if (next_stage_ == nullptr) {
            LOG_ERROR() << "No where to put the pipeline output, "
                      << "please call SetNextStage before starting to push task into pipeline";
        }
        if (!kill_switch_) next_stage_->Push(std::move(t->output));
    }

    ENABLE_IF_VOID(T2, void) OutputToNextStage(const TaskIterator& t) {}

    void CheckQueueNolock(const TaskIterator& curr_it) {
        /*
         * When out_of_order==true, we can retire the tasks out
         * of order, i.e. later tasks can output first.
         * This is the default behavior when the pipeline output type
         * is void.
         */
        if (out_of_order_ && curr_it != input_buffer_.end() && curr_it->state == State::FINISHED) {
            OutputToNextStage(curr_it);
            input_buffer_.erase(curr_it);
            cv_ok_to_push_.notify_one();
        } else {
            while (!input_buffer_.empty() && input_buffer_.front().state == State::FINISHED) {
                OutputToNextStage(input_buffer_.begin());
                input_buffer_.pop_front();
                cv_ok_to_push_.notify_one();
            }
        }
        if (input_buffer_.empty()) cv_stage_clear_.notify_all();
        size_t n_running = 0;
        for (TaskIterator it = input_buffer_.begin(); it != input_buffer_.end(); ++it) {
            if (n_running++ >= max_active_tasks_) break;
            if (it->state == State::NEW_TASK) {
                // if we are killing pipeline, don't schedule any new task
                if (kill_switch_) {
                    it->state = State::FINISHED;
                } else {
                    it->state = State::RUNNING;
                    thread_pool_->PushTask(priority_, seq_num_++, [it, this]() { RunTask(it); });
                }
            }
        }
    }
};
}  // namespace fma_common
