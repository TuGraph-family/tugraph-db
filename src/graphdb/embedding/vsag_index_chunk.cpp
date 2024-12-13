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

#include <boost/endian/conversion.hpp>
#include <filesystem>
#include <fstream>
#include <nlohmann/json.hpp>

#include "common/exceptions.h"
#include "graphdb/graph_cf.h"
#include "index.h"
#include "index_chunk.h"
#include "transaction/transaction.h"

namespace graphdb {
namespace embedding {

using namespace boost::endian;

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
    VsagHNSWIndexChunk(rocksdb::TransactionDB* db, GraphCF* graph_cf,
                       const meta::VertexVectorIndex& meta,
                       std::string chunk_id, const std::string& parameters)
        : IndexChunk(db, graph_cf, meta), chunk_id_(std::move(chunk_id)) {
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

    void Seal() override;

    int64_t ApplyDelta() override;

    static std::unique_ptr<VsagHNSWIndexChunk> Load(
        rocksdb::TransactionDB* db, GraphCF* graph_cf,
        const meta::VertexVectorIndex& meta, const std::string& chunk_id,
        const std::string& parameters, int64_t& timestamp);

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
        delta_ =
            std::make_unique<Delta>(meta_.dimensions(), meta_.distance_type());
    }

    delta_->Add(ds);

    // refactor later using batch write
    rocksdb::WriteOptions wo;
    std::string key = chunk_id_;
    int64_t bigendian_tm = native_to_big(ds.timestamp);
    key.append((const char*)&bigendian_tm, sizeof(int64_t));
    meta::VectorIndexUpdate update;
    update.set_type(meta::UpdateType::Add);
    update.add_ids(ds.vids[0]);
    auto s = db_->Put(wo, graph_cf_->vector_index_delta, key,
                      update.SerializeAsString());
}

void VsagHNSWIndexChunk::AddBatch(const DataSet& ds) {
    if (ds.n == 0) {
        return;
    }

    // Add should only be used at compaction, so no need to hold lock;
    // std::unique_lock<std::shared_mutex> lock(mutex_);
    std::vector<int64_t> labels;
    labels.reserve(ds.n);

    int64_t* vids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto vid = vids[i];
        labels.push_back(mapper_->Add(vid));
    }

    auto dataset = vsag::Dataset::Make();

    dataset->Dim(meta_.dimensions())
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
    assert(ds.timestamp > 0);

    std::unique_lock<std::shared_mutex> lock(mutex_);

    if (mapper_->GetVIdCnt() == 0) {
        return;
    }

    if (del_map_ == nullptr) {
        del_map_ = std::make_unique<DeleteMap>(GetElementsNum());
    }

    int64_t vid = ds.vids[0];
    auto position = mapper_->GetLabelFromVId(vid);
    if (!del_map_->SetDelete(position)) {
        delta_->Delete(ds);
    }

    // refactor later using batch write
    rocksdb::WriteOptions wo;
    std::string key = chunk_id_;
    int64_t bigendian_tm = native_to_big(ds.timestamp);
    key.append((const char*)&bigendian_tm, sizeof(int64_t));
    meta::VectorIndexUpdate update;
    update.set_type(meta::UpdateType::Delete);
    update.add_ids(vid);
    auto s = db_->Put(wo, graph_cf_->vector_index_delta, key,
                      update.SerializeAsString());
}

void VsagHNSWIndexChunk::Search(const DataSet& ds, const SearchParams& params) {
    if (ds.n == 0) {
        return;
    }

    auto dataset = vsag::Dataset::Make();
    dataset->Dim(meta_.dimensions())->NumElements(1)->Owner(false);
    auto parameters =
        nlohmann::json{{"hnsw", {{"ef_search", params.ef_search.value()}}}}
            .dump();

    std::shared_lock<std::shared_mutex> lock(mutex_);
    VsagIdSelector id_selector(del_map_.get());

    for (size_t i = 0; i < ds.n; i++) {
        const float* query =
            reinterpret_cast<const float*>(ds.x) + meta_.dimensions() * i;
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
    std::filesystem::path target_path = fmt::format(
        "{}/{}.{}", meta_.index_dir(), chunk_id_, vsag_hnsw_index_postfix);

    std::filesystem::path tempFilePath =
        std::filesystem::temp_directory_path() /
        fmt::format("{}.{}", chunk_id_, vsag_hnsw_index_postfix);

    std::ofstream tmpFile(target_path, std::ios::binary);
    index_->Serialize(tmpFile);

    tmpFile.flush();
    tmpFile.close();

    std::filesystem::rename(tempFilePath, target_path);
}

void VsagHNSWIndexChunk::Seal() {
    WriteToFile();
    mapper_->WriteToFile(meta_.index_dir());
}

int64_t VsagHNSWIndexChunk::ApplyDelta() {
    if (del_map_ == nullptr) {
        del_map_ = std::make_unique<DeleteMap>(mapper_->GetVIdCnt());
    }
    if (delta_ == nullptr) {
        delta_ =
            std::make_unique<Delta>(meta_.dimensions(), meta_.distance_type());
    }
    int64_t max_timestamp = 0;
    int64_t timestamp;
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->vector_index_delta));
    std::set<int64_t> deletes;
    std::set<int64_t> updates;
    for (iter->Seek(chunk_id_);
         iter->Valid() && iter->key().starts_with(chunk_id_); iter->Next()) {
        auto key = iter->key();
        key.remove_prefix(chunk_id_.size());
        assert(key.size() == sizeof(int64_t));
        timestamp = *(int64_t*)key.data();
        if (timestamp > max_timestamp) {
            max_timestamp = timestamp;
        }
        auto val = iter->value();
        meta::VectorIndexUpdate update;
        auto ret = update.ParseFromArray(val.data(), val.size());
        assert(ret);
        if (update.type() == meta::UpdateType::Add) {
            for (auto vid : update.ids()) {
                updates.insert(vid);
            }
        } else {
            for (auto vid : update.ids()) {
                deletes.insert(vid);
            }
        }
    }

    if (!deletes.empty()) {
        for (auto vid : deletes) {
            del_map_->SetDelete(mapper_->GetLabelFromVId(vid));
        }
    }

    // the vid in updates can already been deleted
    if (!updates.empty()) {
        std::vector<int64_t> vids;
        vids.reserve(updates.size());
        DataSet ds;
        std::vector<float> embeddings;
        embeddings.reserve(updates.size() * meta_.dimensions());

        rocksdb::ReadOptions ro;
        std::string property_key;
        std::string property_val;
        int64_t bigendian_vid;
        uint32_t bigendian_pid = native_to_big(meta_.property_id());
        for (auto vid : updates) {
            bigendian_vid = native_to_big(vid);
            property_key.clear();
            property_val.clear();
            property_key.append((const char*)&bigendian_vid,
                                sizeof(bigendian_vid));
            property_key.append((const char*)&bigendian_pid,
                                sizeof(bigendian_pid));
            auto s = db_->Get(ro, graph_cf_->vertex_property, property_key,
                              &property_val);
            if (s.IsNotFound()) {
                continue;
            } else if (!s.ok()) {
                THROW_CODE(StorageEngineError, s.ToString());
            }
            Value pv;
            pv.Deserialize(property_val.data(), property_val.size());
            if (!pv.IsArray()) {
                continue;
            }
            auto& array = pv.AsArray();
            if (array.empty() || array.size() != meta_.dimensions()) {
                continue;
            }

            if (!array[0].IsDouble() && !array[0].IsFloat()) {
                continue;
            }

            for (size_t i = 0; i < array.size(); i++) {
                if (array[i].IsDouble()) {
                    embeddings.push_back(
                        static_cast<float>(array[i].AsDouble()));
                } else {
                    embeddings.push_back(array[i].AsFloat());
                }
            }
            ds.n++;
            vids.push_back(vid);
        }

        ds.Vids(vids.data()).Data(embeddings.data());

        delta_->AddBatch(ds);
    }

    return max_timestamp;
}

std::unique_ptr<VsagHNSWIndexChunk> VsagHNSWIndexChunk::Load(
    rocksdb::TransactionDB* db, GraphCF* graph_cf,
    const meta::VertexVectorIndex& meta, const std::string& chunk_id,
    const std::string& parameters, int64_t& timestamp) {
    std::unique_ptr<VsagHNSWIndexChunk> index_chunk =
        std::make_unique<VsagHNSWIndexChunk>(db, graph_cf, meta, chunk_id,
                                             parameters);

    auto file_path = fmt::format("{}/{}.{}", meta.index_dir(), chunk_id,
                                 vsag_hnsw_index_postfix);
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

    std::unique_ptr<IdMapper> mapper =
        IdMapper::Load(meta.index_dir(), chunk_id);
    if (mapper == nullptr) {
        THROW_CODE(InvalidParameter, "IdMapper for chunk id {} not exists",
                   chunk_id);
    }
    index_chunk->mapper_.reset(mapper.release());

    // construct DeletaMap and Delta from rocksdb
    timestamp = index_chunk->ApplyDelta();

    return index_chunk;
}

std::unique_ptr<IndexChunk> CreateVsagIndexChunk(
    rocksdb::TransactionDB* db, GraphCF* graph_cf,
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
                db, graph_cf, meta, chunk_id, index_params.dump());
        }
        default:
            THROW_CODE(VectorIndexException, "index type {} not supported",
                       meta::VectorIndexType_Name(meta.index_type()));
    }
}

std::unique_ptr<IndexChunk> LoadVsagIndexChunk(
    rocksdb::TransactionDB* db, GraphCF* graph_cf,
    const meta::VertexVectorIndex& meta, const std::string& chunk_id,
    int64_t& timestamp) {
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

            return VsagHNSWIndexChunk::Load(db, graph_cf, meta, chunk_id,
                                            index_params.dump(), timestamp);
        }
        default:
            THROW_CODE(VectorIndexException, "index type {} not supported",
                       meta::VectorIndexType_Name(meta.index_type()));
    }
}

}  // namespace embedding

}  // namespace graphdb
