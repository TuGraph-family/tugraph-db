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

#include "common/exceptions.h"

namespace graphdb {

namespace embedding {

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

    if (ds.n != 1) {
        THROW_CODE(InvalidParameter, "Add one item each time, ds.n = {}", ds.n);
    }

    std::shared_lock lock(mutex_);

    auto vid = ds.vids[0];
    if (vid >= ingest_chunk_->GetMinVId()) {
        ingest_chunk_->Add(ds, &ingest_chunk_full);
    } else if ((m_chunk = GetTargetMemIndexChunk(vid)) != nullptr) {
        m_chunk->Add(ds, nullptr);
    } else if ((p_chunk = GetTargetPersistIndexChunk(vid)) != nullptr) {
        p_chunk->Add(ds);
    } else {
        THROW_CODE(InvalidParameter, "Cannot find proper chunk for vid {}",
                   vid);
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
