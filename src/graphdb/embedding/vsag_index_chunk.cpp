/**
 * Copyright 2024 AntGroup CO., Ltd.
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
 *
 *  Author:
 *      Junwang Zhao <zhaojunwang.zjw@antgroup.com>
 */

#include <vsag/vsag.h>

#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "common/exceptions.h"
#include "index.h"
#include "index_chunk.h"

namespace graphdb {

namespace embedding {

namespace {
const static std::string vsag_hnsw_index_postfix = "vsag.hnsw";

class VsagIdSelector : public IdSelector {
   public:
    VsagIdSelector(const DeleteMap* del_map) : del_map_(del_map) {}

    ~VsagIdSelector() override = default;

    bool is_member(int64_t id) const override {
        return !del_map_->IsDeleted(id);
    }

    bool is_filtered(int64_t id) const override {
        return del_map_->IsDeleted(id);
    }

   private:
    const DeleteMap* del_map_;  // not own
};
}  // namespace

class VsagHNSWIndexChunk : public IndexChunk {
   public:
    VsagHNSWIndexChunk(const std::string& index_dir, std::string chunk_id,
                       int64_t dim, meta::VectorDistanceType distance_type,
                       const std::string& parameters)
        : IndexChunk(index_dir, dim, meta::HNSW, distance_type),
          chunk_id_(std::move(chunk_id)) {
        mapper_ = std::make_unique<IdMapper>(chunk_id);

        auto ret = vsag::Factory::CreateIndex("hnsw", parameters);
        if (ret.has_value()) {
            index_ = std::move(ret.value());
        } else {
            THROW_CODE(VectorIndexException,
                       "Failed to create vsag hnsw index");
        }
    }

    bool IsTrained() const override { return true; }

    void Train(const DataSet& ds) override {
        // do nothing
        return;
    }

    void Add(const DataSet& ds) override;

    void AddBatch(const DataSet& ds) override;

    void Delete(const DataSet& ds) override;

    void Search(const DataSet& ds, const SearchParams& params) override;

    void RangeSearch(const DataSet& ds, const SearchParams& params) override;

    void WriteToFile() override;

    static std::unique_ptr<VsagHNSWIndexChunk> Load(
        const std::string& dir, const std::string chunk_id, int64_t dim,
        meta::VectorDistanceType distance_type, const std::string& parameters);

    int64_t GetMemoryUsage() override { return index_->GetMemoryUsage(); }

   private:
    std::string chunk_id_;
    std::shared_ptr<vsag::Index> index_;
    std::shared_mutex mutex_;
};

void VsagHNSWIndexChunk::Add(const DataSet& ds) {
    if (ds.n != 1) {
        THROW_CODE(InvalidParameter, "Add one item at a time: {}", ds.n);
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    if (delta_ == nullptr) {
        delta_ = std::make_unique<Delta>(dim_, distance_type_, chunk_id_);
    }

    delta_->Add(ds);
    delta_->Flush(index_dir_);
}

void VsagHNSWIndexChunk::AddBatch(const DataSet& ds) {
    if (ds.n == 0) {
        return;
    }

    // Add should only be used at compaction, so no need to hold lock;
    // std::unique_lock<std::shared_mutex> lock(mutex_);
    std::vector<int64_t> labels;
    labels.reserve(ds.n);

    int64_t* rowids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto rowid = rowids[i];
        labels.push_back(mapper_->Add(rowid));
    }

    auto dataset = vsag::Dataset::Make();

    dataset->Dim(dim_)
        ->NumElements(ds.n)
        ->Ids(labels.data())
        ->Float32Vectors(reinterpret_cast<const float*>(ds.x));

    auto result = index_->Add(dataset);
    if (result.has_value()) {
        if (!result.value().empty()) {
            THROW_CODE(VectorIndexException,
                       "failed to insert {} ids into vector index",
                       result.value().size());
        }
    } else {
        THROW_CODE(VectorIndexException, result.error().message);
    }
}

void VsagHNSWIndexChunk::Delete(const DataSet& ds) {
    if (ds.n != 1) {
        THROW_CODE(InvalidParameter, "Add one item at a time: {}", ds.n);
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (mapper_->GetVIdCnt() == 0) {
        return;
    }

    if (del_map_ == nullptr) {
        del_map_ = std::make_unique<DeleteMap>(GetElementsNum(), chunk_id_);
    }

    int64_t vid = ds.vids[0];
    auto position = mapper_->GetLabelFromVId(vid);
    del_map_->SetDelete(position);

    del_map_->Flush(index_dir_);
}

void VsagHNSWIndexChunk::Search(const DataSet& ds, const SearchParams& params) {
    if (ds.n == 0) {
        return;
    }

    auto dataset = vsag::Dataset::Make();
    dataset->Dim(dim_)->NumElements(1)->Owner(false);
    auto parameters =
        nlohmann::json{{"hnsw", {{"ef_search", params.ef_search.value()}}}}
            .dump();

    std::shared_lock<std::shared_mutex> lock(mutex_);
    VsagIdSelector id_selector(del_map_.get());

    for (size_t i = 0; i < ds.n; i++) {
        const float* query = reinterpret_cast<const float*>(ds.x) + dim_ * i;
        auto distances = reinterpret_cast<float*>(ds.distances) + ds.k * i;
        auto vids = ds.vids + ds.k * i;
        dataset->Float32Vectors(query);
        auto result = index_->KnnSearch(dataset, ds.k, parameters,
                                        [&id_selector](int64_t id) -> bool {
                                            return id_selector.is_filtered(id);
                                        });
        if (result.has_value()) {
            for (int64_t j = 0; j < result.value()->GetDim(); j++) {
                auto labelId = result.value()->GetIds()[j];
                vids[j] = mapper_->GetVIdFromLabel(labelId);
            }
            memcpy(distances, result.value()->GetDistances(),
                   result.value()->GetDim() * sizeof(float));
        } else {
            THROW_CODE(VectorIndexException, result.error().message);
        }
    }
}

void VsagHNSWIndexChunk::RangeSearch(const DataSet& ds,
                                     const SearchParams& params) {
    THROW_CODE(VectorIndexException, "TODO: implement range search later");
}

void VsagHNSWIndexChunk::WriteToFile() {
    std::filesystem::path target_path =
        fmt::format("{}/{}.{}", index_dir_, chunk_id_, vsag_hnsw_index_postfix);

    std::filesystem::path tempFilePath =
        std::filesystem::temp_directory_path() /
        fmt::format("{}.{}", chunk_id_, vsag_hnsw_index_postfix);

    std::ofstream tmpFile(target_path, std::ios::binary);
    index_->Serialize(tmpFile);

    tmpFile.flush();
    tmpFile.close();

    std::filesystem::rename(tempFilePath, target_path);
}

std::unique_ptr<VsagHNSWIndexChunk> VsagHNSWIndexChunk::Load(
    const std::string& dir, const std::string chunk_id, int64_t dim,
    meta::VectorDistanceType distance_type, const std::string& parameters) {
    std::unique_ptr<VsagHNSWIndexChunk> index_chunk =
        std::make_unique<VsagHNSWIndexChunk>(dir, chunk_id, dim, distance_type,
                                             parameters);

    auto file_path =
        fmt::format("{}/{}.{}", dir, chunk_id, vsag_hnsw_index_postfix);
    if (!std::filesystem::exists(file_path)) {
        THROW_CODE(InvalidParameter, "index file for chunk id {} not exists",
                   chunk_id);
    }

    std::ifstream is(file_path, std::ios::binary);
    if (!is.is_open()) {
        THROW_CODE(IOException, "Failed to open the file: {}", file_path);
    }

    auto result = index_chunk->index_->Deserialize(is);
    if (!result.has_value()) {
        THROW_CODE(IOException, "Failed to deserize vector index");
    }

    is.close();

    std::unique_ptr<IdMapper> mapper = IdMapper::Load(dir, chunk_id);
    if (mapper == nullptr) {
        THROW_CODE(InvalidParameter, "IdMapper for chunk id {} not exists",
                   chunk_id);
    }
    index_chunk->mapper_.reset(mapper.release());

    std::unique_ptr<DeleteMap> del_map =
        DeleteMap::Load(dir, chunk_id, index_chunk->mapper_->GetVIdCnt());
    if (del_map != nullptr) {
        index_chunk->del_map_.reset(del_map.release());
    }

    std::unique_ptr<Delta> delta =
        Delta::Load(dir, dim, distance_type, chunk_id);
    if (delta != nullptr) {
        index_chunk->delta_.reset(delta.release());
    }

    return index_chunk;
}

std::unique_ptr<IndexChunk> CreateVsagIndexChunk(
    const meta::VertexVectorIndex& meta, const std::string& chunk_id) {
    switch (meta.index_type()) {
        case meta::VectorIndexType::HNSW: {
            nlohmann::json hnsw_params{
                {"max_degree", meta.hnsw_m()},
                {"ef_construction", meta.hnsw_ef_construction()}};

            auto metric_type =
                meta::VectorDistanceType_Name(meta.distance_type());
            std::transform(metric_type.begin(), metric_type.end(),
                           metric_type.begin(), ::tolower);
            nlohmann::json index_params{{"dtype", "float32"},
                                        {"metric_type", metric_type},
                                        {"dim", meta.dimensions()},
                                        {"hnsw", hnsw_params}};
            return std::make_unique<VsagHNSWIndexChunk>(
                meta.index_dir(), chunk_id, meta.dimensions(),
                meta.distance_type(), index_params.dump());
        }
        default:
            THROW_CODE(VectorIndexException, "index type {} not supported",
                       meta::VectorIndexType_Name(meta.index_type()));
    }
}

std::unique_ptr<IndexChunk> LoadVsagIndexChunk(
    const meta::VertexVectorIndex& meta, const std::string& chunk_id) {
    switch (meta.index_type()) {
        case meta::VectorIndexType::HNSW: {
            nlohmann::json hnsw_params{
                {"max_degree", meta.hnsw_m()},
                {"ef_construction", meta.hnsw_ef_construction()}};

            auto metric_type =
                meta::VectorDistanceType_Name(meta.distance_type());
            std::transform(metric_type.begin(), metric_type.end(),
                           metric_type.begin(), ::tolower);
            nlohmann::json index_params{{"dtype", "float32"},
                                        {"metric_type", metric_type},
                                        {"dim", meta.dimensions()},
                                        {"hnsw", hnsw_params}};

            return VsagHNSWIndexChunk::Load(
                meta.index_dir(), chunk_id, meta.dimensions(),
                meta.distance_type(), index_params.dump());
        }
        default:
            THROW_CODE(VectorIndexException, "index type {} not supported",
                       meta::VectorIndexType_Name(meta.index_type()));
    }
}

}  // namespace embedding

}  // namespace graphdb
