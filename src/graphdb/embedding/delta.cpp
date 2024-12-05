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

#include "embedding/delta.h"

#include <faiss/index_factory.h>
#include <spdlog/fmt/fmt.h>

#include <filesystem>
#include <fstream>

#include "common/exceptions.h"
#include "embedding/faiss_internal.h"
#include "embedding/index.h"

namespace graphdb {

namespace embedding {

namespace {
const static std::string delta_postfix = "delta";
}

Delta::Delta(int64_t dim, meta::VectorDistanceType distance_type,
             const std::string& chunk_id) {
    file_name_ = fmt::format("{}.{}", chunk_id, delta_postfix);
    delta_.reset(faiss::index_factory(
        dim, "Flat", DistanceTypeToFaissMetricType(distance_type)));
}

void Delta::Delete(const DataSet& ds) {
    if (ds.n == 0 || row_cnt_ == 0) {
        return;
    }

    int64_t* rowids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto rowid = rowids[i];
        if (rowid_labelid_map_.count(rowid)) {
            rowid_vec_[rowid_labelid_map_[rowid]] = -1;
        }
    }
}

void Delta::Update(const DataSet& ds) {
    if (ds.n == 0) {
        return;
    }

    delta_->add(ds.n, reinterpret_cast<const float*>(ds.x));
    int64_t* rowids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto rowid = rowids[i];
        if (rowid_labelid_map_.count(rowid)) {
            rowid_vec_[rowid_labelid_map_[rowid]] = -1;
        } else {
            updated_rowid_vec_.push_back(rowid);
        }
        rowid_vec_.push_back(rowid);
        rowid_labelid_map_[rowid] = row_cnt_++;
    }
}

void Delta::Flush(const std::string& dir) {
    if (flushed_cnt_ == updated_rowid_vec_.size()) {
        return;
    }

    std::filesystem::path delta_path = fmt::format("{}/{}", dir, file_name_);
    if (flushed_cnt_ > 0 && !std::filesystem::exists(delta_path)) {
        THROW_CODE(IOException, "Delta file not exists");
    }

    std::ofstream deltaFile(delta_path, std::ios::binary | std::ios::app);
    if (!deltaFile) {
        THROW_CODE(IOException, "Failed to open deltaFile");
    }

    deltaFile.write(
        reinterpret_cast<const char*>(updated_rowid_vec_.data() + flushed_cnt_),
        (updated_rowid_vec_.size() - flushed_cnt_) * sizeof(int64_t));

    deltaFile.flush();
    deltaFile.close();
}

void Delta::Populate(const DataSet& ds) {
    if (ds.n == 0) {
        return;
    }

    delta_->add(ds.n, reinterpret_cast<const float*>(ds.x));
    int64_t* rowids = ds.vids;
    for (size_t i = 0; i < ds.n; i++) {
        auto rowid = rowids[i];
        if (rowid_labelid_map_.count(rowid)) {
            THROW_CODE(InputError, "rowid {} should not exists", rowid);
        }
        rowid_vec_.push_back(rowid);
        rowid_labelid_map_[rowid] = row_cnt_++;
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
    delta->updated_rowid_vec_.reserve(delta->flushed_cnt_);

    int64_t rowid;
    while (is.read(reinterpret_cast<char*>(&rowid), sizeof(int64_t))) {
        delta->updated_rowid_vec_.push_back(rowid);
    }

    return std::move(delta);
}

}  // namespace embedding

}  // namespace graphdb
