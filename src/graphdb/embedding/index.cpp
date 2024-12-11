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

#include "index.h"

#include <rocksdb/utilities/write_batch_with_index.h>

#include <boost/endian/conversion.hpp>

#include "common/exceptions.h"
#include "common/logger.h"
#include "transaction/transaction.h"

namespace graphdb {

namespace embedding {

using namespace boost::endian;

int64_t Index::GetElementsNum() {
    std::shared_lock lock(mutex_);

    int64_t num = ingest_chunk_->GetElementsNum();
    for (auto& m_chunk : mem_chunks_) {
        num += m_chunk->GetElementsNum();
    }

    for (auto& p_chunk : persist_chunks_) {
        num += p_chunk->GetElementsNum();
    }

    return num;
}

int64_t Index::GetMemoryUsage() {
    std::shared_lock lock(mutex_);
    int64_t usage = ingest_chunk_->GetMemoryUsage();
    for (auto& m_chunk : mem_chunks_) {
        usage += m_chunk->GetMemoryUsage();
    }

    for (auto& p_chunk : persist_chunks_) {
        usage += p_chunk->GetMemoryUsage();
    }

    return usage;
}

int64_t Index::GetDeleteIdsNum() {
    std::shared_lock lock(mutex_);
    int64_t ret = ingest_chunk_->GetDeleteIdsNum();
    for (auto& m_chunk : mem_chunks_) {
        ret += m_chunk->GetMemoryUsage();
    }

    for (auto& p_chunk : persist_chunks_) {
        ret += p_chunk->GetMemoryUsage();
    }

    return ret;
}

void Index::Add(const DataSet& ds) {
    bool ingest_chunk_full = false;
    MemIndexChunk* m_chunk = nullptr;
    IndexChunk* p_chunk = nullptr;
    MemIndexChunk* original_ingest_chunk = nullptr;

    if (ds.n != 1) {
        THROW_CODE(InvalidParameter, "Add one item each time, ds.n = {}", ds.n);
    }

    std::shared_lock lock(mutex_);

    auto vid = ds.vids[0];
    if (vid >= ingest_chunk_->GetMinVId()) {
        ingest_chunk_->Add(ds, &ingest_chunk_full);
        original_ingest_chunk = ingest_chunk_.get();
    } else if ((m_chunk = GetTargetMemIndexChunk(vid)) != nullptr) {
        m_chunk->Add(ds, nullptr);
    } else if ((p_chunk = GetTargetPersistIndexChunk(vid)) != nullptr) {
        p_chunk->Add(ds);
    } else {
        THROW_CODE(InvalidParameter, "Cannot find proper chunk for vid {}",
                   vid);
    }

    lock.unlock();

    if (ingest_chunk_full) {
        // we need to switch to a injestion chunk
        std::unique_lock u_lock(mutex_);
        if (ingest_chunk_.get() != original_ingest_chunk) {
            // already switch by other Add
            return;
        }

        int64_t next_vid = ingest_chunk_->GetMaxVId() + 1;
        mem_chunks_.push_back(ingest_chunk_);
        ingest_chunk_ = std::make_shared<MemIndexChunk>(
            next_vid, meta_.dimensions(), meta_.index_type(),
            meta_.distance_type());
    }
}

void Index::Delete(const DataSet& ds) {
    MemIndexChunk* m_chunk = nullptr;
    IndexChunk* p_chunk = nullptr;

    if (ds.n != 1) {
        THROW_CODE(InvalidParameter, "Delete one item each time, ds.n = {}",
                   ds.n);
    }

    std::shared_lock lock(mutex_);

    auto vid = ds.vids[0];
    if (vid >= ingest_chunk_->GetMaxVId()) {
        ingest_chunk_->Delete(ds);
    } else if ((m_chunk = GetTargetMemIndexChunk(vid)) != nullptr) {
        m_chunk->Delete(ds);
    } else if ((p_chunk = GetTargetPersistIndexChunk(vid)) != nullptr) {
        p_chunk->Delete(ds);
    } else {
        THROW_CODE(InvalidParameter, "Cannot find proper chunk for vid {}",
                   vid);
    }
}

void Index::Search(const DataSet& ds, const SearchParams& params) {
    if (ds.n != 1) {
        THROW_CODE(InvalidParameter, "Delete one item each time, ds.n = {}",
                   ds.n);
    }

    std::vector<std::pair<int64_t, float>> res;

    std::shared_lock lock(mutex_);

    for (size_t i = 0; i < ds.k; i++) {
        ds.vids[i] = -1;
    }

    // search ingest chunk
    ingest_chunk_->Search(ds);
    for (size_t i = 0; i < ds.k; i++) {
        if (ds.vids[i] != -1) {
            res.emplace_back(ds.vids[i],
                             reinterpret_cast<float*>(ds.distances)[i]);
        }
    }

    // search all mem chunks
    for (auto& m_chunk : mem_chunks_) {
        for (size_t i = 0; i < ds.k; i++) {
            ds.vids[i] = -1;
        }
        m_chunk->Search(ds);
        for (size_t i = 0; i < ds.k; i++) {
            if (ds.vids[i] != -1) {
                res.emplace_back(ds.vids[i],
                                 reinterpret_cast<float*>(ds.distances)[i]);
            }
        }
    }

    // search all persistent chunks
    for (auto& p_chunk : persist_chunks_) {
        for (size_t i = 0; i < ds.k; i++) {
            ds.vids[i] = -1;
        }
        p_chunk->Search(ds, params);
        for (size_t i = 0; i < ds.k; i++) {
            if (ds.vids[i] != -1) {
                res.emplace_back(ds.vids[i],
                                 reinterpret_cast<float*>(ds.distances)[i]);
            }
        }
    }

    std::sort(res.begin(), res.end(),
              [](auto& a, auto& b) { return a.second < b.second; });

    // fill back the results
    for (size_t i = 0; i < std::min(res.size(), (size_t)ds.k); i++) {
        ds.vids[i] = res[i].first;
        reinterpret_cast<float*>(ds.distances)[i] = res[i].second;
    }
}

void Index::RangeSearch(const DataSet& ds, const SearchParams& params) {
    THROW_CODE(VectorIndexException, "TODO: implement range search later");
}

/**
 * Mem chunks should be ordered by the min_vid
 */
MemIndexChunk* Index::GetTargetMemIndexChunk(int64_t vid) {
    for (auto it = mem_chunks_.rbegin(); it != mem_chunks_.rend(); it++) {
        auto& m_chunk = *it;
        if (vid >= m_chunk->GetMinVId()) {
            return m_chunk.get();
        }
    }

    return nullptr;
}

void Index::Load(const meta::VectorIndexManifest& manifest) {
    std::unique_lock lock(mutex_);
    chunk_ids_.reserve(manifest.chunk_ids_size());
    for (auto& chunk_id : manifest.chunk_ids()) {
        chunk_ids_.push_back(chunk_id);
        // adapt other vector lib later
        auto p_chunk = LoadVsagIndexChunk(meta_, chunk_id);
        if (!persist_chunks_.empty()) {
            if (p_chunk->GetMinVId() <= persist_chunks_.back()->GetMaxVId()) {
                THROW_CODE(VectorIndexException, "Manifest corrupted");
            }
        }
        persist_chunks_.push_back(std::move(p_chunk));
    }

    // create a new mem index chunk for data ingestion
    ingest_chunk_ = std::make_shared<MemIndexChunk>(
        persist_chunks_.back()->GetMaxVId() + 1, meta_.dimensions(),
        meta_.index_type(), meta_.distance_type());
}

void Index::Compaction() {
    std::shared_lock lock(mutex_);

    /**
     * only compact single mem chunk for now, consider persistent chunk
     * compaction later
     */
    if (mem_chunks_.empty()) {
        return;
    }

    int64_t start_vid = mem_chunks_.front()->GetMinVId();
    int64_t end_vid = mem_chunks_.front()->GetMaxVId();

    lock.unlock();

    // construct a new IndexChunk
    auto chunk_id = uuid_gen_.NextNoLock();
    auto p_chunk = CreateVsagIndexChunk(meta_, chunk_id);

    rocksdb::ReadOptions ro;
    std::unique_ptr<rocksdb::Iterator> iter(
        db_->NewIterator(ro, graph_cf_->vertex_label_vid));

    SPDLOG_INFO("Begin to compact chunk: {}, [{}, {}]", chunk_id, start_vid,
                end_vid);
    int count = 0;
    std::string prefix((const char*)&lid_, sizeof(lid_));
    prefix.append((const char*)&start_vid, sizeof(start_vid));
    for (iter->Seek(prefix); iter->Valid(); iter->Next()) {
        auto key = iter->key();
        uint32_t lid = *(uint32_t*)key.data();
        if (lid != lid_) {
            break;
        }
        key.remove_prefix(sizeof(uint32_t));
        int64_t vid = *(int64_t*)key.data();
        if (vid > end_vid) {
            break;
        }
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

        // Change to batch add later
        DataSet ds;
        int64_t native_vid = big_to_native(vid);
        ds.NumElements(1).Vids(&native_vid).Data(copy.get());
        p_chunk->AddBatch(ds);

        count++;
        if (count % 10000 == 0) {
            SPDLOG_INFO("{} vector indexes have been load", count);
        }
    }
    SPDLOG_INFO("End to compact chunk_id: {}, vector num: {}", chunk_id, count);

    p_chunk->Seal();

    std::unique_lock u_lock(mutex_);
    chunk_ids_.push_back(chunk_id);
    mem_chunks_.erase(mem_chunks_.begin());
    persist_chunks_.emplace_back(p_chunk.release());

    // update manifest
    meta::VectorIndexManifest manifest;
    for (auto& cid : chunk_ids_) {
        manifest.add_chunk_ids(cid);
    }
    rocksdb::WriteOptions wo;
    auto s = db_->Put(wo, graph_cf_->vector_index_manifest, meta_.index_dir(),
                      manifest.SerializeAsString());
}

/**
 * Persist chunks should be ordered by the min_vid
 */
IndexChunk* Index::GetTargetPersistIndexChunk(int64_t vid) {
    for (auto it = persist_chunks_.rbegin(); it != persist_chunks_.rend();
         it++) {
        auto& p_chunk = *it;
        if (vid > p_chunk->GetMinVId()) {
            return p_chunk.get();
        }
    }

    return nullptr;
}

}  // namespace embedding

}  // namespace graphdb
