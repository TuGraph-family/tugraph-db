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

#pragma once
#include <iostream>
#include <memory>
#include <mutex>
#include <cstdlib>
#include <unordered_map>

#define LGR_MEM_PROFILE 0

namespace lgraph {
#if LGR_MEM_PROFILE
namespace _detail {
class MemManager {
    struct MemBlockInfo {
        MemBlockInfo() {}
        MemBlockInfo(size_t s, const char* f, size_t ln) : size(s), file(f), line_num(ln) {}

        size_t size;
        const char* file;
        size_t line_num;
    };

    std::mutex mutex_;
    std::unordered_map<void*, MemBlockInfo> blocks_;

 public:
    ~MemManager() {
        std::lock_guard<std::mutex> l(mutex_);
        if (!blocks_.empty()) {
            std::cerr << "Memory block not released: " << std::endl;
            for (auto& kv : blocks_) {
                std::cerr << "\t[" << kv.first << "]: "
                          << "size=" << kv.second.size << ", malloced in " << kv.second.file << ":"
                          << kv.second.line_num << std::endl;
            }
            exit(1);
        }
    }

    void* Malloc(size_t s, const char* file, size_t ln) {
        std::lock_guard<std::mutex> l(mutex_);
        void* p = malloc(s);
        if (blocks_.find(p) != blocks_.end()) {
            std::cerr << "Pointer " << p << " returned twice in memory manager." << std::endl;
            exit(1);
        }
        blocks_[p] = MemBlockInfo(s, file, ln);
        return p;
    }

    void Free(void* p, const char* file, size_t ln) {
        std::lock_guard<std::mutex> l(mutex_);
        if (blocks_.find(p) == blocks_.end()) {
            std::cerr << "Failed to free pointer " << p << ": already freed or not malloced. "
                      << "Error occurred at " << file << ":" << ln;
            exit(1);
        }
        blocks_.erase(p);
        free(p);
    }

    void* Realloc(void* p, size_t s, const char* file, size_t ln) {
        std::lock_guard<std::mutex> l(mutex_);
        if (blocks_.find(p) == blocks_.end()) {
            std::cerr << "Failed to realloc pointer " << p << ": already freed or not malloced. "
                      << "Error occurred at " << file << ":" << ln;
            exit(1);
        }
        blocks_.erase(p);
        void* np = realloc(p, s);
        blocks_[np] = MemBlockInfo(s, file, ln);
        return np;
    }
};

inline MemManager& GetMemManager() {
    static MemManager manager;
    return manager;
}
}  // namespace _detail
#define LBMalloc(size) ::lgraph::_detail::GetMemManager().Malloc((size), __FILE__, __LINE__)
#define LBFree(p) ::lgraph::_detail::GetMemManager().Free((p), __FILE__, __LINE__)
#define LBRealloc(p, size) \
    ::lgraph::_detail::GetMemManager().Realloc((p), (size), __FILE__, __LINE__)
#else
#define LBMalloc(size) malloc(size)
#define LBFree(p) free(p)
#define LBRealloc(p, size) realloc((p), (size))
#endif
}  // namespace lgraph
