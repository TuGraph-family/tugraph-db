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

#include <unordered_map>
#include <mutex>
#include <memory>
#include <string>
#include <vector>

#include "/root/tugraph-db/include/tools/lgraph_log.h"


namespace lgraph {

class VectorIndexCounter {
  size_t count_;
  bool indexed_;

 public:
  explicit VectorIndexCounter(size_t count);

  VectorIndexCounter(const VectorIndexCounter& rhs);

  VectorIndexCounter(VectorIndexCounter&& rhs) = delete;

  VectorIndexCounter& operator=(const VectorIndexCounter& rhs) = delete;

  VectorIndexCounter& operator=(VectorIndexCounter&& rhs) = delete;

  size_t getCount() { return count_; }

  bool isIndexed() { return indexed_; }

  void addCount();

  bool MakeVectorIndex();

  bool UpdateCount(size_t DataSize);

  bool WhetherUpdate();
};
}  // namespace lgraph
