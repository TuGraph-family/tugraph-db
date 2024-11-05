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

#ifndef GEAXFRONTEND_UTILS_TINYSCOPEGUARD_H_
#define GEAXFRONTEND_UTILS_TINYSCOPEGUARD_H_

#include <functional>

namespace geax::frontend {

class TinyScopeGuard {
public:
  template <class Callable>
  TinyScopeGuard(Callable &&undo_func) try
      : f(std::forward<Callable>(undo_func)) {
  } catch (...) {
    undo_func();
    throw;
  }

  TinyScopeGuard(TinyScopeGuard &&other) : f(std::move(other.f)) {
    other.f = nullptr;
  }

  ~TinyScopeGuard() {
    if (f)
      f(); // must not throw
  }

  void dismiss() noexcept { f = nullptr; }

  TinyScopeGuard(const TinyScopeGuard &) = delete;
  void operator=(const TinyScopeGuard &) = delete;

private:
  std::function<void()> f;
};

} // namespace geax::frontend

#endif  // GEAXFRONTEND_UTILS_TINYSCOPEGUARD_H_
