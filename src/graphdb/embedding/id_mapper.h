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
 * Index Chunks' row ids are scaned from rocksdb, thus sorted
 * with punching holes.
 * This structure should be persisted.
 */
class IdMapper {
   public:
    IdMapper(const std::string& chunk_id);
    ~IdMapper() = default;

    void Add(int64_t rowId);
    void Seal();

    int64_t GetRowIdFromLabel(int64_t labelId);
    int64_t GetLabelFromRowId(int64_t rowId);

    int64_t GetMinRowId() const { return min_row_id_; }
    int64_t GetMaxRowId() const { return max_row_id_; }
    int64_t GetRowCnt() const { return row_cnt_; }
    const std::string& GetFileName() const { return file_name_; }

    /* Write this data structure to a persistent file */
    void WriteToFile(const std::string& dir);
    /* Load this data structure from a persistent file */
    static std::unique_ptr<IdMapper> Load(const std::string& dir,
                                          const std::string& chunk_id);

   private:
    int64_t min_row_id_;
    int64_t max_row_id_;
    int64_t row_cnt_{0};
    std::map<int64_t, int64_t> rowid_position_map_;
    std::map<int64_t, int64_t> position_rowid_map_;
    std::string file_name_;
};

}  // namespace embedding

}  // namespace graphdb
