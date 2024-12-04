//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <thread>

#include "fma-common/atomic_ops.h"
#include "fma-common/cache_aligned_vector.h"
#include "fma-common/thread_id.h"
#include "fma-common/utils.h"
#include "fma-common/string_formatter.h"

namespace fma_common {

enum class LockStatus {
    SUCCESS = 0,
    INTERRUPTED = 1,
    UPGRADE_FAILED = 2
};

class InvalidThreadIdError : public std::runtime_error {
 public:
    InvalidThreadIdError() : std::runtime_error("Invalid thread id.") {}
    explicit InvalidThreadIdError(const std::string& msg) : std::runtime_error(msg) {}
};

// RWLock using thread-local storage.
// Multi-reader, multi-writer, read optimized, write prioritized RWLock.
// Read locks are kept in thread-local storage, so reading is optimal.
// Readers wait for writers as long as there is any writer, so write is prioritized.
// NOTE: ReadLock/ReadUnlock pairs must be used in the same thread since we keep the
// lock info in TLS.
// Allows re-entry.
// The locking process is interrupted if should_interrupt() returns true.
template <typename ShouldInterruptFuncT>
class InterruptableTLSRWLock {
    std::atomic<int64_t> n_writers_;
    StaticCacheAlignedVector<std::atomic<int64_t>, FMA_MAX_THREADS> reader_locks_;
    std::atomic<int> curr_writer_;
    std::atomic<int64_t> write_reentry_level_;
    ShouldInterruptFuncT should_interrupt_;

    DISABLE_COPY(InterruptableTLSRWLock);
    DISABLE_MOVE(InterruptableTLSRWLock);

    void ThrowOnInvalidTID(int tid) {
        if (tid < 0 || tid >= FMA_MAX_THREADS) {
            throw InvalidThreadIdError(FMA_FMT(
                "Invalid thread id, tid:{}, FMA_MAX_THREADS:{}", tid, FMA_MAX_THREADS));
        }
    }

    #define RETURN_IF_INTERRUPTED() \
        if (should_interrupt_ && should_interrupt_()) return LockStatus::INTERRUPTED;

 public:
    explicit InterruptableTLSRWLock(const ShouldInterruptFuncT& interrupt = ShouldInterruptFuncT())
        : n_writers_(0), curr_writer_(-1), write_reentry_level_(0), should_interrupt_(interrupt) {
        for (size_t i = 0; i < reader_locks_.size(); i++) AtomicStore(reader_locks_[i], 0);
    }

    /**
     * \brief Acquires a read lock. Allows re-entry.
     *
     * \param tid   Thread id of the locking thread.
     *
     * \return LockStatus::SUCCESS if succeed.
     *         LockStatus::INTERRUPTED if interrupted.
     */
    LockStatus ReadLock(int tid) {
        ThrowOnInvalidTID(tid);
        while (true) {
            // if re-entry, we already have the lock
            if (AtomicFetchInc(reader_locks_[tid]) > 0) break;
            // otherwise, wait for writers if necessary
            // we might already have write lock, in this case just return
            if (AtomicLoad(n_writers_) == 0 || AtomicLoad(curr_writer_) == tid) break;
            // there is writer, release read lock and wait
            AtomicFetchDec(reader_locks_[tid]);
            while (AtomicLoad(n_writers_)) {
                RETURN_IF_INTERRUPTED();
                std::this_thread::yield();
            }
        }
        return LockStatus::SUCCESS;
    }

    LockStatus ReadLock() { return ReadLock(GetMyThreadId()); }

    /**
     * \brief Releases a read lock.
     *
     * \param tid   Thread id of the releasing thread, must match the thread id used in
     *              ReadLock().
     */
    void ReadUnlock(int tid) {
        ThrowOnInvalidTID(tid);
        int64_t r = AtomicFetchDec(reader_locks_[tid]);
        FMA_DBG_CHECK_GE(r, 1);
    }

    void ReadUnlock() { ReadUnlock(GetMyThreadId()); }

    /**
     * Obtain a write lock. Allows re-entry. If the thread already holds read lock,
     * the read lock is upgraded to a write lock.
     *
     * \param tid   Thread id of the locking thread.
     *
     * \return LockStatus::SUCCESS on success.
     *         LockStatus::INTERRUPTED if interrupted.
     *         LockStatus::UPGRADE_FAILED if failed to upgrade a read lock into write lock.
     */
    LockStatus WriteLock(int tid) {
        ThrowOnInvalidTID(tid);
        while (AtomicFetchInc(n_writers_) != 0) {
            // handle re-entry
            if (AtomicLoad(curr_writer_) == tid) {
                write_reentry_level_++;
                return LockStatus::SUCCESS;
            }
            // there are already some writer, wait until it is done
            AtomicFetchDec(n_writers_);
            // If this thread already holds a read lock, we must release the read lock
            // so that the writer can continue.
            if (AtomicLoad(reader_locks_[tid]) != 0) return LockStatus::UPGRADE_FAILED;
            while (AtomicLoad(n_writers_)) {
                RETURN_IF_INTERRUPTED();
                std::this_thread::yield();
            }
        }
        // I am the first writer, check for readers
        for (size_t i = 0; i < reader_locks_.size(); i++) {
            if (tid == (int)i) continue;  // upgrade from read to write lock automatically
            // wait for each reader
            while (AtomicLoad(reader_locks_[i])) {
                if (should_interrupt_ && should_interrupt_()) {
                    // rollback changes and return interrupted
                    AtomicFetchDec(n_writers_);
                    return LockStatus::INTERRUPTED;
                }
                std::this_thread::yield();
            }
        }
        AtomicStore(curr_writer_, tid);
        write_reentry_level_++;
        return LockStatus::SUCCESS;
    }

    // get an exclusive write lock
    // if the calling thread already has read locks, they are ignored and
    // the write lock will be obtained if other threads does not have locks.
    LockStatus WriteLock() { return WriteLock(GetMyThreadId()); }

    void WriteUnlock(int tid) {
        ThrowOnInvalidTID(tid);
        // if this thread no longer holds the write lock, set writer id to -1
        if (AtomicFetchDec(write_reentry_level_) == 1) AtomicStore(curr_writer_, -1);
        // now release the lock
        int64_t r = AtomicFetchDec(n_writers_);
        FMA_DBG_CHECK_GE(r, 1);
    }
};

class LockInterruptedException : public std::runtime_error {
 public:
    LockInterruptedException() : std::runtime_error("Lock interrupted.") {}
};

class LockUpgradeFailedException : public std::runtime_error {
 public:
    LockUpgradeFailedException()
        : std::runtime_error("Failed to upgrade read lock to write lock.") {}
};

template <typename LockT>
class TLSAutoReadLock {
 public:
    typedef LockT LockType;

 private:
    LockT& lock_;
    int tid_;
    bool locked_;
    DISABLE_COPY(TLSAutoReadLock);
    DISABLE_MOVE(TLSAutoReadLock);

 public:
    TLSAutoReadLock(LockT& l, int tid) : lock_(l), tid_(tid), locked_(false) { Lock(); }

    explicit TLSAutoReadLock(LockT& l) : TLSAutoReadLock(l, GetMyThreadId()) {}

    ~TLSAutoReadLock() { Unlock(); }

    void Unlock() {
        if (_F_UNLIKELY(!locked_)) return;
        lock_.ReadUnlock(tid_);
        locked_ = false;
    }

    void Lock() {
        if (_F_UNLIKELY(locked_)) return;
        LockStatus status = lock_.ReadLock(tid_);
        locked_ = (status == LockStatus::SUCCESS);
        if (!locked_) throw LockInterruptedException();
    }
};

template <typename LockT>
class TLSAutoWriteLock {
 public:
    typedef LockT LockType;

 private:
    LockT& lock_;
    int tid_;
    bool locked_;
    DISABLE_COPY(TLSAutoWriteLock);
    DISABLE_MOVE(TLSAutoWriteLock);

 public:
    TLSAutoWriteLock(LockT& l, int tid) : lock_(l), tid_(tid), locked_(false) { Lock(); }

    explicit TLSAutoWriteLock(LockT& l) : TLSAutoWriteLock(l, GetMyThreadId()) {}

    ~TLSAutoWriteLock() { Unlock(); }

    void Unlock() {
        if (_F_UNLIKELY(!locked_)) return;
        lock_.WriteUnlock(tid_);
        locked_ = false;
    }

    void Lock() {
        if (_F_UNLIKELY(locked_)) return;
        LockStatus status = lock_.WriteLock(tid_);
        locked_ = (status == LockStatus::SUCCESS);
        if (status == LockStatus::INTERRUPTED) {
            throw LockInterruptedException();
        } else if (status == LockStatus::UPGRADE_FAILED) {
            throw LockUpgradeFailedException();
        }
    }
};

// Single-writer, multi-reader lock using spining
// no-reentry allowed, otherwise might cause dead-lock, e.g. in the following case:
// t1: read_lock    t2: write_lock    t1: read_lock
// t2 is waiting for n_readers to become 0, while t1 is waiting for writing to become false
class SpinningRWLock {
    std::atomic<size_t> n_readers;
    std::atomic<bool> writing;

    DISABLE_COPY(SpinningRWLock);
    DISABLE_MOVE(SpinningRWLock);

 public:
    SpinningRWLock() : n_readers(0), writing(false) {}

    void ReadLock() {
        while (true) {
            n_readers++;
            if (!writing) return;
            n_readers--;
            while (writing) std::this_thread::yield();
        }
    }

    void WriteLock() {
        bool f = false;
        while (!writing.compare_exchange_strong(f, true, std::memory_order_seq_cst)) {
            std::this_thread::yield();
            f = false;
        }
        while (n_readers) std::this_thread::yield();
    }

    void ReadUnlock() { n_readers--; }

    void WriteUnlock() { writing = false; }
};

// Single-writer, multi-reader lock
// no-reentry allowed, otherwise might cause dead-lock, e.g. in the following case:
// t1: read_lock    t2: write_lock    t1: read_lock
// t2 is waiting for n_readers to become 0, while t1 is waiting for writing to become false
class RWLock {
    std::mutex mutex_;
    std::condition_variable cv_;
    size_t n_readers_;
    bool writing_;

    DISABLE_COPY(RWLock);
    DISABLE_MOVE(RWLock);

 public:
    RWLock() : mutex_(), cv_(), n_readers_(0), writing_(false) {}

    void ReadLock() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() { return !writing_; });
        n_readers_++;
    }

    void WriteLock() {
        std::unique_lock<std::mutex> lock(mutex_);
        cv_.wait(lock, [&]() { return !n_readers_ && !writing_; });
        writing_ = true;
    }

    void ReadUnlock() {
        std::unique_lock<std::mutex> lock(mutex_);
        n_readers_--;
        cv_.notify_all();
    }

    void WriteUnlock() {
        std::unique_lock<std::mutex> lock(mutex_);
        writing_ = false;
        cv_.notify_all();
    }
};

template <typename LockT>
class AutoReadLock {
    LockT& l_;
    bool locked_;

    DISABLE_COPY(AutoReadLock);

 public:
    explicit AutoReadLock(LockT& l) : l_(l) {
        l_.ReadLock();
        locked_ = true;
    }

    AutoReadLock(AutoReadLock&& rhs) : l_(rhs.l_), locked_(rhs.locked_) { rhs.locked_ = false; }

    AutoReadLock& operator=(AutoReadLock&&) = delete;

    ~AutoReadLock() {
        if (locked_) l_.ReadUnlock();
    }

    void Unlock() {
        l_.ReadUnlock();
        locked_ = false;
    }
};

template <typename LockT>
class AutoWriteLock {
    LockT& l_;
    bool locked_;

    DISABLE_COPY(AutoWriteLock);

 public:
    explicit AutoWriteLock(LockT& l) : l_(l) {
        l_.WriteLock();
        locked_ = true;
    }

    AutoWriteLock(AutoWriteLock&& rhs) : l_(rhs.l_), locked_(rhs.locked_) { rhs.locked_ = false; }

    AutoWriteLock& operator=(AutoWriteLock&&) = delete;

    ~AutoWriteLock() {
        if (locked_) l_.WriteUnlock();
    }

    void Unlock() {
        l_.WriteUnlock();
        locked_ = false;
    }
};

}  // namespace fma_common
