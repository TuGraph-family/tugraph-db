//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include <atomic>

namespace fma_common {
template <typename T>
T AtomicLoad(const std::atomic<T>& d) {
    return d.load(std::memory_order_acquire);
}

template <typename T, typename D>
void AtomicStore(std::atomic<T>& d, const D& new_value) {
    d.store(new_value, std::memory_order_release);
}

template <typename T>
T AtomicFetchInc(std::atomic<T>& d) {
    return d.fetch_add(1, std::memory_order_acq_rel);
}

template <typename T>
T AtomicFetchDec(std::atomic<T>& d) {
    return d.fetch_sub(1, std::memory_order_acq_rel);
}
}  // namespace fma_common
