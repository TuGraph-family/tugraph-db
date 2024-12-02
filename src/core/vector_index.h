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

#include <vector>
#include <cstdint>
#include <string>

namespace lgraph {


class VectorIndex {
  friend class Schema;
  friend class LightningGraph;
  friend class Transaction;
  friend class IndexManager;

 protected:
  std::string label_;
  std::string name_;
  std::string distance_type_;
  std::string index_type_;
  int vec_dimension_;
  std::vector<int> index_spec_;

 public:
    VectorIndex(const std::string& label, const std::string& name,
                const std::string& distance_type, const std::string& index_type,
                int vec_dimension, std::vector<int> index_spec);

    VectorIndex(const VectorIndex& rhs);

    VectorIndex(VectorIndex&& rhs) = delete;

    virtual ~VectorIndex() = default;

    VectorIndex& operator=(const VectorIndex& rhs) = delete;

    VectorIndex& operator=(VectorIndex&& rhs) = delete;

    // get label
    std::string GetLabel() { return label_; }

    // get name
    std::string GetName() { return name_; }

    // get distance type
    std::string GetDistanceType() { return distance_type_; }

    // get index type
    std::string GetIndexType() { return index_type_; }

    // get vector dimension
    int GetVecDimension() { return vec_dimension_; }

    // add vector to index and build index
    virtual void Add(const std::vector<std::vector<float>>& vectors,
                 const std::vector<int64_t>& vids) = 0;

    virtual void Remove(const std::vector<int64_t>& vids) = 0;

    virtual void Clear() = 0;

        // serialize index
    virtual std::vector<uint8_t> Save() = 0;

    // load index form serialization
    virtual void Load(std::vector<uint8_t>& idx_bytes) = 0;

    // search vector in index
    virtual std::vector<std::pair<int64_t, float>>
    KnnSearch(const std::vector<float>& query, int64_t top_k, int ef_search) = 0;

    virtual std::vector<std::pair<int64_t, float>>
    RangeSearch(const std::vector<float>& query, float radius, int ef_search, int limit) = 0;

    virtual int64_t GetElementsNum() = 0;
    virtual int64_t GetMemoryUsage() = 0;
    virtual int64_t GetDeletedIdsNum() = 0;
};

struct VectorIndexEntry {
    VectorIndex* index;
    bool add;
    std::vector<int64_t> vids;
    std::vector<std::vector<float>> vectors;
};

}  // namespace lgraph
