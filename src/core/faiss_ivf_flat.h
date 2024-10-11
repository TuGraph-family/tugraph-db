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
#include "core/vector_index.h"
#include "faiss/index_io.h"
#include "faiss/impl/io.h"
#include "faiss/IndexFlat.h"
#include "faiss/IndexIVFFlat.h"

namespace lgraph {

class IVFFlat : public VectorIndex {
  friend class Schema;
  friend class LightningGraph;
  friend class Transaction;
  friend class IndexManager;

  faiss::IndexFlatL2* L2quantizer_;
  faiss::IndexFlatIP* IPquantizer_;
  faiss::IndexIVFFlat* index_;
 public:
  IVFFlat(const std::string& label, const std::string& name,
                const std::string& distance_type, const std::string& index_type,
                int vec_dimension, std::vector<int> index_spec);
  
  IVFFlat(const IVFFlat& rhs) = delete;

  IVFFlat(IVFFlat&& rhs) = delete;

  ~IVFFlat() { index_ = nullptr; }
  
  IVFFlat& operator=(const IVFFlat& rhs) = delete;

  IVFFlat& operator=(IVFFlat&& rhs) = delete;

  // add vector to index and build index
  void Add(const std::vector<std::vector<float>>& vectors,
           const std::vector<int64_t>& vids, int64_t num_vectors) override;
  
  // build index
  void Build() override;
  
  // serialize index
  std::vector<uint8_t> Save() override;
  
  // load index form serialization
  void Load(std::vector<uint8_t>& idx_bytes) override;
  
  // search vector in index
  std::vector<std::pair<int64_t, float>> KnnSearch(
      const std::vector<float>& query, int64_t top_k, int ef_search) override;
  
  std::vector<std::pair<int64_t, float>> RangeSearch(
      const std::vector<float>& query, float radius, int ef_search, int limit) override;
  
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