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

#include "embedding/delete_map.h"
#include "embedding/delta.h"
#include "embedding/id_mapper.h"
#include "proto/meta.pb.h"

namespace graphdb {

namespace embedding {

class DataSet;
class SearchParams;

class ChunkIndexIdSelector : public IdSelector {
   public:
    ChunkIndexIdSelector(const DeleteMap* del_map) : del_map_(del_map) {}

    ~ChunkIndexIdSelector() override = default;

    bool is_member(int64_t id) const override {
        return !del_map_->IsDeleted(id);
    }

    bool is_filtered(int64_t id) const override {
        return del_map_->IsDeleted(id);
    }

   private:
    const DeleteMap* del_map_;  // not own
};

class IndexChunk {
   public:
    IndexChunk(int64_t dim, meta::VectorIndexType index_type,
               meta::VectorDistanceType distance_type)
        : dim_(dim), index_type_(index_type), distance_type_(distance_type) {}

    virtual ~IndexChunk() = default;

    virtual int64_t GetElementsCount() = 0;
    virtual int64_t GetEstimatedMemoryUsage() = 0;

    // Algorithms like ivf need train
    virtual bool IsTrained() const = 0;
    virtual void Train(const DataSet& ds) = 0;

    virtual void Add(const DataSet& ds) = 0;
    // mark delete
    virtual void Delete(const DataSet& ds) = 0;

    virtual void Search(const DataSet& ds, const SearchParams& params) = 0;
    virtual void RangeSearch(const DataSet& ds, const SearchParams& params) = 0;

    virtual void WriteToFile(const std::string& dir) = 0;

   protected:
    int64_t dim_;
    meta::VectorIndexType index_type_;
    meta::VectorDistanceType distance_type_;
    std::unique_ptr<IdMapper> mapper_{nullptr};
    std::unique_ptr<DeleteMap> del_map_{nullptr};
    std::unique_ptr<Delta> delta_{nullptr};
};

extern std::unique_ptr<IndexChunk> CreateVsagIndexChunk(
    const meta::VertexVectorIndex& meta, const std::string& chunk_id);
extern std::unique_ptr<IndexChunk> LoadVsagIndexChunk(
    const meta::VertexVectorIndex meta, const std::string& dir,
    const std::string& chunk_id);

}  // namespace embedding

}  // namespace graphdb