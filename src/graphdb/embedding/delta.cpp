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

#include "delta.h"

#include <faiss/impl/FaissException.h>
#include <faiss/index_factory.h>

#include <filesystem>
#include <fstream>

#include "common/exceptions.h"
#include "faiss_internal.h"
#include "index.h"

namespace graphdb {

namespace embedding {

namespace {

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

class DeltaIdSelector : public IdSelector {
   public:
    DeltaIdSelector(const std::vector<int64_t>& vid_vec) : vid_vec_(vid_vec) {}
    ~DeltaIdSelector() override = default;

    bool is_member(int64_t id) const override { return vid_vec_[id] != -1; }

   private:
    const std::vector<int64_t>& vid_vec_;
};

}  // namespace

Delta::Delta(int64_t dim, meta::VectorDistanceType distance_type) {
    delta_.reset(faiss::index_factory(
        dim, "Flat", DistanceTypeToFaissMetricType(distance_type)));
}

void Delta::Add(const DataSet& ds) {
    assert(ds.n == 1);
    delta_->add(ds.n, reinterpret_cast<const float*>(ds.x));
    int64_t vid = ds.vids[0];

    if (vid_labelid_map_.count(vid)) {
        if (vid_vec_[vid_labelid_map_[vid]] != -1) {
            THROW_CODE(InvalidParameter, "vid {} already exists", vid);
        }
    }
    vid_vec_.push_back(vid);
    vid_labelid_map_[vid] = vid_cnt_++;
}

void Delta::AddBatch(const DataSet& ds) {
    delta_->add(ds.n, reinterpret_cast<const float*>(ds.x));
    int64_t* vids = ds.vids;

    for (size_t i = 0; i < ds.n; i++) {
        int64_t vid = vids[i];
        vid_vec_.push_back(vid);
        vid_labelid_map_[vid] = vid_cnt_++;
    }
}

void Delta::Delete(const DataSet& ds) {
    assert(ds.n == 1);
    int64_t vid = ds.vids[0];
    if (vid_labelid_map_.count(vid) == 0 ||
        vid_vec_[vid_labelid_map_[vid]] == -1) {
        THROW_CODE(InvalidParameter, "vid {} doesn't exist", vid);
    }

    vid_vec_[vid_labelid_map_[vid]] = -1;
}

void Delta::Search(const DataSet& ds) {
    assert(ds.n == 1);
    std::unique_ptr<faiss::SearchParameters> faiss_params =
        std::make_unique<faiss::SearchParameters>();

    DeltaIdSelector id_selector(vid_vec_);
    std::unique_ptr<FaissIDSelector> faiss_selector =
        std::make_unique<FaissIDSelector>(id_selector);

    faiss_params->sel = faiss_selector.get();

    try {
        delta_->search(ds.n, reinterpret_cast<const float*>(ds.x), ds.k,
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
}

void Delta::RangeSearch(const DataSet& ds) {
    THROW_CODE(VectorIndexException, "TODO: implement range search later");
}

}  // namespace embedding

}  // namespace graphdb
