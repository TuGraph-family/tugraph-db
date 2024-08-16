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

#include "core/Vsag_HNSW.h"

namespace lgraph {
HNSW::HNSW(const std::string& label, const std::string& name,
                           const std::string& distance_type,
                           const std::string& index_type, int vec_dimension,
                           std::vector<int> index_spec,
                           std::shared_ptr<KvTable> table)
    : VectorIndex(label, name, distance_type, index_type,
                    vec_dimension, index_spec, std::move(table)) {}

HNSW::HNSW(const HNSW& rhs)
    : VectorIndex(rhs) {}

// add vector to index
bool HNSW::Add(const std::vector<std::vector<float>>& vectors, size_t num_vectors) {

    return true;
}

// build index
bool HNSW::Build() {
    if (distance_type_ == "L2") {

    } else if (distance_type_ == "IP") {

    } else {
        return false;
    }
    return true;
}

// serialize index
std::vector<uint8_t> HNSW::Save() {
    std::vector<uint8_t> blob;
    return blob;
}

// load index form serialization
void HNSW::Load(std::vector<uint8_t>& idx_bytes) {

}

// search vector in index
bool HNSW::Search(const std::vector<float> query, size_t num_results,
                          std::vector<float>& distances, std::vector<int64_t>& indices) {

    return true;
}

// no need to implementation
bool HNSW::GetFlatSearchResult(KvTransaction& txn, const std::vector<float> query,
                                       size_t num_results, std::vector<float>& distances,
                                       std::vector<int64_t>& indices) {
    return true;
}

}  // namespace lgraph
