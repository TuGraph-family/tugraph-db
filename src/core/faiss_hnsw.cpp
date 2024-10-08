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

#include "core/faiss_hnsw.h"

namespace lgraph {
HNSWFlat::HNSWFlat(const std::string& label, const std::string& name,
                           const std::string& distance_type,
                           const std::string& index_type, int vec_dimension,
                           std::vector<int> index_spec,
                           std::shared_ptr<KvTable> table)
    : VectorIndex(label, name, distance_type, index_type,
                    vec_dimension, std::move(index_spec), std::move(table)),
      index_(nullptr) {}

HNSWFlat::HNSWFlat(const HNSWFlat& rhs)
    : VectorIndex(rhs),
      index_(rhs.index_) {}

// add vector to index
bool HNSWFlat::Add(const std::vector<std::vector<float>>& vectors,
                  const std::vector<size_t>& vids, size_t num_vectors) {
    // reduce dimension
    std::vector<float> index_vectors;
    index_vectors.reserve(num_vectors * vec_dimension_);
    for (const auto& vec : vectors) {
        index_vectors.insert(index_vectors.end(), vec.begin(), vec.end());
    }
    if (index_type_ == "HNSW") {
        // train after build quantizer
        index_->add(num_vectors, index_vectors.data());
    } else {
        return false;
    }
    return true;
}

// build index
bool HNSWFlat::Build() {
    index_ = new faiss::IndexHNSWFlat(vec_dimension_, index_spec_[0]);
    if (index_ == nullptr) {
        return false;
    }
    return true;
}

// serialize index
std::vector<uint8_t> HNSWFlat::Save() {
    faiss::VectorIOWriter writer;
    faiss::write_index(index_, &writer, 0);
    return writer.data;
}

// load index form serialization
void HNSWFlat::Load(std::vector<uint8_t>& idx_bytes) {
    faiss::VectorIOReader reader;
    reader.data = idx_bytes;
    index_ = dynamic_cast<faiss::IndexHNSWFlat*>(faiss::read_index(&reader));
}

// search vector in index
bool HNSWFlat::Search(const std::vector<float>& query, size_t num_results,
                          std::vector<float>& distances, std::vector<int64_t>& indices) {
    if (query.empty() || num_results == 0) {
        return false;
    }
    distances.resize(num_results * 1);
    indices.resize(num_results * 1);
    index_->search(1, query.data(), num_results, distances.data(), indices.data());
    return !indices.empty();
}

bool HNSWFlat::GetFlatSearchResult(KvTransaction& txn, const std::vector<float> query,
                                       size_t num_results, std::vector<float>& distances,
                                       std::vector<int64_t>& indices) {
    return true;
}
}  // namespace lgraph