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

#include "embedding/mem_index_chunk.h"

#include <faiss/impl/AuxIndexStructures.h>
#include <faiss/impl/FaissException.h>
#include <faiss/index_factory.h>

#include "common/exceptions.h"
#include "embedding/faiss_internal.h"
#include "embedding/index.h"

namespace graphdb {

namespace embedding {

namespace {

// TODO(anyone): replace this with a configurable parameter
const static int64_t MAX_MEM_CHUNK_ITEM_CNT = 100000;

}  // namespace

class MemIndexIdSelector : public IdSelector {
   public:
    MemIndexIdSelector(const std::vector<int64_t>& rowid_vec)
        : rowid_vec_(rowid_vec) {}
    ~MemIndexIdSelector() override = default;

    bool is_member(int64_t id) const override { return rowid_vec_[id] != -1; }
    bool is_filtered(int64_t id) const override { return rowid_vec_[id] == -1; }

   private:
    const std::vector<int64_t>& rowid_vec_;
};

MemIndexChunk::MemIndexChunk(int64_t min_row_id, int64_t dim,
                             meta::VectorIndexType index_type,
                             meta::VectorDistanceType distance_type)
    : min_row_id_(min_row_id),
      max_row_id_(min_row_id),
      dim_(dim),
      index_type_(index_type),
      distance_type_(distance_type) {
    flat_index_.reset(faiss::index_factory(
        (int)dim, "Flat", DistanceTypeToFaissMetricType(distance_type)));
}

int64_t MemIndexChunk::GetEstimatedMemoryUsage() {
    return dim_ * sizeof(float) * row_cnt_ +
           rowid_vec_.capacity() * sizeof(int64_t) * 3;
}

void MemIndexChunk::Add(const DataSet& ds, bool& chunk_full) {
    if (ds.n == 0) {
        return;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    // add embedding vectors to flat index
    flat_index_->add(ds.n, reinterpret_cast<const float*>(ds.x));

    int64_t* rowids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto rowid = rowids[i];
        if (rowid < min_row_id_) {
            THROW_CODE(OutOfRange,
                       "rowid {} should not smaller than min_row_id {}", rowid,
                       min_row_id_);
        }

        rowid_vec_.push_back(rowid);
        if (rowid_labelid_map_.count(rowid)) {
            auto labelId = rowid_labelid_map_[rowid];
            if (rowid_vec_[labelId] != -1) {
                THROW_CODE(InvalidParameter, "rowid {} exists", rowid);
            }
        }
        rowid_labelid_map_[rowid] = row_cnt_++;

        if (rowid > max_row_id_) {
            max_row_id_ = rowid;
        }
    }

    if (row_cnt_ > MAX_MEM_CHUNK_ITEM_CNT) {
        chunk_full = true;
    }
}

void MemIndexChunk::Delete(const DataSet& ds) {
    if (ds.n == 0 || row_cnt_ == 0) {
        return;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);
    int64_t* rowids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto rowid = rowids[i];
        if (rowid_labelid_map_.count(rowid)) {
            rowid_vec_[rowid_labelid_map_[rowid]] = -1;
            ++del_cnt_;
        }
    }
}

void MemIndexChunk::Search(const DataSet& ds) {
    std::shared_lock<std::shared_mutex> lock(mutex_);
    std::unique_ptr<faiss::SearchParameters> faiss_params =
        std::make_unique<faiss::SearchParameters>();

    MemIndexIdSelector id_selector(rowid_vec_);

    std::unique_ptr<FaissIDSelector> faiss_selector =
        std::make_unique<FaissIDSelector>(id_selector);

    faiss_params->sel = faiss_selector.get();

    try {
        flat_index_->search(ds.n, reinterpret_cast<const float*>(ds.x), ds.k,
                            (float*)ds.distances, ds.vids, faiss_params.get());
    } catch (faiss::FaissException& e) {
        THROW_CODE(VectorIndexException, "search failed with exception: {}",
                   e.msg);
    }
}

void MemIndexChunk::RangeSearch(const DataSet& ds, const SearchParams& params) {
    THROW_CODE(VectorIndexException, "TODO: implement range search later");
}

}  // namespace embedding

}  // namespace graphdb
