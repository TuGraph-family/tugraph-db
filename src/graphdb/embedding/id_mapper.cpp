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

#include "id_mapper.h"

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

inline void ParseIdMapperBuffer(const std::string& buf, size_t length,
                                std::map<int64_t, int64_t>& vid_position_map) {
    const int64_t* start = reinterpret_cast<const int64_t*>(&buf[0]);
    length = length / 8;
    for (size_t i = 0; i < length; i += 2) {
        vid_position_map.emplace(start[i], start[i + 1]);
    }
}
}  // namespace

IdMapper::IdMapper(const std::string& chunk_id) {
    file_name_ = fmt::format("{}.{}", chunk_id, id_mapper_postfix);
}

int64_t IdMapper::Add(int64_t vid) {
    if (vid_cnt_ == 0) {
        min_vid_ = vid;
        vid_position_map_.emplace(vid, vid_cnt_);
    } else {
        if (vid <= max_vid_) {
            THROW_CODE(InvalidParameter,
                       "vid must be strictly incremental, previous vid "
                       "{}, current vid {}",
                       max_vid_, vid);
        } else if (vid > max_vid_ + 1) {
            // not consecutive, add a new entry
            vid_position_map_.emplace(vid, vid_cnt_);
        }
    }

    max_vid_ = vid;
    int64_t position = vid_cnt_;
    vid_cnt_++;
    return position;
}

void IdMapper::Seal() {
    // Construct position_vid_map_
    for (auto& [vid, position] : vid_position_map_) {
        position_vid_map_.emplace(position, vid);
    }
}

int64_t IdMapper::GetVIdFromLabel(int64_t labelId) {
    if (labelId < 0 || labelId >= vid_cnt_) {
        THROW_CODE(InvalidParameter,
                   "labelId {} not valid, should between [0, {}]", labelId,
                   vid_cnt_);
    }

    auto it = position_vid_map_.lower_bound(labelId);
    if (it != position_vid_map_.end()) {
        if (it->first == labelId) {
            return it->second;
        } else {
            // go to previous record and calculate the vid
            it--;
            return it->second + (labelId - it->first);
        }
    } else {
        auto last = position_vid_map_.rbegin();
        return last->second + (labelId - last->first);
    }
}

int64_t IdMapper::GetLabelFromVId(int64_t vid) {
    if (vid < min_vid_ || vid > max_vid_) {
        THROW_CODE(OutOfRange, "vid {} should between [{}, {}]", vid, min_vid_,
                   max_vid_);
    }

    auto it = vid_position_map_.lower_bound(vid);
    if (it != vid_position_map_.end()) {
        if (it->first == vid) {
            return it->second;
        } else {
            auto ubound = it->second;
            // go to previous record and calculate the position
            it--;
            auto labelId = it->second + (vid - it->first);
            if (labelId >= ubound) {
                THROW_CODE(InvalidParameter, "vid not exists");
            }
            return labelId;
        }
    } else {
        auto last = vid_position_map_.rbegin();
        auto labelId = last->second + (vid - last->first);
        if (labelId >= vid_cnt_) {
            THROW_CODE(InvalidParameter, "vid not exists");
        }
        return labelId;
    }
}

/* Write this data structure to a persistent file */
void IdMapper::WriteToFile(const std::string& dir) {
    std::filesystem::path filePath = fmt::format("{}/{}", dir, file_name_);
    std::string buf;
    buf.append(reinterpret_cast<const char*>(&vid_cnt_), sizeof(int64_t));
    buf.append(reinterpret_cast<const char*>(&min_vid_), sizeof(int64_t));
    buf.append(reinterpret_cast<const char*>(&max_vid_), sizeof(int64_t));

    for (auto& [vid, position] : vid_position_map_) {
        buf.append(reinterpret_cast<const char*>(&vid), sizeof(int64_t));
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
        return nullptr;
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

    // read vid_cnt_
    is.read(&buf[0], sizeof(int64_t) * 3);
    if (!is) {
        THROW_CODE(IOException, "id mapper corrupted");
    }
    mapper->vid_cnt_ = *(reinterpret_cast<int64_t*>(&buf[0]));
    mapper->min_vid_ = *(reinterpret_cast<int64_t*>(&buf[0]) + sizeof(int64_t));
    mapper->max_vid_ =
        *(reinterpret_cast<int64_t*>(&buf[0]) + sizeof(int64_t) * 2);

    size_t remain = length - sizeof(int64_t) * 3;
    while (remain > PAGE_SIZE) {
        is.read(&buf[0], PAGE_SIZE);
        if (!is) {
            THROW_CODE(IOException, "id mapper corrupted");
        }
        ParseIdMapperBuffer(buf, PAGE_SIZE, mapper->vid_position_map_);
        remain -= PAGE_SIZE;
    }
    if (remain) {
        // must be <vid, position> pair
        if (remain < sizeof(int64_t) * 2) {
            THROW_CODE(IOException, "id mapper corrupted");
        }
        is.read(&buf[0], remain);
        if (!is) {
            THROW_CODE(IOException, "id mapper corrupted");
        }
        ParseIdMapperBuffer(buf, remain, mapper->vid_position_map_);
    }

    mapper->Seal();

    return mapper;
}

}  // namespace embedding

}  // namespace graphdb
