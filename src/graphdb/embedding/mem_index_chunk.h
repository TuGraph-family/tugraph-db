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

#include <faiss/Index.h>

#include <cstdint>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <vector>
#include <shared_mutex>

#include "proto/meta.pb.h"

namespace graphdb {

namespace embedding {

class DataSet;
class SearchParams;

class MemIndexChunk {
   public:
    MemIndexChunk(int64_t min_vid, int64_t dim,
                  meta::VectorIndexType index_type,
                  meta::VectorDistanceType distance_type);
    ~MemIndexChunk() = default;

    int64_t GetElementsNum() { return vid_cnt_; }
    int64_t GetMemoryUsage();
    int64_t GetDeleteIdsNum() { return del_cnt_; }

    void Add(const DataSet& ds, bool* chunk_full);
    // mark delete
    void Delete(const DataSet& ds);

    void Search(const DataSet& ds);
    void RangeSearch(const DataSet& ds, const SearchParams& params);

    int64_t GetMinVId() { return min_vid_; }
    int64_t GetMaxVId() { return max_vid_; }

   private:
    int64_t min_vid_;
    int64_t max_vid_;
    int64_t vid_cnt_{0};
    int64_t del_cnt_{0};
    std::vector<int64_t> vid_vec_;
    std::unordered_map<int64_t, int64_t> vid_labelid_map_;
    std::unique_ptr<faiss::Index> flat_index_{nullptr};
    int64_t dim_;
    meta::VectorIndexType index_type_;
    meta::VectorDistanceType distance_type_;
    std::shared_mutex mutex_;
};

}  // namespace embedding

}  // namespace graphdb
