﻿/**
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

#include <mutex>

#include "fma-common/logger.h"

namespace lgraph {
static const int LGRAPH_MAX_THREADS = 480;

class ThreadIdAssigner {
    static const int N = LGRAPH_MAX_THREADS;

    static int *occupied_() {
        static int v[N] = {};
        return v;
    }

    static std::mutex &mutex_() {
        static std::mutex m;
        return m;
    }

 public:
    static int GetThreadId() {
        std::lock_guard<std::mutex> l(mutex_());
        for (int i = 0; i < N; i++) {
            if (!occupied_()[i]) {
                occupied_()[i] = 1;
                return i;
            }
        }
        return -1;
    }

    static void ReleaseThreadId(int id) { occupied_()[id] = 0; }
};

class ThreadIdFetcher {
    int id_;

 public:
    ThreadIdFetcher() {
        id_ = ThreadIdAssigner::GetThreadId();
        FMA_ASSERT(id_ >= 0) << "Too many concurrent threads exists!";
    }

    ~ThreadIdFetcher() { ThreadIdAssigner::ReleaseThreadId(id_); }

    int Get() { return id_; }
};

class ThreadLocalId {
    static thread_local ThreadIdFetcher id_;

 public:
    static inline int GetMyThreadId() { return id_.Get(); }
};

static inline int GetMyThreadId() { return ThreadLocalId::GetMyThreadId(); }
}  // namespace lgraph
