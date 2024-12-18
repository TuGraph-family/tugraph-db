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
                    vec_dimension, std::move(index_spec)) {
    Build();
    LOG_INFO() << FMA_FMT("Create IVF_Flat instance, {}:{}", GetLabel(), GetName());
}

IVFFlat::~IVFFlat() {
    LOG_INFO() << FMA_FMT("Destroy IVF_Flat instance, {}:{}", GetLabel(), GetName());
    IPquantizer_ = nullptr;
    L2quantizer_ = nullptr;
    index_ = nullptr;
}

// add vector to index
void IVFFlat::Add(const std::vector<std::vector<float>>& vectors,
                  const std::vector<int64_t>& vids) {
    if (vectors.size() != vids.size()) {
        THROW_CODE(VectorIndexException,
                   "size mismatch, vectors.size:{}, vids.size:{}", vectors.size(), vids.size());
    }
    if (vectors.empty()) {
        return;
    }
    auto num_vectors = vectors.size();
    // reduce dimension
    std::vector<float> index_vectors;
    index_vectors.reserve(num_vectors * vec_dimension_);
    for (const auto& vec : vectors) {
        index_vectors.insert(index_vectors.end(), vec.begin(), vec.end());
    }
    if (!index_->is_trained) {
        // train after build quantizer
        assert(!index_->is_trained);
        index_->train(num_vectors, index_vectors.data());
        assert(index_->is_trained);
        index_->add_with_ids(num_vectors, index_vectors.data(), vids.data());
    } else {
        THROW_CODE(VectorIndexException, "failed to add vector to index");
    }
}

void IVFFlat::Clear() {
    IPquantizer_ = nullptr;
    L2quantizer_ = nullptr;
    index_ = nullptr;
    Build();
}

void IVFFlat::Remove(const std::vector<int64_t>& vids) {
    // not support now
}

// build index
void IVFFlat::Build() {
    if (distance_type_ == "l2") {
        L2quantizer_ = std::make_shared<faiss::IndexFlatL2>(vec_dimension_);
        index_ = std::make_shared<faiss::IndexIVFFlat>
                            (L2quantizer_.get(), vec_dimension_, index_spec_[0]);
    } else if (distance_type_ == "ip") {
        IPquantizer_ = std::make_shared<faiss::IndexFlatIP>(vec_dimension_);
        index_ = std::make_shared<faiss::IndexIVFFlat>
                            (IPquantizer_.get(), vec_dimension_, index_spec_[0]);
    } else {
        THROW_CODE(InputError, "failed to build vector index");
    }
}

// serialize index
std::vector<uint8_t> IVFFlat::Save() {
    faiss::VectorIOWriter writer;
    faiss::write_index(index_.get(), &writer, 0);
    return writer.data;
}

// load index form serialization
void IVFFlat::Load(std::vector<uint8_t>& idx_bytes) {
    faiss::VectorIOReader reader;
    reader.data = idx_bytes;
    auto loadindex = faiss::read_index(&reader);
    index_.reset(dynamic_cast<faiss::IndexIVFFlat*>(loadindex));
}

// search vector in index
std::vector<std::pair<int64_t, float>>
IVFFlat::KnnSearch(const std::vector<float>& query, int64_t top_k, int ef_search) {
    if (query.empty() || top_k == 0) {
        THROW_CODE(InputError, "please check the input");
    }
    std::vector<std::pair<int64_t, float>> ret;
    std::vector<float> distances(top_k);
    std::vector<int64_t> indices(top_k);
    if (index_->ntotal == 0) {
        THROW_CODE(InputError, "there is no indexed vector");
    }
    index_->nprobe = static_cast<size_t>(ef_search);
    index_->search(1, query.data(), top_k, distances.data(), indices.data());
    for (int64_t i = 0; i < top_k; ++i) {
            ret.emplace_back(indices[i], distances[i]);
    }
    return ret;
}

std::vector<std::pair<int64_t, float>>
IVFFlat::RangeSearch(const std::vector<float>& query, float radius, int ef_search, int limit) {
    if (query.empty()) {
        THROW_CODE(InputError, "please check the input");
    }
    std::vector<std::pair<int64_t, float>> ret;
    if (index_->ntotal == 0) {
        THROW_CODE(InputError, "there is no indexed vector");
    }
    index_->nprobe = static_cast<size_t>(ef_search);
    faiss::RangeSearchResult result(1);
    index_->range_search(1, query.data(), radius, &result);
    if (limit != -1) {
        int64_t max = (static_cast<int64_t>(result.lims[1]) < limit) ?
                       static_cast<int64_t>(result.lims[1]) : limit;
        for (int64_t i = 0; i < max; ++i) {
            ret.emplace_back(result.labels[i], result.distances[i]);
        }
    } else {
        for (int64_t i = 0; i < static_cast<int64_t>(result.lims[1]); ++i) {
            ret.emplace_back(result.labels[i], result.distances[i]);
        }
    }
    return ret;
}

int64_t IVFFlat::GetElementsNum() {
    return index_->ntotal;
}

int64_t IVFFlat::GetMemoryUsage() {
    // not support
    return 0;
}

int64_t IVFFlat::GetDeletedIdsNum() {
    // not support
    return 0;
}

}  // namespace lgraph
