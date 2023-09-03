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

#ifndef GEAXFRONTEND_COMMON_FOLLYCHECKEDMATH_H_
#define GEAXFRONTEND_COMMON_FOLLYCHECKEDMATH_H_

#include <cstddef>
#include <limits>
#include <type_traits>

namespace folly::clone {
template <typename T, typename = std::enable_if_t<std::is_unsigned<T>::value>>
bool checked_add(T* result, T a, T b) {
  assert(result != nullptr);
  if (__builtin_expect(!!(a < std::numeric_limits<T>::max() - b), 1)) {
    *result = a + b;
    return true;
  } else {
    *result = {};
    return false;
  }
}
} // namespace folly::clone

#endif // GEAXFRONTEND_COMMON_FOLLYCHECKEDMATH_H_
