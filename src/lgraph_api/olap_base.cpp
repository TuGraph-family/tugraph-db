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

#include "lgraph/olap_base.h"
#include "fma-common/logger.h"

namespace lgraph_api {
namespace olap {

std::shared_ptr<Worker> &Worker::SharedWorker() {
    static std::shared_ptr<Worker> shared_worker = std::make_shared<Worker>();
    return shared_worker;
}

Worker::Worker() : stopping_(false), has_work_(false), mutex_(), cv_(), task_() {
    worker_ = std::thread([&]() {
        while (true) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [&]() { return has_work_ || stopping_; });
            if (stopping_ && !has_work_) {
                return true;
            }
            (*task_)();
            has_work_ = false;
            task_.reset();
            // this is critical as the wrapped function might be defined
            // in an external library, and if that library is closed while
            // the shared_ptr still points to that location, destruction of
            // the shared_ptr may corrupt the stack
            lock.unlock();
            cv_.notify_all();
        }
    });
}

Worker::~Worker() {
    std::unique_lock<std::mutex> lock(mutex_);
    stopping_ = true;
    lock.unlock();
    cv_.notify_all();
    worker_.join();
}

void Worker::Delegate(const std::function<void()> &work) {
    std::unique_lock<std::mutex> lock(mutex_);
    cv_.wait(lock, [&]() { return !has_work_; });
    auto task = std::make_shared<std::packaged_task<void()> >(work);
    auto result = task->get_future();
    task_ = task;
    has_work_ = true;
    lock.unlock();
    cv_.notify_all();
    result.get();
}

ParallelBitset::ParallelBitset(size_t size) : size_(size) {
#if USE_VALGRIND
    data_ = (uint64_t *)malloc(sizeof(uint64_t) * (WORD_OFFSET(size_) + 1));
    if (!data_) throw std::bad_alloc();
#else
    data_ = (uint64_t *)mmap(nullptr, sizeof(uint64_t) * (WORD_OFFSET(size_) + 1),
                             PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE,
                             -1, 0);
    if (data_ == MAP_FAILED) throw std::runtime_error("memory allocation failed");
#endif
    Clear();
}

ParallelBitset::~ParallelBitset() {
#if USE_VALGRIND
    free(data_);
#else
    int error = munmap(data_, sizeof(uint64_t) * (WORD_OFFSET(size_) + 1));
    if (error != 0) {
        fprintf(stderr, "warning: potential memory leak!\n");
    }
#endif
}

void ParallelBitset::Clear() {
    size_t bm_size = WORD_OFFSET(size_);
    auto worker = Worker::SharedWorker();
    worker->Delegate([&]() {
#pragma omp parallel for
        for (size_t i = 0; i < bm_size; i++) {
            data_[i] = 0;
        }
    });
    data_[bm_size] = 0;
}

void ParallelBitset::Fill() {
    size_t bm_size = WORD_OFFSET(size_);
    auto worker = Worker::SharedWorker();
    worker->Delegate([&]() {
#pragma omp parallel for
        for (size_t i = 0; i < bm_size; i++) {
            data_[i] = (uint64_t)-1;
        }
    });
    data_[bm_size] = 0;
    for (size_t i = (bm_size << 6); i < size_; i++) {
        data_[bm_size] |= 1ul << BIT_OFFSET(i);
    }
}

bool ParallelBitset::Has(size_t i) { return (data_[WORD_OFFSET(i)] & (1ul << BIT_OFFSET(i))) != 0; }

bool ParallelBitset::Add(size_t i) {
    return (__sync_fetch_and_or(data_ + WORD_OFFSET(i), 1ul << BIT_OFFSET(i)) &
            (1ul << BIT_OFFSET(i))) == 0;
}

void ParallelBitset::Swap(ParallelBitset &other) {
    if (size_ != other.size_) throw std::runtime_error("size mismatch!");
    std::swap(data_, other.data_);
}

VertexLockGuard::VertexLockGuard(volatile bool *lock) : lock_(lock) {
    do {
        while (*lock_) std::this_thread::yield();
    } while (__sync_lock_test_and_set(lock_, 1));
}

VertexLockGuard::~VertexLockGuard() { __sync_lock_release(lock_); }

}  // namespace olap
}  // namespace lgraph_api
