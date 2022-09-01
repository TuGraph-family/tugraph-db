/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
