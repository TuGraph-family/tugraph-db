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

#pragma once

#include <cstdint>
#include <optional>

#include "graphdb/graph_cf.h"
#include "graphdb/uuid_generator.h"
#include "index_chunk.h"
#include "mem_index_chunk.h"
#include "proto/meta.pb.h"

namespace graphdb {

namespace embedding {

/**
 * DateSet - Used for both input and output
 */
class DataSet {
   public:
    DataSet() = default;
    ~DataSet() = default;

    size_t d{0};               // dimension
    size_t n{0};               // number of vector elements
    size_t k{0};               // number of extacted vectors
    const void* x{nullptr};    // input: vectors to add or search
    int64_t* vids{nullptr};    // input/output: size n or n * k
    void* distances{nullptr};  // output: pairwise distances, size n * k

    DataSet& Dim(int64_t dim) {
        this->d = dim;
        return *this;
    }

    DataSet& NumElements(int64_t num) {
        this->n = num;
        return *this;
    }

    DataSet& K(int64_t topk) {
        this->k = topk;
        return *this;
    }

    // Use void* here because embedding can have different types
    DataSet& Data(const void* embeddings) {
        this->x = embeddings;
        return *this;
    }

    DataSet& Vids(int64_t* vids) {
        this->vids = vids;
        return *this;
    }

    // Use void* here because distance can have different types
    DataSet& Distances(void* distances) {
        this->distances = distances;
        return *this;
    }
};

class SearchParams {
   public:
    // hnsw
    std::optional<int> ef_search;
    // ivf-flat
    std::optional<int> nprobe;

    // range search
    std::optional<float> radius;
    std::optional<float> range_filter;
};

class IdSelector {
   public:
    IdSelector() = default;
    virtual ~IdSelector() = default;

    virtual bool is_member(int64_t id) const = 0;
    virtual bool is_filtered(int64_t id) const = 0;
};

class Index {
   public:
    Index(rocksdb::TransactionDB* db, GraphCF* graph_cf, uint32_t lid,
          uint32_t pid, meta::VertexVectorIndex& meta)
        : db_(db), graph_cf_(graph_cf), lid_(lid), pid_(pid), meta_(meta) {}

    int64_t GetElementsNum();
    int64_t GetMemoryUsage();
    int64_t GetDeleteIdsNum();

    void Add(const DataSet& ds);
    void Delete(const DataSet& ds);

    void Search(const DataSet& ds, const SearchParams& params);
    void RangeSearch(const DataSet& ds, const SearchParams& params);

    /**
     * load persistent vector index from disk
     */
    void Load(const meta::VectorIndexManifest& manifest);

    /**
     * pick any adjacent chunks and do the compaction
     */
    void Compaction();

    /**
     * purge stale data, implement this later
     */
    void Purge();

    std::vector<std::shared_ptr<IndexChunk>>& GetPersistentChunks() {
        return persist_chunks_;
    }

   private:
    /**
     * The following two function should be called with lock already hold.
     */
    MemIndexChunk* GetTargetMemIndexChunk(int64_t vid);
    IndexChunk* GetTargetPersistIndexChunk(int64_t vid);

   private:
    rocksdb::TransactionDB* db_{nullptr};
    GraphCF* graph_cf_{nullptr};
    uint32_t lid_;
    uint32_t pid_;
    meta::VertexVectorIndex& meta_;
    std::vector<std::string> chunk_ids_;
    std::shared_ptr<MemIndexChunk> ingest_chunk_;
    std::vector<std::shared_ptr<MemIndexChunk>> mem_chunks_;
    std::vector<std::shared_ptr<IndexChunk>> persist_chunks_;
    std::shared_mutex mutex_;
    graphdb::UUIDGenerator uuid_gen_;
};

}  // namespace embedding

}  // namespace graphdb
