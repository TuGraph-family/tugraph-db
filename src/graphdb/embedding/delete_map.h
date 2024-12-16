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

#include <cstdint>
#include <memory>
#include <shared_mutex>
#include <string>
#include <unordered_set>
#include <vector>

namespace graphdb {

namespace embedding {

/**
 * This structure is primarily used to mark the Embedding LabelId of Persistent
 * Index Chunk as deleted.
 * For a sparse delete map, we simply inset the LabelId to the set.
 * For a dense delete map, we convert the file to use bits to represent the
 * LabelId.
 */
class DeleteMap {
   public:
    enum Format : uint32_t { SPARSE = 0, DENSE = 1 };

    DeleteMap(int64_t total_bit_count);
    ~DeleteMap() = default;

    /**
     * Set the label id as deleted
     * return:
     *  false if already set before
     *  true if set successfully
     */
    bool SetDelete(int64_t id);
    /**
     * Is the lable id deleted?
     */
    bool IsDeleted(int64_t id) const;

    int64_t GetDeleteCount() const { return deleted_count_; }

   private:
    std::unordered_set<int64_t> deleted_labels_;
    int64_t total_bit_count_;
    size_t deleted_count_{0};
    Format format_{SPARSE};
    std::vector<uint8_t> bitmap_;
    std::shared_mutex mutex_;
};

}  // namespace embedding

}  // namespace graphdb
