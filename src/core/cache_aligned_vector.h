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

#include <atomic>

namespace lgraph {
static const int LGRAPH_CACHE_LINE_SIZE = 64;

template <typename T>
struct alignas(LGRAPH_CACHE_LINE_SIZE) CacheAlignedObject {
    T data;

    CacheAlignedObject() : data() {}
};

template <typename T>
struct alignas(LGRAPH_CACHE_LINE_SIZE) CacheAlignedObject<std::atomic<T>> {
    std::atomic<T> data;

    CacheAlignedObject() : data(T()) {}
};

template <class T, size_t N>
class StaticCacheAlignedVector {
 private:
    // properly aligned uninitialized storage for N T's
    typename std::aligned_storage<sizeof(T), LGRAPH_CACHE_LINE_SIZE>::type data[N];
    std::size_t m_size = N;

 public:
    StaticCacheAlignedVector() {
        // construct value in memory of aligned storage
        // using inplace operator new
        for (size_t i = 0; i < N; i++) {
            new (&data[i]) T();
        }
    }

    // Access an object in aligned storage
    const T& operator[](std::size_t pos) const { return *reinterpret_cast<const T*>(&data[pos]); }

    T& operator[](std::size_t pos) { return *reinterpret_cast<T*>(&data[pos]); }

    // Delete objects from aligned storage
    ~StaticCacheAlignedVector() {
        for (std::size_t pos = 0; pos < m_size; ++pos) {
            reinterpret_cast<T*>(&data[pos])->~T();
        }
    }

    size_t size() const { return m_size; }
};
}  // namespace lgraph
