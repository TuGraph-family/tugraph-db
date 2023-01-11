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
#include "monitor_manager.h"

template <class T>
class MemoryMonitorAllocator {
 public:
    using value_type = T;
    using pointer = T *;
    using const_pointer = const T *;

    using void_pointer = void *;
    using const_void_pointer = const void *;

    using size_type = size_t;
    using difference = std::ptrdiff_t;

    template <class U>
    struct rebind {
        using other = MemoryMonitorAllocator<U>;
    };

    MemoryMonitorAllocator() { allocCount = 0; }
    ~MemoryMonitorAllocator() = default;

    pointer allocate(size_type numObjects) {
        allocCount += numObjects;
        AllocatorManager.AddMemoryUsage(numObjects);
        return static_cast<pointer>(operator new(sizeof(T) * numObjects));
    }

    pointer allocate(size_type numObjects, const_void_pointer hint) { return allocate(numObjects); }

    void deallocate(pointer p, size_type numObjects) {
        allocCount = allocCount - numObjects;
        AllocatorManager.MinusMemoryUsage(numObjects);
        operator delete(p);
    }

    size_type max_size() const { return std::numeric_limits<size_type>::max(); }

    const size_type *get_allocations() {
        const size_type *alloc = &allocCount;
        return alloc;
    }

 private:
    size_type allocCount;
};
