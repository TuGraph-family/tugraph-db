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

#include "core/vector_index_counter.h"

namespace lgraph {

VectorIndexCounter::VectorIndexCounter(size_t count)
    : count_(count),  indexed_(false) {}

VectorIndexCounter::VectorIndexCounter(const VectorIndexCounter& rhs)
    : count_(rhs.count_),
      indexed_(rhs.indexed_) {}


void VectorIndexCounter::addCount() {
    count_++;
}

bool VectorIndexCounter::MakeVectorIndex() {
    indexed_ = true;
    return true;
}

bool VectorIndexCounter::UpdateCount(size_t DataSize) {
    count_ = count_ + DataSize;
    return true;
}

bool VectorIndexCounter::WhetherUpdate() {
    if (count_ >= 10000) {
        count_ = 0;
        return true;
    } else {
        return false;
    }
}

}  // namespace lgraph
