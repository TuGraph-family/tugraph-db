/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef GEAXFRONTEND_COMMON_FOLLYMEMORY_H_
#define GEAXFRONTEND_COMMON_FOLLYMEMORY_H_

#include <type_traits>
#include <cstddef>
#include <stdexcept>

#include "geax-front-end/common/FollyMalloc.h"

namespace folly::clone {

/**
 * SysAllocator
 *
 * Resembles std::allocator, the default Allocator, but wraps std::malloc and
 * std::free.
 */
template <typename T>
class SysAllocator {
 private:
  using Self = SysAllocator<T>;

 public:
  using value_type = T;

  constexpr SysAllocator() = default;

  constexpr SysAllocator(SysAllocator const&) = default;

  template <typename U, std::enable_if_t<!std::is_same<U, T>::value, int> = 0>
  constexpr SysAllocator(SysAllocator<U> const&) noexcept {}

  T* allocate(size_t count) {
    auto const p = std::malloc(sizeof(T) * count);
    if (!p) {
      throw std::bad_alloc();
    }
    return static_cast<T*>(p);
  }
  void deallocate(T* p, size_t count) { sizedFree(p, count * sizeof(T)); }

  friend bool operator==(Self const&, Self const&) noexcept { return true; }
  friend bool operator!=(Self const&, Self const&) noexcept { return false; }
};

} // namespace folly::clone

#endif  // GEAXFRONTEND_COMMON_FOLLYMEMORY_H_
