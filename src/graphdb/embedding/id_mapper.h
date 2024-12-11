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
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace graphdb {

namespace embedding {

/*
 * Index Chunks' vids are scaned from rocksdb, thus sorted
 * with punching holes.
 * This structure should be persisted to file.
 */
class IdMapper {
   public:
    IdMapper(const std::string& chunk_id);
    ~IdMapper() = default;

    // add vid to mapper and return position id
    int64_t Add(int64_t vid);
    void Seal();

    int64_t GetVIdFromLabel(int64_t labelId);
    int64_t GetLabelFromVId(int64_t vid);

    int64_t GetMinVId() const { return min_vid_; }
    int64_t GetMaxVId() const { return max_vid_; }
    int64_t GetVIdCnt() const { return vid_cnt_; }
    const std::string& GetFileName() const { return file_name_; }

    /* Write this data structure to a persistent file */
    void WriteToFile(const std::string& dir);
    /* Load this data structure from a persistent file */
    static std::unique_ptr<IdMapper> Load(const std::string& dir,
                                          const std::string& chunk_id);

   private:
    int64_t min_vid_;
    int64_t max_vid_;
    int64_t vid_cnt_{0};
    std::map<int64_t, int64_t> vid_position_map_;
    std::map<int64_t, int64_t> position_vid_map_;
    std::string file_name_;
};

}  // namespace embedding

}  // namespace graphdb
