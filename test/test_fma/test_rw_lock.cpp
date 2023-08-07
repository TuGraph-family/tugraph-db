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

#include <functional>

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/rw_lock.h"
#include "./unit_test_utils.h"
#include "./rand_r.h"

using namespace fma_common;

struct AutoApplyAndRollbackActions {
 protected:
    std::function<void()> act_;
    std::function<void()> rollback_;
 public:
    AutoApplyAndRollbackActions(const std::function<void()>& act,
                                const std::function<void()>& rollback)
        : act_(act), rollback_(rollback) {
        if (act_) act_();
    }

    ~AutoApplyAndRollbackActions() {
        if (rollback_) rollback_();
    }

    void CancelRollback() {
        rollback_ = nullptr;
    }
};

template<typename LockT, typename AutoRL, typename AutoWL>
void TestWithThreads(size_t nr, size_t nw, size_t nl, size_t nsleep, bool test_upgrade) {
    LockT lock;
    std::vector<std::thread> readers;
    std::vector<std::thread> writers;
    std::atomic<size_t> nreaders(0);
    std::atomic<size_t> nwriters(0);

    for (size_t i = 0; i < nr; i++) {
        readers.emplace_back([&, i]() {
            for (size_t j = 0; j < nl; j++) {
                FMA_DBG() << "reader " << i << " getting read lock";
                try {
                    AutoRL l(lock);
                    FMA_UT_CHECK_EQ(AtomicLoad(nwriters), 0);
                    AutoApplyAndRollbackActions a([&]() { AtomicFetchInc(nreaders); },
                                                    [&]() { AtomicFetchDec(nreaders); });
                    std::this_thread::sleep_for(std::chrono::microseconds(nsleep));
                } catch (LockInterruptedException& e) {
                    FMA_LOG() << "reader " << i << " lock interrupted: " << e.what();
                }
                FMA_DBG() << "reader " << i << " release read lock";
            }
        });
    }

    for (size_t i = 0; i < nw; i++) {
        writers.emplace_back([&, i]() {
            for (size_t j = 0; j < nl; j++) {
                try {
                    if (test_upgrade) {
                        FMA_DBG() << "writer " << i << " getting read lock";
                        AutoRL lr(lock);
                        FMA_UT_CHECK_EQ(AtomicLoad(nwriters), 0);
                        AutoApplyAndRollbackActions a([&]() { AtomicFetchInc(nreaders); },
                                                        [&]() { AtomicFetchDec(nreaders); });
                        FMA_DBG() << "writer " << i << " has read lock";
                        std::this_thread::sleep_for(std::chrono::microseconds(nsleep));
                        AutoWL lw(lock);
                        FMA_DBG() << "writer " << i << " upgrade to write lock";
                        AutoApplyAndRollbackActions b([&]() { AtomicFetchInc(nwriters); },
                                                        [&]() { AtomicFetchDec(nwriters); });
                        FMA_UT_CHECK_EQ(AtomicLoad(nreaders), 1);  // only me
                        FMA_UT_CHECK_EQ(AtomicLoad(nwriters), 1);
                        std::this_thread::sleep_for(std::chrono::microseconds(nsleep));
                    } else {
                        AutoWL lw(lock);
                        FMA_DBG() << "writer " << i << " getting write lock";
                        AutoApplyAndRollbackActions b([&]() { AtomicFetchInc(nwriters); },
                                                      [&]() { AtomicFetchDec(nwriters); });
                        FMA_UT_CHECK_EQ(AtomicLoad(nwriters), 1);
                        std::this_thread::sleep_for(std::chrono::microseconds(nsleep));
                    }
                    FMA_DBG() << "writer " << i << " release read/write lock";
                } catch (LockUpgradeFailedException& e) {
                    FMA_DBG() << "writer " << i
                              << " failed upgrading read lock due to conflict: " << e.what();
                } catch (LockInterruptedException& e) {
                    FMA_LOG() << "writer " << i << " lock interrupted: " << e.what();
                } catch (...) {
                    FMA_FATAL() << "writer " << i << " unknown error.";
                }
            }
        });
    }

    for (auto& t : readers) t.join();
    for (auto& t : writers) t.join();
}

class Interrupt {
    static std::atomic<bool> flag_;
 public:
    static void Set(bool flag) { AtomicStore(flag_, flag); }

    bool operator()() const { return AtomicLoad(flag_); }

    operator bool() const { return true; }
};

std::atomic<bool> Interrupt::flag_{false};

FMA_SET_TEST_PARAMS(RWLock, "");

FMA_UNIT_TEST(RWLock) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    size_t nr = 10;
    size_t nw = 10;
    size_t nl = 100;
    size_t nsleep = 10;
    bool verbose = false;

    fma_common::Configuration config;
    config.Add(nr, "nr", true).Comment("Number of reader threads");
    config.Add(nw, "nw", true).Comment("Number of writer threads");
    config.Add(nl, "nl", true).Comment("Number of locks to do in each thread");
    config.Add(nsleep, "ns", true).Comment("Number of microseconds to sleep in lock");
    config.Add(verbose, "verbose", true).Comment("Whether to print verbose logs");
    config.ParseAndFinalize(argc, argv);

    if (verbose)
        fma_common::Logger::Get().SetLevel(fma_common::LogLevel::LL_DEBUG);

    FMA_LOG() << "Testing SpinningRWLock";
    TestWithThreads<SpinningRWLock, AutoReadLock<SpinningRWLock>, AutoWriteLock<SpinningRWLock>>(
        nr, nw, nl, nsleep, false);
    FMA_LOG() << "Testing RWLock";
    TestWithThreads<SpinningRWLock, AutoReadLock<SpinningRWLock>, AutoWriteLock<SpinningRWLock>>(
        nr, nw, nl, nsleep, false);

    FMA_LOG() << "Testing InterruptableTLSRWLock";
    std::thread interrupt([&]() {
        for (size_t i = 0; i < nl; i++) {
            Interrupt::Set((i % 2 == 0));
            std::this_thread::sleep_for(std::chrono::microseconds(nsleep));
        }
    });
    typedef InterruptableTLSRWLock<Interrupt> TLSLock;
    TestWithThreads<TLSLock, TLSAutoReadLock<TLSLock>, TLSAutoWriteLock<TLSLock>>(
        nr, nw, nl, nsleep, true);
    interrupt.join();

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
