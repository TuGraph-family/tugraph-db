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

#include <fstream>
#include <nlohmann/json.hpp>

#include "common/logger.h"
#include "common/flags.h"
#include "transaction/transaction.h"
#include "spdlog/stopwatch.h"

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
            s = txn->dbtxn()->GetForUpdate(ro, cf_, key,
                                           (std::string*)nullptr);
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
        auto s =
            txn->dbtxn()->GetForUpdate(ro, cf_, index_key, (std::string*)nullptr);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        s = txn->dbtxn()->GetWriteBatch()->SingleDelete(cf_, index_key);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    }
}

// VertexFullTextIndex methods are stubbed out in the header when ftindex is disabled.

VertexVectorIndex::VertexVectorIndex(rocksdb::TransactionDB* db,
                                     boost::asio::io_service &service,
                                     graphdb::GraphCF* graph_cf,
                                     uint32_t index_id,
                                     uint32_t lid, uint32_t pid,
                                     meta::VertexVectorIndex meta,
                                     size_t commit_interval)
    : db_(db), graph_cf_(graph_cf), index_id_(index_id),
      lid_(lid), pid_(pid), meta_(std::move(meta)),
      interval_(commit_interval), timer_(service) {
    // Vector index functionality disabled due to missing vsag dependency.
    // We still initialize bookkeeping state from existing index keys so that
    // metadata remains consistent even though KNN search is not available.
    {
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
        LOG_INFO("vector index {} (vsag disabled), next_wal_id: {}", meta_.name(), next_wal_id_.load());
    }
    {
        spdlog::stopwatch sw;
        int64_t max_vector_id = -1;
        std::string prefix((const char*)&index_id_, sizeof(index_id_));
        rocksdb::ReadOptions ro;
        std::unique_ptr<rocksdb::Iterator> iter(
            db_->NewIterator(ro, graph_cf_->index));
        for (iter->Seek(prefix);
             iter->Valid() && iter->key().starts_with(prefix);
             iter->Next()) {
            auto key = iter->key();
            auto val = iter->value();
            key.remove_prefix(sizeof(index_id_));
            char flag = *key.data();
            key.remove_prefix(1);
            if (flag == 0) {
                assert(key.size() == sizeof(int64_t));
                assert(val.size() == sizeof(int64_t));
                auto vid = *(int64_t*)key.data();
                auto vector_id = *(int64_t*)val.data();
                max_vector_id = std::max(max_vector_id, vector_id);
                vectorid_vid_.emplace(vector_id, vid);
            } else {
                assert(flag == 1);
                assert(key.size() == sizeof(int64_t));
                assert(val.size() == 0);
                auto vector_id = *(int64_t*)key.data();
                max_vector_id = std::max(max_vector_id, vector_id);
                deleted_vector_ids_.emplace(vector_id);
            }
        }
        if (max_vector_id != -1) {
            next_vector_id_ = max_vector_id + 1;
        }
        LOG_INFO("vector index {} (vsag disabled), vectorid_vid size:{}, deleted_vector_ids size:{}, elapsed:{}",
                 meta_.name(), vectorid_vid_.size(), deleted_vector_ids_.size(), sw);
    }
}

void VertexVectorIndex::StartTimer() {
    // Vector WAL apply is disabled when vsag is not available.
}

int64_t VertexVectorIndex::GetElementsNum() {
    // Without vsag, we approximate by the number of live vectors tracked.
    return static_cast<int64_t>(vectorid_vid_.size());
}

int64_t VertexVectorIndex::GetMemoryUsage() {
    // Unknown without the underlying index implementation.
    return 0;
}

int64_t VertexVectorIndex::GetDeletedIdsNum() {
    return 0;
}

std::vector<std::pair<int64_t, float>> VertexVectorIndex::KnnSearch(
    const float* query, int top_k, int ef_search) {
    (void)query;
    (void)top_k;
    (void)ef_search;
    THROW_CODE(VectorIndexException, "Vector index functionality is disabled (vsag not available)");
}

void  VertexVectorIndex::TryDeleteIndex(txn::Transaction* txn, int64_t vid) {
    std::string index_key = IndexKey(vid);
    std::string val;
    auto s = txn->dbtxn()->Get({}, graph_cf_->index, index_key, &val);
    if (s.ok()) {
        assert(val.size() == sizeof(int64_t));
        auto vector_id = *(int64_t*)val.data();
        s = txn->dbtxn()->GetWriteBatch()->Delete(graph_cf_->index, index_key);
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        s = txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->index, DeleteMarkKey(vector_id), {});
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        meta::VectorIndexUpdate wal;
        wal.set_type(meta::UpdateType::Delete);
        wal.set_vector_id(vector_id);
        s = txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->wal, NextWALKey(), wal.SerializeAsString());
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    } else if (!s.IsNotFound()) {
        THROW_CODE(StorageEngineError, s.ToString());
    }
}

std::string VertexVectorIndex::NextWALKey() {
    std::string ret((const char*)&index_id_, sizeof(index_id_));
    uint64_t wal_id = native_to_big(next_wal_id_++);
    ret.append((const char*)&wal_id, sizeof(wal_id));
    return ret;
}

std::string VertexVectorIndex::IndexKey(int64_t vid) {
    char flag = 0;
    std::string ret((const char*)&index_id_, sizeof(index_id_));
    ret.append(1, flag);
    ret.append((const char*)&vid, sizeof(vid));
    return ret;
}

std::string VertexVectorIndex::DeleteMarkKey(int64_t vector_id) {
    char flag = 1;
    std::string ret((const char*)&index_id_, sizeof(index_id_));
    ret.append(1, flag);
    ret.append((const char*)&vector_id, sizeof(vector_id));
    return ret;
}

void VertexVectorIndex::ApplyWAL() {
    // WAL application is a no-op when vsag-based indexing is disabled.
}

void VertexVectorIndex::AddIndex(txn::Transaction* txn, int64_t vid,
                                 meta::VectorIndexUpdate& wal) {
    // vid -> vector id
    auto vector_id = next_vector_id_++;
    auto s = txn->dbtxn()->GetWriteBatch()->Put(
        graph_cf_->index, IndexKey(vid), rocksdb::Slice((const char*)&vector_id, sizeof(vector_id)));
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
    wal.set_vid(vid);
    wal.set_vector_id(vector_id);
    s = txn->dbtxn()->GetWriteBatch()->Put(graph_cf_->wal, NextWALKey(), wal.SerializeAsString());
    if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
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
        std::unique_ptr<float[]> embedding(new float[array.size()]);
        for (size_t i = 0; i < array.size(); i++) {
            if (array[i].IsDouble()) {
                embedding[i] = static_cast<float>(array[i].AsDouble());
            } else {
                embedding[i] = array[i].AsFloat();
            }
        }
        auto vector_id = next_vector_id_++;
        s = db_->Put({}, graph_cf_->index, IndexKey(vid), rocksdb::Slice((const char*)&vector_id, sizeof(vector_id)));
        if (!s.ok()) THROW_CODE(StorageEngineError, s.ToString());
        vectorid_vid_.emplace(vector_id, vid);
        count++;
        if (count % 10000 == 0) {
            SPDLOG_INFO("{} vector indexes have been load", count);
        }
    }
    SPDLOG_INFO("End to load vector index (vsag disabled): {}, index num: {}", meta_.name(), count);
}

}
