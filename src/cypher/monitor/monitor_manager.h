/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include <stdlib.h>
#include <pthread.h>

#include <atomic>
#include <mutex>
#include <unordered_map>
#include <string>
#include <thread>
#include <limits>

#include "core/defs.h"
#include "graph/graph.h"
#include "parser/data_typedef.h"
#include "execution_plan/runtime_context.h"
#include "cypher/cypher_types.h"

struct MemoryInfo {
    size_t usage;
    size_t limit;
    MemoryInfo() : usage(0), limit(lgraph::_detail::DEFAULT_MEM_LIMIT) {}
    MemoryInfo(size_t u, size_t l, size_t m) : usage(u), limit(l) {}
    MemoryInfo(const MemoryInfo& other) : usage(other.usage), limit(other.limit) {}
    MemoryInfo& operator=(const MemoryInfo& other) {
        usage = other.usage;
        limit = other.limit;
        return *this;
    }
    ~MemoryInfo() {}
};

struct MonitorManager {
    mutable std::mutex _mutex;
    std::unordered_map<std::thread::id, std::string> user_id;
    std::unordered_map<std::string, MemoryInfo> memory_manager;
    MonitorManager() {}
    ~MonitorManager() {}

    void AddMemoryUsage(size_t size) {
        size_t memory_limit = this->GetMemoryLimit();
        std::lock_guard<std::mutex> lck(_mutex);
        std::thread::id tid = std::this_thread::get_id();

        if (user_id.find(tid) == user_id.end()) {
            return;
        }
        std::string user = user_id[tid];

        size_t memory_usage = memory_manager[user].usage + size;
        if (memory_usage > memory_limit) {
            throw lgraph::CypherException(
                FMA_FMT("excess user memory limit,user memory limit is {},your memory usage is {}",
                        memory_limit, memory_usage));
        }
        memory_manager[user].usage += size;
    }

    void MinusMemoryUsage(size_t size) {
        std::lock_guard<std::mutex> lck(_mutex);
        std::thread::id tid = std::this_thread::get_id();

        if (user_id.find(tid) == user_id.end()) {
            return;
        }
        std::string user = user_id[tid];

        if (memory_manager[user].usage < size) {
            throw lgraph::CypherException(FMA_FMT("memory usage is {}, size is {}, user is {}",
                                                  memory_manager[user].usage, size, user));
        }
        memory_manager[user].usage -= size;
    }

    size_t GetMemoryUsage(std::string user) {
        if (memory_manager.find(user) == memory_manager.end()) {
            return 0;
        }
        return memory_manager[user].usage;
    }

    void BindTid(std::string user) {
        std::lock_guard<std::mutex> lck(_mutex);
        std::thread::id tid = std::this_thread::get_id();
        if (user_id.find(tid) == user_id.end()) {
            auto it = user_id.end();
            user_id.emplace_hint(it, tid, user);
        } else {
            user_id[tid] = user;
        }
    }

    void SetMemoryLimit(size_t size) {
        std::lock_guard<std::mutex> lck(_mutex);
        std::thread::id tid = std::this_thread::get_id();
        std::string user = user_id[tid];
        if (memory_manager.find(user) == memory_manager.end()) {
            struct MemoryInfo init_mem;
            auto it = memory_manager.end();
            memory_manager.emplace_hint(it, user, init_mem);
            return;
        }
        memory_manager[user].limit = size;
    }

    size_t GetMemoryLimit() {
        std::lock_guard<std::mutex> lck(_mutex);
        std::thread::id tid = std::this_thread::get_id();
        if (user_id.find(tid) == user_id.end()) {
            return lgraph::_detail::DEFAULT_MEM_LIMIT;
        }
        std::string user = user_id[tid];
        return memory_manager[user].limit;
    }
};

extern MonitorManager AllocatorManager;
