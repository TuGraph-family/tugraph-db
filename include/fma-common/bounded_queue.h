//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\bounded_queue.h.
 *
 * \brief   Declares the bounded queue class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */
#pragma once

#include <condition_variable>
#include <deque>
#include <limits>
#include <mutex>

#include "fma-common/type_traits.h"

namespace fma_common {
/*!
 * \class   DataConsumer
 *
 * \brief   A data consumer that consumes data of type T.
 */
template <typename T>
class DataConsumer {
 public:
    /*!
     * \fn  virtual DataConsumer::~DataConsumer()
     *
     * \brief   Destructor.
     */
    virtual ~DataConsumer() {}

    /*!
     * \fn  virtual bool DataConsumer::Push(T&&) = 0;
     *
     * \brief   Pushes an data into the consumer.
     *
     * \param [in,out]  parameter1  rvalue reference to the data to be consumed
     *
     * \return  True if it succeeds, false if it fails.
     */
    virtual bool Push(T&&) = 0;

    /*!
     * \fn  virtual bool DataConsumer::Push(T&&) = 0;
     *
     * \brief   Pushes an data into the consumer.
     *
     * \param  parameter1  const reference to the data to be consumed
     *
     * \return  True if it succeeds, false if it fails.
     */
    virtual bool Push(const T&) = 0;
};

/*!
 * \class   DataProducer
 *
 * \brief   A data producer that produces data of type T.
 */
template <typename T>
class DataProducer {
 public:
    virtual ~DataProducer() {}

    /*!
     * \fn  virtual bool DataProducer::Pop(T&) = 0;
     *
     * \brief   Pop a data object from the producer.
     *
     * \param [in,out]  parameter1  The data to be popped.
     *
     * \return  True if it succeeds, false if it fails.
     */
    virtual bool Pop(T&) = 0;
};

/*!
 * \class   BoundedQueue
 *
 * \brief   Concurrent queue which has limited capacity. Trying to push
 *          into a full queue will block the caller.
 */
template <typename T>
class BoundedQueue : public DataConsumer<T>, public DataProducer<T> {
    using LockGuard = std::lock_guard<std::mutex>;
    using UniqueLock = std::unique_lock<std::mutex>;

    std::deque<T> queue_;
    bool kill_switch_;
    mutable std::mutex mutex_;
    std::condition_variable cv_ok_to_push_;
    std::condition_variable cv_ok_to_pop_;
    size_t capacity_;

 public:
    DISABLE_COPY(BoundedQueue);
    /*! \brief Move constructor is disable because mutex and cv cannot be moved */
    DISABLE_MOVE(BoundedQueue);

    /*!
     * \fn  BoundedQueue::BoundedQueue(size_t capacity = std::numeric_limits<size_t>::max())
     *
     * \brief   Constructor.
     *
     * \param   capacity    (Optional) The maximum capacity.
     */
    explicit BoundedQueue(size_t capacity = std::numeric_limits<size_t>::max())
        : kill_switch_(false), capacity_(capacity) {}

    /*!
     * \fn  virtual BoundedQueue::~BoundedQueue()
     *
     * \brief   Destructor.
     */
    virtual ~BoundedQueue() { EndQueue(); }

    /*!
     * \fn  void BoundedQueue::SetCapacity(size_t capacity)
     *
     * \brief   Sets the maximum capacity. MUST be called before pushing
     *          any element.
     *
     * \param   capacity    The capacity.
     */
    void SetCapacity(size_t capacity) { capacity_ = capacity; }

    /*!
     * \fn  virtual bool DataConsumer::Push(T&&) = 0;
     *
     * \brief   Pushes an data into the consumer.
     *
     * \param [in,out]  parameter1  rvalue reference to the data to be consumed
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Push(T&& d) override {
        UniqueLock l(mutex_);
        while (queue_.size() >= capacity_ && !kill_switch_) {
            cv_ok_to_push_.wait(l);
        }
        if (kill_switch_) {
            return false;
        }
        queue_.emplace_back(std::move(d));
        cv_ok_to_pop_.notify_one();
        return !kill_switch_;
    }

    /*!
     * \fn  virtual bool DataConsumer::Push(T&&) = 0;
     *
     * \brief   Pushes an data into the consumer.
     *
     * \param  parameter1  const reference to the data to be consumed
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Push(const T& d) override {
        T tmp(d);
        return Push(std::move(tmp));
    }

    /*!
     * \fn  bool BoundedQueue::PopFront(T& d)
     *
     * \brief   Pops the front element into d.
     *
     * \param [in,out]  d   data to be popped.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool PopFront(T& d) { return PopImpl(d, true); }

    /*!
     * \fn  bool BoundedQueue::PopBack(T& d)
     *
     * \brief   Pops the back element into d.
     *
     * \param [in,out]  d   data to be popped.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool PopBack(T& d) { return PopImpl(d, false); }

    /*!
     * \fn  virtual bool DataProducer::Pop(T&) = 0;
     *
     * \brief   Pop a data object from the producer.
     *
     * \param [in,out]  parameter1  The data to be popped.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool Pop(T& d) override { return PopFront(d); }

    /*!
     * \fn  void BoundedQueue::EndQueue()
     *
     * \brief   Send kill signal to the threads that is waiting for push and
     *          pop, causing them to return false.
     */
    void EndQueue() {
        LockGuard l(mutex_);
        kill_switch_ = true;
        cv_ok_to_pop_.notify_all();
        cv_ok_to_push_.notify_all();
    }

    /*!
     * \fn  bool BoundedQueue::WaitEmpty()
     *
     * \brief   Wait for the queue to become empty.
     *          This should be used when no one is pushing into the queue, or
     *          the result if undefined.
     *
     * \return  True if kill_switch is not on
     */
    bool WaitEmpty() {
        UniqueLock l(mutex_);
        while (!queue_.empty() && !kill_switch_) {
            cv_ok_to_push_.wait(l);
        }
        return !kill_switch_;
    }

    bool IsEmpty() const {
        UniqueLock l(mutex_);
        return queue_.empty();
    }

 protected:
    bool PopImpl(T& d, bool pop_front) {
        UniqueLock l(mutex_);
        while (queue_.empty() && !kill_switch_) {
            cv_ok_to_pop_.wait(l);
        }
        if (kill_switch_) {
            return false;
        }
        if (pop_front) {
            d = std::move(queue_.front());
            queue_.pop_front();
        } else {
            d = std::move(queue_.back());
            queue_.pop_back();
        }
        if (queue_.size() < capacity_) {
            cv_ok_to_push_.notify_one();
        }
        return true;
    }
};
}  // namespace fma_common
