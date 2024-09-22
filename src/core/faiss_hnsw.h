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
#include "core/vector_index_layer.h"
#include "faiss/index_io.h"
#include "faiss/impl/io.h"
#include "faiss/IndexHNSW.h"

namespace lgraph {

class HNSWFlat : public VectorIndex {
  friend class Schema;
  friend class LightningGraph;
  friend class Transaction;
  friend class IndexManager;

  faiss::IndexHNSWFlat* index_;

 public:
  HNSWFlat(const std::string& label, const std::string& name,
                const std::string& distance_type, const std::string& index_type,
                int vec_dimension, std::vector<int> index_spec, std::shared_ptr<KvTable> table);

  HNSWFlat(const HNSWFlat& rhs);

  HNSWFlat(HNSWFlat&& rhs) = delete;

  ~HNSWFlat() { delete index_; }

  HNSWFlat& operator=(const HNSWFlat& rhs) = delete;

  HNSWFlat& operator=(HNSWFlat&& rhs) = delete;

  // add vector to index and build index
  bool Add(const std::vector<std::vector<float>>& vectors,
           const std::vector<size_t>& vids, size_t num_vectors) override;

  // build index
  bool Build() override;

  // serialize index
  std::vector<uint8_t> Save() override;

  // load index form serialization
  void Load(std::vector<uint8_t>& idx_bytes) override;

  // search vector in index
  bool Search(const std::vector<float>& query, size_t num_results,
              std::vector<float>& distances, std::vector<int64_t>& indices) override;
              
  bool GetFlatSearchResult(KvTransaction& txn, const std::vector<float> query,
                           size_t num_results, std::vector<float>& distances,
                           std::vector<int64_t>& indices) override;
};
}  // namespace lgraph