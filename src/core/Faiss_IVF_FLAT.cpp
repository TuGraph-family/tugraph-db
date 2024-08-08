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

#include "core/Faiss_IVF_FLAT.h"

namespace lgraph {
FaissIVFFlat::FaissIVFFlat(const std::string& label, const std::string& name,
                           const std::string& distance_type,
                           const std::string& index_type, int vec_dimension,
                           std::vector<int> index_spec,
                           std::shared_ptr<KvTable> table)
    : VectorIndex(label, name, distance_type, index_type,
                    vec_dimension, index_spec, std::move(table)),
      L2quantizer_(nullptr),
      IPquantizer_(nullptr),
      index_(nullptr) {}

FaissIVFFlat::FaissIVFFlat(const FaissIVFFlat& rhs)
    : VectorIndex(rhs),
      L2quantizer_(rhs.L2quantizer_),
      IPquantizer_(rhs.IPquantizer_),
      index_(rhs.index_) {}

// add vector to index
bool FaissIVFFlat::Add(const std::vector<std::vector<float>>& vectors, size_t num_vectors) {
    // reduce dimension
    std::vector<float> index_vectors;
    index_vectors.reserve(num_vectors * vec_dimension_);
    for (const auto& vec : vectors) {
        index_vectors.insert(index_vectors.end(), vec.begin(), vec.end());
    }
    if (index_type_ == "IVF_FLAT") {
        // train after build quantizer
        assert(!index_->is_trained);
        index_->train(num_vectors, index_vectors.data());
        assert(index_->is_trained);
        index_->add(num_vectors, index_vectors.data());
    } else {
        return false;
    }
    return true;
}

// build index
bool FaissIVFFlat::Build() {
    if (distance_type_ == "L2") {
        L2quantizer_ = new faiss::IndexFlatL2(vec_dimension_);
        index_ = new faiss::IndexIVFFlat(L2quantizer_, vec_dimension_, index_spec_[0]);
    } else if (distance_type_ == "IP") {
        IPquantizer_ = new faiss::IndexFlatIP(vec_dimension_);
        index_ = new faiss::IndexIVFFlat(IPquantizer_, vec_dimension_, index_spec_[0]);
    } else {
        return false;
    }
    return true;
}

// serialize index
std::vector<uint8_t> FaissIVFFlat::Save() {
    faiss::VectorIOWriter writer;
    faiss::write_index(index_, &writer, 0);
    return writer.data;
}

// load index form serialization
void FaissIVFFlat::Load(std::vector<uint8_t>& idx_bytes) {
    faiss::VectorIOReader reader;
    reader.data = idx_bytes;
    index_ = dynamic_cast<faiss::IndexIVFFlat*>(faiss::read_index(&reader));
}

// search vector in index
bool FaissIVFFlat::Search(const std::vector<float> query, size_t num_results,
                          std::vector<float>& distances, std::vector<int64_t>& indices) {
    if (query.empty() || num_results == 0) {
        return false;
    }
    distances.resize(num_results * 1);
    indices.resize(num_results * 1);
    index_->nprobe = static_cast<size_t>(query_spec_);
    index_->search(1, query.data(), num_results, distances.data(), indices.data());
    return !indices.empty();
}

bool FaissIVFFlat::GetFlatSearchResult(KvTransaction& txn, const std::vector<float> query,
                                       size_t num_results, std::vector<float>& distances,
                                       std::vector<int64_t>& indices) {
    std::vector<std::vector<float>> floatvector;
    size_t count = 0;
    auto kv_iter = table_->GetIterator(txn);
    for (kv_iter->GotoFirstKey(); kv_iter->IsValid(); kv_iter->Next()) {
        auto prop = kv_iter->GetValue();
        auto vector = prop.AsType<std::vector<float>>();
        floatvector.emplace_back(vector);
        count++;
    }
    std::vector<float> index_vectors;
    index_vectors.reserve(count * vec_dimension_);
    for (auto vec : floatvector) {
        index_vectors.insert(index_vectors.end(), vec.begin(), vec.end());
    }
    std::unique_ptr<faiss::IndexFlat> index;
    if (distance_type_ ==  "IP") {
        index = std::make_unique<faiss::IndexFlatIP>(vec_dimension_);
    } else if (distance_type_ == "L2") {
        index = std::make_unique<faiss::IndexFlatL2>(vec_dimension_);
    } else {
        return false;
    }
    index->add(count, index_vectors.data());
    distances.resize(num_results * 1);
    indices.resize(num_results * 1);
    index->search(1, query.data(), num_results, distances.data(), indices.data());
    return true;
}

}  // namespace lgraph
