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

#include <faiss/index_factory.h>
#include <spdlog/fmt/fmt.h>

#include <filesystem>
#include <fstream>

#include "common/exceptions.h"
#include "faiss_internal.h"
#include "index.h"

namespace graphdb {

namespace embedding {

namespace {
const static std::string delta_postfix = "delta";

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
}

Delta::Delta(int64_t dim, meta::VectorDistanceType distance_type,
             const std::string& chunk_id) {
    file_name_ = fmt::format("{}.{}", chunk_id, delta_postfix);
    delta_.reset(faiss::index_factory(
        dim, "Flat", DistanceTypeToFaissMetricType(distance_type)));
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

void Delta::Add(const DataSet& ds) {
    assert(ds.n == 1);
    delta_->add(ds.n, reinterpret_cast<const float*>(ds.x));
    int64_t vid = ds.vids[0];

    if (vid_labelid_map_.count(vid)) {
        if (vid_vec_[vid_labelid_map_[vid]] != -1) {
            THROW_CODE(InvalidParameter, "vid {} already exists", vid);
        }
    } else {
        updated_vid_vec_.push_back(vid);
    }

    vid_vec_.push_back(vid);
    vid_labelid_map_[vid] = vid_cnt_++;
}

void Delta::Flush(const std::string& dir) {
    if (flushed_cnt_ == updated_vid_vec_.size()) {
        return;
    }

    std::filesystem::path delta_path = fmt::format("{}/{}", dir, file_name_);
    if (flushed_cnt_ > 0 && !std::filesystem::exists(delta_path)) {
        THROW_CODE(IOException, "Delta file not exists");
    }

    // append to end of file
    std::ofstream deltaFile(delta_path, std::ios::binary | std::ios::app);
    if (!deltaFile) {
        THROW_CODE(IOException, "Failed to open deltaFile");
    }

    deltaFile.write(
        reinterpret_cast<const char*>(updated_vid_vec_.data() + flushed_cnt_),
        (updated_vid_vec_.size() - flushed_cnt_) * sizeof(int64_t));

    deltaFile.flush();
    deltaFile.close();
}

void Delta::Populate(const DataSet& ds) {
    if (ds.n == 0) {
        return;
    }

    delta_->add(ds.n, reinterpret_cast<const float*>(ds.x));
    int64_t* vids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto vid = vids[i];
        if (vid_labelid_map_.count(vid)) {
            THROW_CODE(InvalidParameter, "vid {} should not exists", vid);
        }
        vid_vec_.push_back(vid);
        vid_labelid_map_[vid] = vid_cnt_++;
    }
}

std::unique_ptr<Delta> Delta::Load(const std::string& dir, int64_t dim,
                                   meta::VectorDistanceType distance_type,
                                   const std::string& chunk_id) {
    std::unique_ptr<Delta> delta =
        std::make_unique<Delta>(dim, distance_type, chunk_id);

    auto delta_path = fmt::format("{}/{}", dir, delta->file_name_);
    if (!std::filesystem::exists(delta_path)) {
        return nullptr;
    }

    // Open the file in read mode
    std::ifstream is(delta_path, std::ios::binary);
    if (!is.is_open()) {
        THROW_CODE(IOException, "Failed to open the file: {}", delta_path);
    }

    // get lenght of file
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    delta->flushed_cnt_ = length / sizeof(int64_t);
    delta->updated_vid_vec_.reserve(delta->flushed_cnt_);

    int64_t rowid;
    while (is.read(reinterpret_cast<char*>(&rowid), sizeof(int64_t))) {
        delta->updated_vid_vec_.push_back(rowid);
    }

    return delta;
}

}  // namespace embedding

}  // namespace graphdb
