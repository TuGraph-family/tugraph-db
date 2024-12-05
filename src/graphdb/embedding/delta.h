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
 * This structure is mainly used for Persistent Index Chunks'
 * update operation.
 */
class Delta {
   public:
    Delta(int64_t dim, meta::VectorDistanceType distance_type,
          const std::string& chunk_id);
    ~Delta() = default;

    void Update(const DataSet& ds);
    void Delete(const DataSet& ds);

    /* Flush the updated rowids to persistent file */
    void Flush(const std::string& dir);

    /**
     * This is called after Load interface, read the updated
     * vids from rocksdb and add to Delta.
     */
    void Populate(const DataSet& ds);

    /**
     * Load this data structure from persistent file, Populate need
     * to be called after this.
     */
    static std::unique_ptr<Delta> Load(const std::string& dir, int64_t dim,
                                       meta::VectorDistanceType distance_type,
                                       const std::string& chunk_id);

    const std::vector<int64_t>& GetUpdatedVids() { return updated_rowid_vec_; }

   private:
    int64_t row_cnt_{0};
    std::vector<int64_t> rowid_vec_;
    std::unordered_map<int64_t, int64_t> rowid_labelid_map_;
    std::unique_ptr<faiss::Index> delta_;
    std::vector<int64_t> updated_rowid_vec_;
    int32_t flushed_cnt_;
    std::string file_name_;
};

}  // namespace embedding

}  // namespace graphdb