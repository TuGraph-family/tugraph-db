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

#include <rocksdb/utilities/write_batch_with_index.h>

#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "ftindex/include/lib.rs.h"
#include "transaction/transaction.h"

using namespace txn;
using namespace boost::endian;
namespace graphdb {
void VertexPropertyIndex::AddIndex(Transaction* txn, int64_t vid,
                                   rocksdb::Slice value) {
    if (meta_.is_unique()) {
        rocksdb::ReadOptions ro;
        std::string exists_val;
        // lock index

        std::string index_key = IndexKey(value.ToString());
        auto s = txn->dbtxn()->GetForUpdate(ro, cf_, index_key, &exists_val);
        if (s.ok()) {
            THROW_CODE(IndexValueAlreadyExist);
        } else if (!s.IsNotFound()) {
            THROW_CODE(StorageEngineError, s.ToString());
        }
        rocksdb::Slice index_val((const char*)&vid, sizeof(vid));
        s = txn->dbtxn()->GetWriteBatch()->Put(cf_, index_key, index_val);
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
        std::string index_key = IndexKey(new_value.ToString());
        auto s = txn->dbtxn()->GetForUpdate(ro, cf_, index_key, &tmp);
        if (s.ok()) {
            THROW_CODE(IndexValueAlreadyExist);
        } else if (!s.IsNotFound()) {
            THROW_CODE(StorageEngineError, s.ToString());
        }
        if (old_value) {
            // lock index
            std::string key = IndexKey(*old_value);
            s = txn->dbtxn()->GetForUpdate(ro, cf_, key, (std::string*)nullptr);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            s = txn->dbtxn()->GetWriteBatch()->SingleDelete(cf_, key);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        }
        s = txn->dbtxn()->GetWriteBatch()->Put(
            cf_, index_key, rocksdb::Slice((const char*)&vid, sizeof(vid)));
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

std::string VertexPropertyIndex::IndexKey(const std::string& val) {
    std::string index_key((const char*)&index_id_, sizeof(index_id_));
    index_key.append(val);
    return index_key;
}

void VertexPropertyIndex::DeleteIndex(Transaction* txn, rocksdb::Slice value) {
    if (meta_.is_unique()) {
        rocksdb::ReadOptions ro;
        // lock index
        std::string index_key = IndexKey(value.ToString());
        auto s = txn->dbtxn()->GetForUpdate(ro, cf_, index_key,
                                            (std::string*)nullptr);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        s = txn->dbtxn()->GetWriteBatch()->SingleDelete(cf_, index_key);
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
        ApplyWAL();
        StartTimer();
    });
}

VertexFullTextIndex::VertexFullTextIndex(
    rocksdb::TransactionDB* db, boost::asio::io_service& service,
    GraphCF* graph_cf, IdGenerator* id_generator,
    meta::VertexFullTextIndex meta, uint32_t index_id,
    const std::unordered_set<uint32_t>& lids,
    const std::unordered_set<uint32_t>& pids, size_t commit_interval)
    : db_(db),
      graph_cf_(graph_cf),
      id_generator_(id_generator),
      meta_(std::move(meta)),
      index_id_(index_id),
      lids_(lids),
      pids_(pids),
      interval_(commit_interval),
      timer_(service) {
    ::rust::Vec<::rust::String> fields;
    for (auto& prop : meta_.properties()) {
        fields.push_back(prop);
    }
    instance_ = std::make_unique<::rust::Box<::FTIndex>>(
        new_ftindex(meta_.path(), fields));
    ft_index_ = instance_->operator->();
    auto payload = ft_get_payload(*ft_index_);
    if (!payload.empty()) {
        apply_id_ = std::stoull(payload.c_str());
    }

    std::string prefix((const char*)&index_id_, sizeof(index_id_));
    prefix.append(8, 0xFF);
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->wal));
    iter->SeekForPrev(prefix);
    if (iter->Valid()) {
        auto key = iter->key();
        if (key.starts_with({(const char*)&index_id_, sizeof(index_id_)})) {
            key.remove_prefix(sizeof(index_id_));
            assert(key.size() == sizeof(uint64_t));
            uint64_t wal_id = *(uint64_t*)key.data();
            next_wal_id_ = big_to_native(wal_id) + 1;
        }
    }
    StartTimer();
}

void VertexFullTextIndex::AddIndex(txn::Transaction* txn, int64_t vid,
                                   const meta::FullTextIndexUpdate& wal) {
    auto s =
        txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->index, IndexKey(vid), {});
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    s = txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->wal, NextWALKey(),
                                           wal.SerializeAsString());
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}

void VertexFullTextIndex::DeleteIndex(txn::Transaction* txn, int64_t vid,
                                      const meta::FullTextIndexUpdate& wal) {
    auto s =
        txn->dbtxn()->GetWriteBatch()->Delete(graph_cf_->index, IndexKey(vid));
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    s = txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->wal, NextWALKey(),
                                           wal.SerializeAsString());
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
}

bool VertexFullTextIndex::IsIndexed(Transaction* txn, int64_t vid) {
    std::string index_key = IndexKey(vid);
    std::string val;
    auto s = txn->dbtxn()->Get({}, graph_cf_->index, index_key, &val);
    if (s.ok()) {
        return true;
    } else if (s.IsNotFound()) {
        return false;
    } else {
        THROW_CODE(StorageEngineError, s.ToString());
    }
}

std::string VertexFullTextIndex::IndexKey(int64_t vid) {
    std::string ret((const char*)&index_id_, sizeof(index_id_));
    ret.append((const char*)&vid, sizeof(vid));
    return ret;
}

std::string VertexFullTextIndex::NextWALKey() {
    std::string ret((const char*)&index_id_, sizeof(index_id_));
    uint64_t wal_id = native_to_big(next_wal_id_++);
    ret.append((const char*)&wal_id, sizeof(wal_id));
    return ret;
}

void VertexFullTextIndex::Load() {
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
                int64_t id = *(int64_t*)key.data();
                auto s = db_->Put({}, graph_cf_->index, IndexKey(id), {});
                if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
                AddVertex(id, fields, values);
                count++;
                if (count == 10000) {
                    Commit("0");
                    count = 0;
                }
            }
        }
    }
    if (count > 0) {
        Commit("0");
    }
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

void VertexFullTextIndex::Commit(const std::string& payload) {
    ft_commit(*ft_index_, payload);
}

void VertexFullTextIndex::ApplyWAL() {
    std::lock_guard<std::mutex> lock(mutex_);
    std::string prefix((const char*)&index_id_, sizeof(index_id_));
    std::string start_key(prefix);
    uint64_t next = big_to_native(apply_id_) + 1;
    native_to_big_inplace(next);
    start_key.append((const char*)&next, sizeof(next));
    int count = 0;
    uint64_t consumed_wal_id = 0;
    rocksdb::WriteBatch delete_batch;
    rocksdb::ReadOptions ro;
    rocksdb::WriteOptions wo;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->wal));
    for (iter->Seek(start_key);
         iter->Valid() && iter->key().starts_with(prefix); iter->Next()) {
        auto key = iter->key();
        delete_batch.Delete(graph_cf_->wal, key.ToString());

        key.remove_prefix(sizeof(index_id_));
        assert(key.size() == sizeof(apply_id_));
        consumed_wal_id = *(uint64_t*)key.data();
        meta::FullTextIndexUpdate update;
        auto val = iter->value();
        auto ret = update.ParseFromArray(val.data(), val.size());
        assert(ret);
        if (update.type() == meta::UpdateType::Add) {
            AddVertex(
                update.id(),
                {std::make_move_iterator(update.mutable_fields()->begin()),
                 std::make_move_iterator(update.mutable_fields()->end())},
                {std::make_move_iterator(update.mutable_values()->begin()),
                 std::make_move_iterator(update.mutable_values()->end())});
        } else {
            DeleteVertex(update.id());
        }
        if (++count == 1000) {
            auto payload = std::to_string(big_to_native(consumed_wal_id));
            Commit(payload);
            LOG_DEBUG("apply {} wal, payload: {}", count, payload);
            count = 0;
            rocksdb::TransactionDBWriteOptimizations two;
            two.skip_concurrency_control = true;
            two.skip_duplicate_key_check = true;
            auto s = db_->Write(wo, two, &delete_batch);
            if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
            delete_batch.Clear();
        }
    }
    if (count > 0) {
        auto payload = std::to_string(big_to_native(consumed_wal_id));
        Commit(payload);
        LOG_DEBUG("apply {} wal, payload: {}", count, payload);
        count = 0;
        rocksdb::TransactionDBWriteOptimizations two;
        two.skip_concurrency_control = true;
        two.skip_duplicate_key_check = true;
        auto s = db_->Write(wo, two, &delete_batch);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        delete_batch.Clear();
    }
    if (consumed_wal_id != 0) {
        apply_id_ = consumed_wal_id;
    }
}

::rust::Vec<::IdScore> VertexFullTextIndex::Query(const std::string& query,
                                                  size_t top_n) {
    return ft_query(*ft_index_, query, QueryOptions{top_n});
}

VertexVectorIndex::VertexVectorIndex(rocksdb::TransactionDB* db,
                                     graphdb::GraphCF* graph_cf, uint32_t lid,
                                     uint32_t pid, meta::VertexVectorIndex meta)
    : db_(db),
      graph_cf_(graph_cf),
      lid_(lid),
      pid_(pid),
      meta_(std::move(meta)) {
    index_ = std::make_unique<embedding::Index>(meta_);
}

int64_t VertexVectorIndex::GetElementsNum() { return index_->GetElementsNum(); }

int64_t VertexVectorIndex::GetMemoryUsage() { return index_->GetMemoryUsage(); }

int64_t VertexVectorIndex::GetDeletedIdsNum() {
    return index_->GetDeleteIdsNum();
}

std::vector<std::pair<int64_t, float>> VertexVectorIndex::KnnSearch(
    const float* query, int top_k, int ef_search) {
    std::vector<std::pair<int64_t, float>> ret;

    std::vector<int64_t> vids(top_k, -1);
    std::vector<float> distances(top_k);
    embedding::SearchParams params;
    embedding::DataSet ds;
    ds.NumElements(1)
        .K(top_k)
        .Data(query)
        .Vids(vids.data())
        .Distances(distances.data());
    params.ef_search = ef_search;

    index_->Search(ds, params);

    for (int i = 0; i < top_k; i++) {
        if (vids[i] >= 0) {
            ret.push_back({vids[i], distances[i]});
        } else {
            break;
        }
    }
    return ret;
}

void VertexVectorIndex::RemoveIfExists(int64_t vid) {
    auto native_id = big_to_native(vid);
    assert(native_id >= 0);
    embedding::DataSet ds;
    ds.Vids(&native_id).NumElements(1);
    index_->Delete(ds);
}

void VertexVectorIndex::Add(int64_t vid, std::unique_ptr<float[]> embedding) {
    auto native_id = big_to_native(vid);
    assert(native_id >= 0);
    embedding::DataSet ds;
    ds.Vids(&native_id).NumElements(1).Data(embedding.get());
    index_->Add(ds);
}

void VertexVectorIndex::Load() {
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->vertex_label_vid));
    SPDLOG_INFO("Begin to load vector index: {}", meta_.name());
    int count = 0;
    rocksdb::Slice prefix((const char*)&lid_, sizeof(lid_));
    for (iter->Seek(prefix); iter->Valid() && iter->key().starts_with(prefix);
         iter->Next()) {
        auto key = iter->key();
        key.remove_prefix(sizeof(uint32_t));
        int64_t vid = *(int64_t*)key.data();
        std::string property_key = key.ToString();
        property_key.append((const char*)&pid_, sizeof(pid_));
        std::string property_val;
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
        std::unique_ptr<float[]> copy(new float[array.size()]);
        for (size_t i = 0; i < array.size(); i++) {
            if (array[i].IsDouble()) {
                copy[i] = static_cast<float>(array[i].AsDouble());
            } else {
                copy[i] = array[i].AsFloat();
            }
        }

        // Change to Batch add later
        Add(vid, std::move(copy));

        count++;
        if (count % 10000 == 0) {
            SPDLOG_INFO("{} vector indexes have been load", count);
        }
    }
    SPDLOG_INFO("End to load vector index: {}, index num: {}", meta_.name(),
                count);
}

void VertexVectorIndex::LoadFromDisk(
    const meta::VectorIndexManifest& manifest) {
    index_->Load(manifest);
    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->vertex_label_vid));
    if (index_->GetPersistentChunks().empty()) {
        return Load();
    }
    SPDLOG_INFO("Begin to load remaining vector index: {}", meta_.name());
    int count = 0;
    // the last persistant chunk hold the largest persisted vid
    int64_t start_vid =
        native_to_big(index_->GetPersistentChunks().back()->GetMaxVId() + 1);
    std::string prefix((const char*)&lid_, sizeof(lid_));
    prefix.append((const char*)&start_vid, sizeof(start_vid));
    for (iter->Seek(prefix); iter->Valid(); iter->Next()) {
        auto key = iter->key();
        // scan until we get a different lid prefix
        uint32_t lid = *(uint32_t*)key.data();
        if (lid != lid_) {
            break;
        }
        key.remove_prefix(sizeof(uint32_t));
        int64_t vid = *(int64_t*)key.data();
        std::string property_key = key.ToString();
        property_key.append((const char*)&pid_, sizeof(pid_));
        std::string property_val;
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

        std::unique_ptr<float[]> copy(new float[array.size()]);
        for (size_t i = 0; i < array.size(); i++) {
            if (array[i].IsDouble()) {
                copy[i] = static_cast<float>(array[i].AsDouble());
            } else {
                copy[i] = array[i].AsFloat();
            }
        }

        // Change to Batch add later
        Add(vid, std::move(copy));

        count++;
        if (count % 10000 == 0) {
            SPDLOG_INFO("{} vector indexes have been load", count);
        }
    }

    SPDLOG_INFO("End to load remaining vector index: {}, index num: {}",
                meta_.name(), count);
}
}  // namespace graphdb
