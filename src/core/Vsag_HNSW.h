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
#include <unordered_set>
#include <fstream>
#include "tools/json.hpp"
#include "core/vector_index_layer.h"
#include "vsag/vsag.h"

namespace lgraph {

class HNSW : public VectorIndex {
  friend class Schema;
  friend class LightningGraph;
  friend class Transaction;
  friend class IndexManager;

  std::unordered_set<int64_t> delete_ids_ = {};
  std::shared_ptr<vsag::Index> createindex_;
  vsag::Index* index_;

 public:
  HNSW(const std::string& label, const std::string& name,
                const std::string& distance_type, const std::string& index_type,
                int vec_dimension, std::vector<int> index_spec);

  HNSW(const HNSW& rhs);

  HNSW(HNSW&& rhs) = delete;

  ~HNSW() { index_ = nullptr; }

  HNSW& operator=(const HNSW& rhs) = delete;

  HNSW& operator=(HNSW&& rhs) = delete;

  // add vector to index and build index
  bool Add(const std::vector<std::vector<float>>& vectors,
           const std::vector<int64_t>& vids, int64_t num_vectors) override;

  // build index
  bool Build() override;

  // serialize index
  std::vector<uint8_t> Save() override;

  // load index form serialization
  void Load(std::vector<uint8_t>& idx_bytes) override;

  // search vector in index
  bool Search(const std::vector<float>& query, int64_t num_results,
              std::vector<float>& distances, std::vector<int64_t>& indices) override;

  template <typename T>
  static void writeBinaryPOD(std::ostream& out, const T& podRef) {
      out.write((char*)&podRef, sizeof(T));
  }

  template <typename T>
  static void readBinaryPOD(std::istream& in, T& podRef) {
      in.read((char*)&podRef, sizeof(T));
  }
};
}  // namespace lgraph
