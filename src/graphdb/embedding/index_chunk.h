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

#include "delete_map.h"
#include "delta.h"
#include "id_mapper.h"
#include "proto/meta.pb.h"

namespace graphdb {

namespace embedding {

class DataSet;
class SearchParams;

class IndexChunk {
   public:
    IndexChunk(rocksdb::TransactionDB* db, GraphCF* graph_cf,
               const meta::VertexVectorIndex& meta)
        : db_(db), graph_cf_(graph_cf), meta_(meta) {}

    virtual ~IndexChunk() = default;

    virtual int64_t GetElementsNum() { return mapper_->GetVIdCnt(); }
    virtual int64_t GetMemoryUsage() = 0;
    virtual int64_t GetDeleteIdsNum() { return del_map_->GetDeleteCount(); };

    // Algorithms like ivf need train
    virtual bool IsTrained() const = 0;
    virtual void Train(const DataSet& ds) = 0;

    // Add is used for user operation, data goes to delta_
    virtual void Add(const DataSet& ds) = 0;
    // AddBatch is used for compaction
    virtual void AddBatch(const DataSet& ds) = 0;
    // mark delete
    virtual void Delete(const DataSet& ds) = 0;

    // after seal, the updated data should go to delta_
    virtual void Seal() = 0;

    virtual void Search(const DataSet& ds, const SearchParams& params) = 0;
    virtual void RangeSearch(const DataSet& ds, const SearchParams& params) = 0;

    int64_t GetMinVId() { return mapper_->GetMinVId(); }
    int64_t GetMaxVId() { return mapper_->GetMaxVId(); }

    virtual void WriteToFile() = 0;

    // apply wal and return the max timestamp
    virtual int64_t ApplyDelta() = 0;

   protected:
    rocksdb::TransactionDB* db_{nullptr};
    GraphCF* graph_cf_{nullptr};
    const meta::VertexVectorIndex& meta_;
    std::unique_ptr<IdMapper> mapper_{nullptr};
    std::unique_ptr<DeleteMap> del_map_{nullptr};
    std::unique_ptr<Delta> delta_{nullptr};
};

extern std::unique_ptr<IndexChunk> CreateVsagIndexChunk(
    rocksdb::TransactionDB* db, GraphCF* graph_cf,
    const meta::VertexVectorIndex& meta, const std::string& chunk_id);
extern std::unique_ptr<IndexChunk> LoadVsagIndexChunk(
    rocksdb::TransactionDB* db, GraphCF* graph_cf,
    const meta::VertexVectorIndex& meta, const std::string& chunk_id,
    int64_t& timestamp);

}  // namespace embedding

}  // namespace graphdb
