/**
 * Copyright 2023 AntGroup CO., Ltd.
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
 *
 *  Author:
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#ifndef GEAXFRONTEND_COMMON_OBJECTALLOCATOR_H_
#define GEAXFRONTEND_COMMON_OBJECTALLOCATOR_H_

#include <forward_list>
#include <type_traits>

#include "geax-front-end/common/FollyArena.h"

namespace geax {
namespace common {

/**
 * This allocator allocates different types of objects,
 * and also is responsible for their lifetime.
 *
 * It is suitable for those objects with a small amount, and are shared at different
 * places.
 *
 * ThreadSafe: NO.
 */
template <typename Alloc = folly::clone::SysArena>
class ObjectAllocator {
public:
    /**
     * Constructors
     */
    explicit ObjectAllocator(Alloc& alloc) : alloc_(&alloc) {}
    // uses a default allocator
    ObjectAllocator() : alloc_(&defaultAlloc_) {}

    // allocates an object of type T with parameters Args
    template <typename T, typename... Args>
    T* allocate(Args&&... args);

    ObjectAllocator(const ObjectAllocator&) = delete;
    void operator=(const ObjectAllocator&) = delete;
    ObjectAllocator(ObjectAllocator&&) = delete;
    void operator=(const ObjectAllocator&&) = delete;
    size_t totalSize() { return defaultAlloc_.totalSize(); }

private:
    class ObjectHolder;
    static_assert(sizeof(ObjectHolder) == 16, "ObjectHolder is too large");

    // ObjectHolder holds an object and its destructor, release the resource hold by the object
    // in the destructor of this holder.
    class ObjectHolder {
    public:
        template <typename T>
        explicit ObjectHolder(T* obj)
            : obj_(obj), destructFn_([](void* ptr) {
                  // Optimized
                  // Note: don't worry, the optimizer will optimize this
                  // So this is the advantage of std::function, we can
                  // just call nothing without an address jump.
                  if (!std::is_trivially_destructible<T>::value) {  // NOLINT
                      reinterpret_cast<T*>(ptr)->~T();              // NOLINT
                  }                                                 // NOLINT
              }) {}                                                 // NOLINT
        ~ObjectHolder() { destructFn_(obj_); }

    private:
        void* obj_;
        void (*destructFn_)(void*);
        // Typically, the size of a function is 32, and we should not pay this
        // too much to avoid an indrect function call.
        // std::function<void(void *)> destructFn_;
    };

private:
    Alloc defaultAlloc_;
    Alloc* alloc_{nullptr};
    // TODO(boyao.zby) Let's replace it with single ended queue,
    // i.e., SegmentVector<> in the future.
    std::forward_list<ObjectHolder> objects_;
};

template <typename Alloc>
template <typename T, typename... Args>
T* ObjectAllocator<Alloc>::allocate(Args&&... args) {
    // alloc_ can never be null, never!
    T* obj = nullptr;
    auto p = alloc_->allocate(sizeof(T));
    if (__builtin_expect(!!(p != nullptr), 1)) {
        obj = new (p) T(std::forward<Args>(args)...);
        objects_.emplace_front(obj);
    }
    return obj;
}

using ObjectArenaAllocator = ObjectAllocator<folly::clone::SysArena>;

}  // namespace common
}  // namespace geax

#endif  // GEAXFRONTEND_COMMON_OBJECTALLOCATOR_H_
