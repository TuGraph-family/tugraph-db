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

#include "mem_index_chunk.h"

#include <faiss/impl/AuxIndexStructures.h>
#include <faiss/impl/FaissException.h>
#include <faiss/index_factory.h>

#include "common/exceptions.h"
#include "delta.h"
#include "faiss_internal.h"
#include "index.h"

namespace graphdb {

namespace embedding {

namespace {

// TODO(anyone): replace this with a configurable parameter
const static int64_t MAX_MEM_CHUNK_ITEM_CNT = 100000;

faiss::MetricType DistanceTypeToFaissMetricType(
    meta::VectorDistanceType distance_type) {
    switch (distance_type) {
        case meta::VectorDistanceType::L2:
            return faiss::MetricType::METRIC_L2;
        // cosine can be converted to IP, so faiss do not have COSINE metric
        case meta::VectorDistanceType::IP:
        case meta::VectorDistanceType::COSINE:
            return faiss::MetricType::METRIC_INNER_PRODUCT;
        default:
            THROW_CODE(VectorIndexException, "index type {} not supported",
                       meta::VectorDistanceType_Name(distance_type));
    }
}

}  // namespace

class MemIndexIdSelector : public IdSelector {
   public:
    MemIndexIdSelector(const std::vector<int64_t>& vid_vec)
        : vid_vec_(vid_vec) {}
    ~MemIndexIdSelector() override = default;

    bool is_member(int64_t id) const override { return vid_vec_[id] != -1; }
    bool is_filtered(int64_t id) const override { return vid_vec_[id] == -1; }

   private:
    const std::vector<int64_t>& vid_vec_;
};

MemIndexChunk::MemIndexChunk(int64_t min_vid, int64_t dim,
                             meta::VectorIndexType index_type,
                             meta::VectorDistanceType distance_type)
    : min_vid_(min_vid),
      max_vid_(min_vid),
      dim_(dim),
      index_type_(index_type),
      distance_type_(distance_type) {
    flat_index_.reset(faiss::index_factory(
        (int)dim, "Flat", DistanceTypeToFaissMetricType(distance_type)));
}

int64_t MemIndexChunk::GetMemoryUsage() {
    return dim_ * sizeof(float) * vid_cnt_ +
           vid_vec_.capacity() * sizeof(int64_t) * 3;
}

void MemIndexChunk::Add(const DataSet& ds, bool* chunk_full) {
    assert(ds.n == 1);

    std::unique_lock lock(mutex_);

    if (!sealed) {
        // add embedding vectors to flat index
        flat_index_->add(ds.n, reinterpret_cast<const float*>(ds.x));

        auto vid = ds.vids[0];
        if (vid < min_vid_) {
            THROW_CODE(OutOfRange, "vid {} should not smaller than min_vid {}",
                       vid, min_vid_);
        }

        vid_vec_.push_back(vid);
        if (vid_labelid_map_.count(vid)) {
            auto labelId = vid_labelid_map_[vid];
            if (vid_vec_[labelId] != -1) {
                THROW_CODE(InvalidParameter, "vid {} already exists", vid);
            }
        }
        vid_labelid_map_[vid] = vid_cnt_++;

        if (vid > max_vid_) {
            max_vid_ = vid;
        }

        if (chunk_full != nullptr && vid_cnt_ > MAX_MEM_CHUNK_ITEM_CNT) {
            *chunk_full = true;
        }
    } else {
        delta_->Add(ds);
    }
}

void MemIndexChunk::Delete(const DataSet& ds) {
    assert(ds.n == 1);

    if (vid_cnt_ == 0) {
        return;
    }

    std::unique_lock<std::shared_mutex> lock(mutex_);

    auto vid = ds.vids[0];
    if (vid_labelid_map_.count(vid)) {
        vid_vec_[vid_labelid_map_[vid]] = -1;
        ++del_cnt_;
    }

    if (sealed) {
        delta_->Delete(ds);
    }
}

void MemIndexChunk::Search(const DataSet& ds) {
    std::unique_ptr<faiss::SearchParameters> faiss_params =
        std::make_unique<faiss::SearchParameters>();

    std::shared_lock lock(mutex_);
    MemIndexIdSelector id_selector(vid_vec_);

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

    // convert labelid to vid
    for (size_t i = 0; i < ds.n * ds.k; i++) {
        if (ds.vids[i] >= 0) {
            ds.vids[i] = vid_vec_[ds.vids[i]];
        }
    }

    if (sealed && delta_->GetVIdCnt() > 0) {
        // search delta_
        std::vector<int64_t> vids(ds.k, -1);
        std::vector<float> distances(ds.k);
        DataSet delta_ds = ds;
        delta_ds.Vids(vids.data()).Distances(distances.data());
        delta_->Search(delta_ds);

        // merge the result
        std::vector<std::pair<int64_t, float>> res;
        for (size_t i = 0; i < delta_ds.k; i++) {
            if (vids[i] != -1) {
                res.emplace_back(vids[i], distances[i]);
            }
        }
        if (res.empty()) {
            return;
        }
        for (size_t i = 0; i < ds.k; i++) {
            if (ds.vids[i] != -1) {
                res.emplace_back(ds.vids[i],
                                 reinterpret_cast<float*>(ds.distances)[i]);
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
}

void MemIndexChunk::RangeSearch(const DataSet& ds, const SearchParams& params) {
    THROW_CODE(VectorIndexException, "TODO: implement range search later");
}

void MemIndexChunk::Seal() {
    if (sealed) {
        return;
    }
    delta_ = std::make_unique<Delta>(dim_, distance_type_);
    sealed = true;
}

}  // namespace embedding

}  // namespace graphdb
