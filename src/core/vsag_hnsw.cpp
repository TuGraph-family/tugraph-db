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

#include "core/vsag_hnsw.h"

#include <utility>

namespace lgraph {
HNSW::HNSW(const std::string& label, const std::string& name,
                           const std::string& distance_type,
                           const std::string& index_type, int vec_dimension,
                           std::vector<int> index_spec)
    : VectorIndex(label, name, distance_type, index_type,
                    vec_dimension, std::move(index_spec)),
      createindex_(nullptr), index_(createindex_.get()) {}

HNSW::HNSW(const HNSW& rhs)
    : VectorIndex(rhs),
      createindex_(rhs.createindex_),
      index_(createindex_.get()) {}

// add vector to index
bool HNSW::Add(const std::vector<std::vector<float>>& vectors,
               const std::vector<int64_t>& vids, int64_t num_vectors) {
    // reduce dimension
    if (num_vectors == 0) {
        for (size_t i = 0; i < vids.size(); i++) {
            auto result = index_->Remove(static_cast<int64_t>(vids[i]));
            if (!result.has_value() || !result.value()) {
                return false;
            }
        }
        return true;
    }
    auto* index_vectors = new float[num_vectors * vec_dimension_];
    auto* ids = new int64_t[num_vectors];
    for (int64_t i = 0; i < num_vectors; i++) {
        std::copy(vectors[i].begin(), vectors[i].end(), &index_vectors[i * vec_dimension_]);
    }
    for (int64_t i = 0; i < num_vectors; i++) {
        ids[i] = vids[i];
    }
    if (index_type_ == "HNSW") {
        auto dataset = vsag::Dataset::Make();
        dataset->Dim(vec_dimension_)->NumElements(num_vectors)
               ->Ids(ids)->Float32Vectors(index_vectors);
        auto result = index_->Add(dataset);
        return result.has_value();
    } else {
        delete[] ids;
        delete[] index_vectors;
        return true;
    }
}

bool HNSW::Build() {
    nlohmann::json hnsw_parameters{
        {"max_degree", index_spec_[0]},
        {"ef_construction", index_spec_[1]}
    };
    nlohmann::json index_parameters;
    if (distance_type_ == "L2") {
        index_parameters = {
            {"dtype", "float32"},
            {"metric_type", "l2"},
            {"dim", vec_dimension_},
            {"hnsw", hnsw_parameters}
        };
    } else if (distance_type_ == "IP") {
        index_parameters = {
            {"dtype", "float32"},
            {"metric_type", "ip"},
            {"dim", vec_dimension_},
            {"hnsw", hnsw_parameters}
        };
    } else {
        return false;
    }
    auto temp = vsag::Factory::CreateIndex("hnsw", index_parameters.dump());
    if (temp.has_value()) {
        createindex_ = std::move(temp.value());
        index_ = createindex_.get();
    } else {
        return false;
    }
    return true;
}

// serialize index
std::vector<uint8_t> HNSW::Save() {
    std::vector<uint8_t> blob;
    if (!index_) {
        return blob;
    }
    if (auto bs = index_->Serialize(); bs.has_value()) {
        auto keys = bs->GetKeys();
        std::ofstream file("hnsw.index", std::ios::binary);
        std::vector<uint64_t> offsets;
        uint64_t offset = 0;
        for (const auto& key : keys) {
            vsag::Binary b = bs->Get(key);
            writeBinaryPOD(file, b.size);
            file.write(reinterpret_cast<const char*>(b.data.get()), b.size);
            offsets.push_back(offset);
            offset += sizeof(b.size) + b.size;
        }
        for (uint64_t i = 0; i < keys.size(); ++i) {
            const auto& key = keys[i];
            int64_t len = key.length();
            writeBinaryPOD(file, len);
            file.write(key.c_str(), len);
            writeBinaryPOD(file, offsets[i]);
        }
        writeBinaryPOD(file, keys.size());
        writeBinaryPOD(file, offset);
        file.close();
        std::ifstream input_file("hnsw.index", std::ios::binary | std::ios::ate);
        if (input_file.is_open()) {
            std::streamsize size = input_file.tellg();
            input_file.seekg(0, std::ios::beg);
            blob.resize(size);
            input_file.read(reinterpret_cast<char*>(blob.data()), size);
            input_file.close();
        }
    }
    std::remove("hnsw.index");
    return blob;
}

// load index form serialization
void HNSW::Load(std::vector<uint8_t>& idx_bytes) {
    const std::string filename = "hnsw.index";
    std::ofstream output_file(filename, std::ios::binary);
    output_file.write(reinterpret_cast<const char*>(idx_bytes.data()), idx_bytes.size());
    output_file.close();
    std::ifstream file(filename, std::ios::in | std::ios::binary);
    if (!file.is_open()) {
        return;
    }
    file.seekg(-static_cast<int>(sizeof(uint64_t) * 2), std::ios::end);
    if (file.fail()) {
        return;
    }
    uint64_t num_keys = 0, footer_offset = 0;
    readBinaryPOD(file, num_keys);
    readBinaryPOD(file, footer_offset);
    if (num_keys == 0 || footer_offset == 0) {
        return;
    }
    file.seekg(footer_offset, std::ios::beg);
    std::vector<std::string> keys;
    std::vector<uint64_t> offsets;
    for (uint64_t i = 0; i < num_keys; ++i) {
        int64_t key_len = 0;
        readBinaryPOD(file, key_len);
        std::vector<char> key_buf(key_len);
        file.read(key_buf.data(), key_len);
        keys.push_back(std::string(key_buf.begin(), key_buf.end()));
        uint64_t offset = 0;
        readBinaryPOD(file, offset);
        offsets.push_back(offset);
    }
    vsag::ReaderSet bs;
    for (uint64_t i = 0; i < num_keys; ++i) {
        int64_t size = (i + 1 == num_keys) ? (footer_offset - offsets[i] - sizeof(uint64_t))
                                       : (offsets[i + 1] - offsets[i] - sizeof(uint64_t));
        auto file_reader = vsag::Factory::CreateLocalFileReader(filename,
                                                    offsets[i] + sizeof(uint64_t), size);
        bs.Set(keys[i], file_reader);
    }
    file.close();
    std::remove(filename.c_str());
    index_->Deserialize(bs);
}

// search vector in index
bool HNSW::Search(const std::vector<float>& query, int64_t num_results,
                          std::vector<float>& distances, std::vector<int64_t>& indices) {
    if (!index_) {
        return false;
    }
    float* query_copy = new float[query.size()];
    std::copy(query.begin(), query.end(), query_copy);
    auto dataset = vsag::Dataset::Make();
    dataset->Dim(vec_dimension_)
           ->NumElements(1)
           ->Float32Vectors(query_copy);
    nlohmann::json parameters{
        {"hnsw", {{"ef_search", query_spec_}}},
    };
    if (index_->GetNumElements() < num_results) {
        return false;
    }
    auto result = index_->KnnSearch(dataset, num_results, parameters.dump());
    if (result.has_value()) {
        for (int64_t i = 0; i < result.value()->GetDim(); ++i) {
            indices.push_back(result.value()->GetIds()[i]);
            distances.push_back(result.value()->GetDistances()[i]);
        }
        return true;
    }
    return false;
}
}  // namespace lgraph
