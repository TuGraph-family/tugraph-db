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

#include "fma-common/logger.h"
#include "fma-common/configuration.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/managed_object.h"
#include "./ut_utils.h"

class TestManagedObject : public TuGraphTest {};

struct Ref {
    std::atomic<int>* ref_;

    explicit Ref(std::atomic<int>* ref_count) : ref_(ref_count) { (*ref_count)++; }

    ~Ref() { (*ref_)--; }
};

TEST_F(TestManagedObject, ManagedObject) {
    size_t n_ptr = 10000;
    size_t n_thread = 100;
    int argc = _ut_argc;
    char ** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(n_ptr, "n_ptr", true).Comment("Number of pointers to use in stress test");
    config.Add(n_thread, "n_thread", true).Comment("Number of threads to use in stress test");
    config.ParseAndFinalize(argc, argv);
    using lgraph::GCRefCountedPtr;
    std::atomic<int> ref(0);
    {
        GCRefCountedPtr<Ref> p1;
        UT_EXPECT_EQ(ref, 0);
        GCRefCountedPtr<Ref> p2(new Ref(&ref));
        UT_EXPECT_EQ(ref, 1);
        GCRefCountedPtr<Ref> p3(new Ref(&ref));
        UT_EXPECT_EQ(ref, 2);
        p2.Assign(nullptr);
        fma_common::SleepS(1.3);
        UT_EXPECT_EQ(ref, 1);
        p1 = std::move(p3);
        UT_EXPECT_EQ(ref, 1);
        p3.Assign(nullptr);
        UT_EXPECT_EQ(ref, 1);
        GCRefCountedPtr<Ref> p4(std::move(p1));

        auto managed_ptr = p4.GetScopedRef();
        p4.Assign(nullptr);
        UT_EXPECT_EQ(ref, 1);
        managed_ptr.Release();
        fma_common::SleepS(1.3);
        UT_EXPECT_EQ(ref, 0);
    }

    bool started = false;
    bool end = false;
    std::mutex mutex;
    std::condition_variable cv;

    GCRefCountedPtr<Ref> p5(new Ref(&ref));

    std::thread t([&]() {
        auto mptr = p5.GetScopedRef();
        auto mptr2 = p5.GetScopedRef();
        std::unique_lock<std::mutex> l(mutex);
        started = true;
        cv.notify_all();
        while (!end) cv.wait(l);
    });

    std::atomic<int> ref2(0);
    {
        std::unique_lock<std::mutex> l(mutex);
        while (!started) cv.wait(l);
    }
    p5.EmplaceNewVersion(&ref2);
    fma_common::SleepS(1.3);
    UT_EXPECT_EQ(ref, 1);
    UT_EXPECT_EQ(ref2, 1);
    end = true;
    cv.notify_all();
    t.join();
    fma_common::SleepS(1.3);
    UT_EXPECT_EQ(ref, 0);
    UT_EXPECT_EQ(ref2, 1);

    // stress test
    std::atomic<size_t> n_threads_started(0);
    end = false;
    std::vector<GCRefCountedPtr<Ref>> v(n_ptr);
    for (auto& p : v) p.Assign(new Ref(&ref));
    std::vector<std::thread> ts;
    for (size_t i = 0; i < n_thread; i++)
        ts.emplace_back([&]() {
            std::vector<lgraph::ScopedRef<Ref>> mptrs;
            for (auto& p : v) mptrs.emplace_back(p.GetScopedRef());
            for (size_t i = 0; i < 100; i++)
                mptrs.emplace_back(v[rand() % v.size()].GetScopedRef());
            n_threads_started++;
            if (n_threads_started == n_thread) cv.notify_all();
            std::unique_lock<std::mutex> l(mutex);
            while (!end) cv.wait(l);
        });
    {
        std::unique_lock<std::mutex> l(mutex);
        while (n_threads_started != n_thread) cv.wait(l);
    }
    // all ref get
    v.clear();
    fma_common::SleepS(1.3);
    UT_EXPECT_EQ(ref, n_ptr);
    end = true;
    cv.notify_all();
    UT_LOG() << "Waiting for threads to join";
    for (auto& t : ts) t.join();
    UT_LOG() << "Threads joined";
    fma_common::SleepS(1.3);
    UT_EXPECT_EQ(ref, 0);
}
