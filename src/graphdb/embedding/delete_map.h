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
 * For a sparse delete map, we simply append the LabelId to the end of the file.
 * For a dense delete map, we convert the file to use bits to represent the
 * LabelId.
 *
 * N.B. Though the LabelId is of type int64_t, we can safely convert it into
 * unsigned int since we never hold more than 2^32-1 labels in one single Index
 * Chunk.
 */
class DeleteMap {
   public:
    enum Format : uint32_t { SPARSE = 0, DENSE = 1 };

    DeleteMap(uint32_t total_bit_count, const std::string& chunk_id);
    ~DeleteMap() = default;

    /**
     * Set the label id as deleted
     */
    void SetDelete(uint32_t id);
    /**
     * Is the lable id deleted?
     */
    bool IsDeleted(uint32_t id) const;

    /**
     * Load this data structure from a persistent file.
     */
    static std::unique_ptr<DeleteMap> Load(const std::string& dir,
                                           const std::string& chunk_id,
                                           uint32_t total_bit_count);
    /**
     * Flush the dirty bits to file. For Sparse format, we write a brand new
     * delete map file. For Dense format, we bookeeping the dirtied page and
     * do partial write.
     */
    void Flush(const std::string& dir);

    uint32_t GetDeleteCount() const { return deleted_count_; }
    std::shared_mutex& GetSharedMutex() { return mutex_; }

    const std::string& GetFileName() const { return file_name_; }

   private:
    std::unordered_set<uint32_t> deleted_labels_;
    uint32_t total_bit_count_;
    uint32_t deleted_count_{0};
    std::string file_name_;
    Format format_{SPARSE};
    bool switch_format_{false};
    bool dirty_{false};
    std::vector<uint8_t> dirty_pages_;
    std::vector<uint8_t> bitmap_;

    std::shared_mutex mutex_;
};

}  // namespace embedding

}  // namespace graphdb
