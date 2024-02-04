//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <atomic>
#include "fma-common/type_traits.h"

namespace fma_common {
static const int FMA_CACHE_LINE_SIZE = 64;

template <typename T>
struct alignas(FMA_CACHE_LINE_SIZE) CacheAlignedObject {
    T data;

    CacheAlignedObject() : data() {}
};

template <typename T>
struct alignas(FMA_CACHE_LINE_SIZE) CacheAlignedObject<std::atomic<T>> {
    std::atomic<T> data;

    CacheAlignedObject() : data(T()) {}
};

template <class T, size_t N>
class StaticCacheAlignedVector {
 private:
    // properly aligned uninitialized storage for N T's
    T* data;
    char* buffer_;
    std::size_t m_size = N;

 public:
    StaticCacheAlignedVector() {
        buffer_ = new char[sizeof(T) * N + FMA_CACHE_LINE_SIZE];
        data = (T*)(buffer_ + (FMA_CACHE_LINE_SIZE - ((uint64_t)buffer_ % FMA_CACHE_LINE_SIZE)));
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
        delete[] buffer_;
    }

    size_t size() const { return m_size; }
};
}  // namespace fma_common
