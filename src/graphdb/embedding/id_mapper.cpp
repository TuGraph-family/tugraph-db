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

#include "embedding/id_mapper.h"

#include <spdlog/fmt/fmt.h>

#include <filesystem>
#include <fstream>

#include "common/exceptions.h"
#include "common/logger.h"

namespace graphdb {

namespace embedding {

namespace {
const static std::string id_mapper_postfix = "mapper";

// 4K
const size_t PAGE_SIZE = 4096;

inline void ParseIdMapperBuffer(
    const std::string& buf, int length,
    std::map<int64_t, int64_t>& rowid_position_map) {
    const int64_t* start = reinterpret_cast<const int64_t*>(&buf[0]);
    length = length / 8;
    for (size_t i = 0; i < length; i += 2) {
        rowid_position_map.emplace(start[i], start[i + 1]);
    }
}
}  // namespace

IdMapper::IdMapper(const std::string& chunk_id) {
    file_name_ = fmt::format("{}.{}", chunk_id, id_mapper_postfix);
}

int64_t IdMapper::Add(int64_t rowId) {
    if (row_cnt_ == 0) {
        min_row_id_ = rowId;
        rowid_position_map_.emplace(rowId, row_cnt_);
    } else {
        if (rowId <= max_row_id_) {
            THROW_CODE(InvalidParameter,
                       "row id must be strictly incremental, previous rowid "
                       "{}, current rowid {}",
                       max_row_id_, rowId);
        } else if (rowId > max_row_id_ + 1) {
            rowid_position_map_.emplace(rowId, row_cnt_);
        }
    }

    max_row_id_ = rowId;
    int64_t position = row_cnt_;
    row_cnt_++;
    return position;
}

void IdMapper::Seal() {
    // Construct position_rowid_map_
    for (auto& [rowid, position] : rowid_position_map_) {
        position_rowid_map_.emplace(position, rowid);
    }
}

int64_t IdMapper::GetRowIdFromLabel(int64_t labelId) {
    if (labelId < 0 || labelId >= row_cnt_) {
        THROW_CODE(InvalidParameter,
                   "labelId {} not valid, should between [0, {}]", labelId,
                   row_cnt_);
    }

    auto it = position_rowid_map_.lower_bound(labelId);
    if (it != position_rowid_map_.end()) {
        if (it->first == labelId) {
            return it->second;
        } else {
            // go to previous record and calculate the rowid
            it--;
            return it->second + (labelId - it->first);
        }
    } else {
        auto last = position_rowid_map_.rbegin();
        return last->second + (labelId - last->first);
    }
}

int64_t IdMapper::GetLabelFromRowId(int64_t rowId) {
    if (rowId < min_row_id_ || rowId > max_row_id_) {
        THROW_CODE(OutOfRange, "row id {} should between [{}, {}]", rowId,
                   min_row_id_, max_row_id_);
    }

    auto it = rowid_position_map_.lower_bound(rowId);
    if (it != rowid_position_map_.end()) {
        if (it->first == rowId) {
            return it->second;
        } else {
            auto ubound = it->second;
            // go to previous record and calculate the position
            it--;
            auto labelId = it->second + (rowId - it->first);
            if (labelId >= ubound) {
                THROW_CODE(InvalidParameter, "row id not exists");
            }
            return labelId;
        }
    } else {
        auto last = rowid_position_map_.rbegin();
        auto labelId = last->second + (rowId - last->first);
        if (labelId >= row_cnt_) {
            THROW_CODE(InvalidParameter, "row id not exists");
        }
        return labelId;
    }
}

/* Write this data structure to a persistent file */
void IdMapper::WriteToFile(const std::string& dir) {
    std::filesystem::path filePath = fmt::format("{}/{}", dir, file_name_);
    std::string buf;
    buf.append(reinterpret_cast<const char*>(&row_cnt_), sizeof(int64_t));
    for (auto& [rowid, position] : rowid_position_map_) {
        buf.append(reinterpret_cast<const char*>(&rowid), sizeof(int64_t));
        buf.append(reinterpret_cast<const char*>(&position), sizeof(int64_t));
    }

    std::filesystem::path tmpFilePath =
        std::filesystem::temp_directory_path() / file_name_;
    std::ofstream tmpFile(tmpFilePath, std::ios::binary);
    if (!tmpFile) {
        THROW_CODE(IOException, "Failed to open tmpFile");
    }

    tmpFile.write(buf.data(), buf.size());
    if (!tmpFile) {
        THROW_CODE(IOException, "Failed to write to temporary file");
    }

    tmpFile.flush();
    tmpFile.close();

    std::filesystem::rename(tmpFilePath, filePath);
}

/* Load this data structure from a persistent file */
std::unique_ptr<IdMapper> IdMapper::Load(const std::string& dir,
                                         const std::string& chunk_id) {
    std::unique_ptr<IdMapper> mapper = std::make_unique<IdMapper>(chunk_id);

    auto file_path = fmt::format("{}/{}", dir, mapper->GetFileName());
    if (!std::filesystem::exists(file_path)) {
        return;
    }

    // Open the file in read mode
    std::ifstream is(file_path, std::ios::binary);
    if (!is.is_open()) {
        THROW_CODE(IOException, "Failed to open the file: {}", file_path);
    }

    // get length of file
    is.seekg(0, is.end);
    int length = is.tellg();
    is.seekg(0, is.beg);

    std::string buf(PAGE_SIZE, '\0');

    // read row_cnt_
    is.read(&buf[0], sizeof(int64_t));
    if (!is) {
        THROW_CODE(IOException, "id mapper corrupted");
    }
    mapper->row_cnt_ = *(reinterpret_cast<int64_t*>(&buf[0]));

    int remain = length - sizeof(int64_t);
    while (remain > PAGE_SIZE) {
        is.read(&buf[0], PAGE_SIZE);
        if (!is) {
            THROW_CODE(IOException, "id mapper corrupted");
        }
        ParseIdMapperBuffer(buf, PAGE_SIZE, mapper->rowid_position_map_);
        remain -= PAGE_SIZE;
    }
    if (remain) {
        // must be <rowid, position> pair
        if (remain < sizeof(int64_t) * 2) {
            THROW_CODE(IOException, "id mapper corrupted");
        }
        is.read(&buf[0], remain);
        if (!is) {
            THROW_CODE(IOException, "id mapper corrupted");
        }
        ParseIdMapperBuffer(buf, remain, mapper->rowid_position_map_);
    }

    mapper->Seal();

    return std::move(mapper);
}

}  // namespace embedding

}  // namespace graphdb
