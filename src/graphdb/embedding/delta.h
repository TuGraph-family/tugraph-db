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
#include <string>
#include <unordered_map>
#include <vector>

#include "proto/meta.pb.h"

namespace graphdb {

namespace embedding {

class DataSet;
class MemIndexChunk;

/**
 * This structure is mainly used for record updated data after Chunks'
 * Seal operation.
 */
class Delta {
   public:
    Delta(int64_t dim, meta::VectorDistanceType distance_type);
    ~Delta() = default;

    void Add(const DataSet& ds);
    void AddBatch(const DataSet& ds);
    void Delete(const DataSet& ds);

    void Search(const DataSet& ds);
    void RangeSearch(const DataSet& ds);

    int64_t GetVIdCnt() { return vid_cnt_; }

    std::vector<int64_t>& GetVIdVec() { return vid_vec_; }

   private:
    int64_t vid_cnt_{0};
    std::vector<int64_t> vid_vec_;
    std::unordered_map<int64_t, int64_t> vid_labelid_map_;
    std::unique_ptr<faiss::Index> delta_;
};

}  // namespace embedding

}  // namespace graphdb
