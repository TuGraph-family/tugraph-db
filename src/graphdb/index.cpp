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
*/

//
// Created by botu.wzy
//

#include "index.h"
#include "transaction/transaction.h"
#include "ftindex/include/lib.rs.h"
#include "common/logger.h"
#include <rocksdb/utilities/write_batch_with_index.h>
#include <nlohmann/json.hpp>

using namespace txn;
namespace graphdb {
void VertexPropertyIndex::AddIndex(Transaction* txn, int64_t vid,
                                   rocksdb::Slice value) {
    if (meta_.is_unique()) {
        rocksdb::ReadOptions ro;
        std::string exists_val;
        // lock index
        auto s = txn->dbtxn()->GetForUpdate(ro, cf_, value, &exists_val);
        if (s.ok()) {
            THROW_CODE(IndexValueAlreadyExist);
        } else if (!s.IsNotFound()) {
            THROW_CODE(StorageEngineError, s.ToString());
        }
        rocksdb::Slice index_val((const char*)&vid, sizeof(vid));
        s = txn->dbtxn()->GetWriteBatch()->Put(cf_, value, index_val);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void VertexPropertyIndex::UpdateIndex(Transaction* txn, int64_t vid,
                                      rocksdb::Slice new_value,
                                      const std::string* old_value) {
    if (meta_.is_unique()) {
        std::string tmp;
        rocksdb::ReadOptions ro;
        // lock index
        auto s = txn->dbtxn()->GetForUpdate(ro, cf_, new_value, &tmp);
        if (s.ok()) {
            THROW_CODE(IndexValueAlreadyExist);
        } else if (!s.IsNotFound()) {
            THROW_CODE(StorageEngineError, s.ToString());
        }
        if (old_value) {
            // lock index
            s = txn->dbtxn()->GetForUpdate(ro, cf_, *old_value,
                                           (std::string*)nullptr);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            s = txn->dbtxn()->GetWriteBatch()->SingleDelete(cf_, *old_value);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        }
        s = txn->dbtxn()->GetWriteBatch()->Put(
            cf_, new_value, rocksdb::Slice((const char*)&vid, sizeof(vid)));
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void VertexPropertyIndex::DeleteIndex(Transaction* txn, rocksdb::Slice value) {
    if (meta_.is_unique()) {
        rocksdb::ReadOptions ro;
        // lock index
        auto s =
            txn->dbtxn()->GetForUpdate(ro, cf_, value, (std::string*)nullptr);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        s = txn->dbtxn()->GetWriteBatch()->SingleDelete(cf_, value);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

void VertexFullTextIndex::StartTimer() {
    timer_.expires_after(std::chrono::seconds(interval_));
    timer_.async_wait([this](const boost::system::error_code& e) {
        if (e) {
            LOG_ERROR("timer async_wait error: {}", e.message());
            return;
        }
        if (auto_commit_) {
            Commit();
        }
        StartTimer();
    });
}

VertexFullTextIndex::VertexFullTextIndex(rocksdb::TransactionDB* db,
                                         boost::asio::io_service &service,
                                         GraphCF* graph_cf,
                                         IdGenerator* id_generator,
                                         meta::VertexFullTextIndex meta,
                                         const std::unordered_set<uint32_t>& lids,
                                         const std::unordered_set<uint32_t>& pids,
                                         size_t commit_interval)
    : db_(db), graph_cf_(graph_cf),
      id_generator_(id_generator),
      meta_(std::move(meta)),
      lids_(lids),
      pids_(pids),
      interval_(commit_interval),
      timer_(service) {
    ::rust::Vec<::rust::String> fields;
    for (auto& prop : meta_.properties()) {
        fields.push_back(prop);
    }
    instance_ =
        std::make_unique<::rust::Box<::FTIndex>>(new_ftindex(meta_.path(), fields));
    ft_index_ = instance_->operator->();
    StartTimer();
}

void VertexFullTextIndex::Load() {
    auto_commit_ = false;
    int count = 0;
    for (auto lid : lids_) {
        rocksdb::ReadOptions ro;
        std::unique_ptr<rocksdb::Iterator> iter(
            db_->NewIterator(ro, graph_cf_->vertex_label_vid));
        rocksdb::Slice prefix((const char*)&lid, sizeof(lid));
        for (iter->Seek(prefix);
             iter->Valid() && iter->key().starts_with(prefix); iter->Next()) {
            auto key = iter->key();
            key.remove_prefix(sizeof(uint32_t));
            std::vector<std::string> fields;
            std::vector<std::string> values;
            for (auto pid : pids_) {
                std::string prop_name =
                    id_generator_->GetPropertyName(pid).value();
                std::string property_val;
                std::string property_key = key.ToString();
                property_key.append((const char*)&pid, sizeof(pid));
                auto s = db_->Get(ro, graph_cf_->vertex_property, property_key,
                                  &property_val);
                if (s.IsNotFound()) {
                    continue;
                } else if (!s.ok()) {
                    THROW_CODE(StorageEngineError, s.ToString());
                }
                Value pv;
                pv.Deserialize(property_val.data(), property_val.size());
                if (!pv.IsString()) {
                    continue;
                }
                fields.push_back(prop_name);
                values.push_back(pv.AsString());
            }
            if (!fields.empty()) {
                AddVertex(*(int64_t*)key.data(), fields, values);
                count++;
                if (count == 10000) {
                    Commit();
                    count = 0;
                }
            }
        }
    }
    if (count > 0) {
        Commit();
    }
    auto_commit_ = true;
}

void VertexFullTextIndex::AddVertex(int64_t id, std::vector<std::string> fields,
                                    std::vector<std::string> values) {
    ::rust::Vec<::rust::String> rust_fields;
    ::rust::Vec<::rust::String> rust_values;
    for (auto& item : fields) {
        rust_fields.emplace_back(std::move(item));
    }
    for (auto& item : values) {
        rust_values.emplace_back(std::move(item));
    }
    ft_add_document(*ft_index_, id, rust_fields, rust_values);
}

bool VertexFullTextIndex::MatchLabelIds(
    const std::unordered_set<uint32_t>& lids) const {
    return std::any_of(lids.begin(), lids.end(), [this](uint32_t lid) {
        return lids_.find(lid) != lids_.end();
    });
}

bool VertexFullTextIndex::MatchPropertyIds(
    const std::unordered_set<uint32_t>& pids) const {
    return std::any_of(pids.begin(), pids.end(), [this](uint32_t lid) {
        return pids_.find(lid) != pids_.end();
    });
}

void VertexFullTextIndex::DeleteVertex(int64_t id) {
    ft_delete_document(*ft_index_, id);
}

void VertexFullTextIndex::Commit() {
    std::unique_lock write_lock(mutex_);
    ft_commit(*ft_index_);
}

::rust::Vec<::IdScore> VertexFullTextIndex::Query(const std::string& query,
                                                  size_t top_n) {
    return ft_query(*ft_index_, query, QueryOptions{top_n});
}

VertexVectorIndex::VertexVectorIndex(rocksdb::TransactionDB* db,
                                     graphdb::GraphCF* graph_cf,
                                     uint32_t lid, uint32_t pid,
                                     meta::VertexVectorIndex meta)
    : db_(db), graph_cf_(graph_cf), lid_(lid), pid_(pid), meta_(std::move(meta)) {
    sharding_num_ = meta_.sharding_num();
    for (int i = 0; i < sharding_num_; i++) {
        indexes_.emplace_back(std::make_unique<VsagIndex>(meta_));
    }
}

int64_t VertexVectorIndex::GetElementsNum() {
    int64_t ret = 0;
    for (auto& index : indexes_) {
        ret += index->GetElementsNum();
    }
    return ret;
}

int64_t VertexVectorIndex::GetMemoryUsage() {
    int64_t ret = 0;
    for (auto& index : indexes_) {
        ret += index->GetMemoryUsage();
    }
    return ret;
}

int64_t VertexVectorIndex::GetDeletedIdsNum() {
    int64_t ret = 0;
    for (auto& index : indexes_) {
        ret += index->GetDeletedIdsNum();
    }
    return ret;
}

std::vector<std::pair<int64_t, float>> VertexVectorIndex::KnnSearch(
    const float* query, int top_k, int ef_search) {
    std::vector<std::pair<int64_t, float>> ret;
    for (auto& index : indexes_) {
        auto tmp = index->KnnSearch(query, top_k, ef_search);
        ret.insert(ret.end(), tmp.begin(), tmp.end());
    }
    std::sort(ret.begin(), ret.end(), [](auto& a, auto& b){
        return a.second < b.second;
    });
    if ((int)ret.size() > top_k) {
        ret.resize(top_k);
    }
    return ret;
}

void VertexVectorIndex::RemoveIfExists(int64_t vid) {
    auto native_id = boost::endian::big_to_native(vid);
    assert(native_id >= 0);
    indexes_[native_id % sharding_num_]->RemoveIfExists(vid);
}

void VertexVectorIndex::Add(int64_t vid, std::unique_ptr<float[]> embedding) {
    auto native_id = boost::endian::big_to_native(vid);
    assert(native_id >= 0);
    indexes_[native_id % sharding_num_]->Add(vid, std::move(embedding));
}

void VertexVectorIndex::Load() {
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->vertex_label_vid));
    SPDLOG_INFO("Begin to load vector index: {}", meta_.name());
    int count = 0;
    rocksdb::Slice prefix((const char*)&lid_, sizeof(lid_));
    boost::asio::thread_pool pool(sharding_num_);
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto key = iter->key();
        key.remove_prefix(sizeof(uint32_t));
        int64_t vid = *(int64_t*)key.data();
        std::string property_key = key.ToString();
        property_key.append((const char*)&pid_, sizeof(pid_));
        std::string property_val;
        auto s = db_->Get(ro, graph_cf_->vertex_property, property_key, &property_val);
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
        std::unique_ptr<float[]> copy(new float[array.size()]);
        for (size_t i = 0; i < array.size(); i++) {
            if (array[i].IsDouble()) {
                copy[i] = static_cast<float>(array[i].AsDouble());
            } else {
                copy[i] = array[i].AsFloat();
            }
        }
        boost::asio::post(pool, [this, vid, copy = std::move(copy)]() mutable {
            Add(vid, std::move(copy));
        });
        count++;
        if (count % 10000 == 0) {
            SPDLOG_INFO("{} vector indexes have been load", count);
        }
    }
    pool.join();
    SPDLOG_INFO("End to load vector index: {}, index num: {}", meta_.name(), count);
}

VsagIndex::VsagIndex(meta::VertexVectorIndex meta)
    : meta_(std::move(meta)) {
    nlohmann::json hnsw_parameters {
        {"max_degree", meta_.hnsw_m()},
        {"ef_construction", meta_.hnsw_ef_construction()}
    };
    if (meta_.distance_type() != "l2" && meta_.distance_type() != "ip") {
        THROW_CODE(VectorIndexException, "invalid metric_type: {}", meta_.distance_type());
    }
    nlohmann::json index_parameters {
        {"dtype", "float32"},
        {"metric_type", meta_.distance_type()},
        {"dim", meta_.dimensions()},
        {"hnsw", hnsw_parameters}
    };
    auto ret = vsag::Factory::CreateIndex("hnsw", index_parameters.dump());
    if (ret.has_value()) {
        index_ = std::move(ret.value());
    } else {
        THROW_CODE(VectorIndexException, "Failed to create vector index");
    }
}

void VsagIndex::Add(int64_t vid, std::unique_ptr<float[]> embedding) {
    std::unique_lock write_lock(mutex_);
    if (vid_vectorId_.count(vid)) {
        THROW_CODE(VectorIndexException, "vid {} already exists", vid);
    }
    auto* id = new int64_t[1];
    vectorId_++;
    id[0] = vectorId_;
    vid_vectorId_[vid] = vectorId_;
    vectorId_vid_[vectorId_] = {false, vid};
    auto dataset = vsag::Dataset::Make();
    dataset->Dim(meta_.dimensions())->NumElements(1)
        ->Ids(id)->Float32Vectors(embedding.release());
    auto result = index_->Add(dataset);
    if (result.has_value()) {
        if (!result.value().empty()) {
            THROW_CODE(VectorIndexException, "failed to insert {} ids into vector index", result.value().size());
        }
    } else {
        THROW_CODE(VectorIndexException, result.error().message);
    }
}

void VsagIndex::RemoveIfExists(int64_t vid) {
    std::unique_lock write_lock(mutex_);
    auto iter = vid_vectorId_.find(vid);
    if (iter != vid_vectorId_.end()) {
        vectorId_vid_.at(iter->second) = { true, -1};
        vid_vectorId_.erase(iter);
        deleted_vids_num_++;
    }
}

std::vector<std::pair<int64_t, float>> VsagIndex::KnnSearch(
    const float* query, int top_k, int ef_search) {
    auto dataset = vsag::Dataset::Make();
    dataset->Dim(meta_.dimensions())
        ->NumElements(1)
        ->Float32Vectors(query)
        ->Owner(false);
    nlohmann::json parameters{
        {"hnsw", {{"ef_search", ef_search}}},
    };
    std::vector<std::pair<int64_t, float>> ret;
    std::shared_lock read_lock(mutex_);
    auto result = index_->KnnSearch(dataset, top_k, parameters.dump(), [this](int64_t id)->bool {
        return vectorId_vid_.at(id).first;
    });
    if (result.has_value()) {
        for (int64_t i = 0; i < result.value()->GetDim(); ++i) {
            auto vectorId = result.value()->GetIds()[i];
            ret.emplace_back(vectorId_vid_.at(vectorId).second, result.value()->GetDistances()[i]);
        }
    } else {
        THROW_CODE(VectorIndexException, result.error().message);
    }
    return ret;
}

}
