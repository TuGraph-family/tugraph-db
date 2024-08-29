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
                    vec_dimension, index_spec, std::move(table)),
      createindex_(nullptr), index_(nullptr) { delete_ids_.clear(); }

HNSW::HNSW(const HNSW& rhs)
    : VectorIndex(rhs),
      delete_ids_(rhs.delete_ids_),
      createindex_(rhs.createindex_), 
      index_(rhs.index_) {}

// add vector to index
bool HNSW::Add(const std::vector<std::vector<float>>& vectors, const std::vector<size_t>& vids, size_t num_vectors) {
    // reduce dimension
    if (num_vectors == 0) {
        for (size_t i = 0; i < vids.size(); i++){
            delete_ids_.insert(static_cast<int64_t>(vids[i]));
        }
        return true;
    }
    std::vector<float> index_vectors(num_vectors * vec_dimension_);
    auto it = index_vectors.begin();
    for (const auto& vec : vectors) {
        it = std::copy(vec.begin(), vec.end(), it);
    }
    std::vector<int64_t> ids(num_vectors);
    for (size_t i = 0; i < num_vectors; i++) {
        ids[i] = static_cast<int64_t>(vids[i]);
    }
    if (index_type_ == "HNSW") {
        auto dataset = vsag::Dataset::Make();
        dataset->Dim(vec_dimension_)->NumElements(num_vectors)->Ids(ids.data())->Float32Vectors(index_vectors.data());
        auto result = index_->Add(dataset);
        return result.has_value();
    }
    return true;
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
        char key_buf[key_len + 1];
        memset(key_buf, 0, key_len + 1);
        file.read(key_buf, key_len);
        keys.push_back(key_buf);
        uint64_t offset = 0;
        readBinaryPOD(file, offset);
        offsets.push_back(offset);
    }
    vsag::ReaderSet bs;
    for (uint64_t i = 0; i < num_keys; ++i) {
        int64_t size = (i + 1 == num_keys) ? footer_offset : offsets[i + 1];
        size -= (offsets[i] + sizeof(uint64_t));
        auto file_reader = vsag::Factory::CreateLocalFileReader(filename, offsets[i] + sizeof(uint64_t), size);
        bs.Set(keys[i], file_reader);
    }
    file.close();
    std::remove(filename.c_str());
    index_->Deserialize(bs);
}

// search vector in index
bool HNSW::Search(const std::vector<float> query, size_t num_results,
                          std::vector<float>& distances, std::vector<int64_t>& indices) {
    if (!index_) {
        return false;
    }
    std::function<bool(int64_t)> delete_filter_ = [this](int64_t id) -> bool {
        return delete_ids_.find(id) == delete_ids_.end();
    };    
    auto dataset = vsag::Dataset::Make();
    dataset->Dim(query.size())
           ->NumElements(1)
           ->Float32Vectors(query.data());
    nlohmann::json parameters{
        {"hnsw", {{"ef_search", query_spec_}}},
    };
    auto result = index_->KnnSearch(dataset, num_results, parameters.dump(), delete_filter_);
    if (result.has_value()) {
        auto ids = result.value()->GetIds();
        auto dists = result.value()->GetDistances();
        indices.assign(ids, ids + result.value()->GetDim());
        distances.assign(dists, dists + result.value()->GetDim());
        return true;
    }
    return false;
}
// no need to implementation
bool HNSW::GetFlatSearchResult(KvTransaction& txn, const std::vector<float> query,
                                       size_t num_results, std::vector<float>& distances,
                                       std::vector<int64_t>& indices) {
    return true;
}

}  // namespace lgraph
