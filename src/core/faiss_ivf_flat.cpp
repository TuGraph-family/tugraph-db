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
#include <utility>
#include "core/faiss_ivf_flat.h"
#include "tools/lgraph_log.h"
#include "fma-common/string_formatter.h"
#include "lgraph/lgraph_exceptions.h"

namespace lgraph {
IVFFlat::IVFFlat(const std::string& label, const std::string& name,
                           const std::string& distance_type,
                           const std::string& index_type, int vec_dimension,
                           std::vector<int> index_spec)
    : VectorIndex(label, name, distance_type, index_type,
                    vec_dimension, std::move(index_spec)),
      L2quantizer_(nullptr), IPquantizer_(nullptr), index_(nullptr) {}

// add vector to index
void IVFFlat::Add(const std::vector<std::vector<float>>& vectors,
                  const std::vector<int64_t>& vids, int64_t num_vectors) {
    // reduce dimension
    std::vector<float> index_vectors;
    index_vectors.reserve(num_vectors * vec_dimension_);
    for (const auto& vec : vectors) {
        index_vectors.insert(index_vectors.end(), vec.begin(), vec.end());
    }
    if (index_type_ == "ivf_flat") {
        // train after build quantizer
        assert(!index_->is_trained);
        index_->train(num_vectors, index_vectors.data());
        assert(index_->is_trained);
        index_->add_with_ids(num_vectors, index_vectors.data(), vids.data());
    } else {
        THROW_CODE(InputError, "failed to add vector to index");
    }
}

// build index
void IVFFlat::Build() {
    if (distance_type_ == "l2") {
        L2quantizer_ = new faiss::IndexFlatL2(vec_dimension_);
        index_ = new faiss::IndexIVFFlat(L2quantizer_, vec_dimension_, index_spec_[0]);
    } else if (distance_type_ == "ip") {
        IPquantizer_ = new faiss::IndexFlatIP(vec_dimension_);
        index_ = new faiss::IndexIVFFlat(IPquantizer_, vec_dimension_, index_spec_[0]);
    } else {
        THROW_CODE(InputError, "failed to build vector index");
    }
}

// serialize index
std::vector<uint8_t> IVFFlat::Save() {
    faiss::VectorIOWriter writer;
    faiss::write_index(index_, &writer, 0);
    return writer.data;
}

// load index form serialization
void IVFFlat::Load(std::vector<uint8_t>& idx_bytes) {
    faiss::VectorIOReader reader;
    reader.data = idx_bytes;
    index_ = dynamic_cast<faiss::IndexIVFFlat*>(faiss::read_index(&reader));
}

// search vector in index
std::vector<std::pair<int64_t, float>>
IVFFlat::KnnSearch(const std::vector<float>& query, int64_t top_k, int ef_search) {
    if (query.empty() || top_k == 0) {
        THROW_CODE(InputError, "failed to build vector index");
    }
    std::vector<std::pair<int64_t, float>> ret;
    std::vector<float> distances(top_k);
    std::vector<int64_t> indices(top_k); 
    index_->nprobe = static_cast<size_t>(ef_search);
    index_->search(1, query.data(), top_k, distances.data(), indices.data());
    for (int64_t i = 0; i < top_k; ++i) {
            ret.emplace_back(indices[i], distances[i]);
    }
    return ret;
}

std::vector<std::pair<int64_t, float>>
IVFFlat::RangeSearch(const std::vector<float>& query, float radius, int ef_search, int limit) {
    THROW_CODE(InputError, "not support range search in faiss ivf_flat");
}

}  // namespace lgraph