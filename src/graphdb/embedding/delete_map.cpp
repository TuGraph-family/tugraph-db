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

#include "embedding/delete_map.h"

#include <spdlog/fmt/fmt.h>

#include <filesystem>
#include <fstream>

#include "common/exceptions.h"

namespace graphdb {

namespace embedding {

namespace {

const static std::string delete_map_postfix = ".del";

// We expect the delete map file smaller than 8K for SPARSE format
const size_t SPARSE_FORMAT_MAX_DELETE = 2048 - 2;

// 4K
const size_t PAGE_SIZE = 4096;
const size_t PAGE_BIT_CNT = PAGE_SIZE << 3;

inline size_t BitCnt2Byte(size_t bit_count) { return (bit_count + 7) >> 3; }
inline size_t BitCnt2Page(size_t bit_count) {
    return (bit_count + PAGE_BIT_CNT - 1) / PAGE_BIT_CNT;
}

inline size_t Bit2Byte(size_t id) { return id >> 3; }
inline size_t Bit2Page(size_t id) { return id / PAGE_BIT_CNT; }

inline size_t BitByteOffset(size_t id) { return id % 8; }
inline size_t BitPageOffset(size_t id) { return id % PAGE_BIT_CNT; }

inline void BitmapSet(std::vector<uint8_t>& bitmap, size_t id) {
    bitmap[Bit2Byte(id)] |= (1 << (id & 7));
}

inline bool BitmapTest(const std::vector<uint8_t>& bitmap, size_t id) {
    return bitmap[Bit2Byte(id)] & (1 << (id & 7));
}

inline void ParseSparseBuffer(const std::string& buf, int length,
                              std::unordered_set<uint32_t>& deleted_labels) {
    const uint32_t* start = reinterpret_cast<const uint32_t*>(&buf[0]);
    length = length / 4;
    for (size_t i = 0; i < length; i++) {
        deleted_labels.emplace(start[i]);
    }
}

inline void ParseDenseBuffer(const std::string& buf, int length,
                             uint8_t* offset) {
    const uint8_t* start = reinterpret_cast<const uint8_t*>(&buf[0]);
    memcpy(offset, start, length);
}

}  // namespace

DeleteMap::DeleteMap(uint32_t total_bit_count, const std::string& chunk_id)
    : total_bit_count_(total_bit_count) {
    file_name_ = fmt::format("{}.{}", chunk_id, delete_map_postfix);
}

void DeleteMap::SetDelete(uint32_t id) {
    if (id >= total_bit_count_) {
        THROW_CODE(InvalidParameter, "overflow: id {}, capacity {}", id,
                   total_bit_count_);
    }

    if (format_ == SPARSE) {
        if (deleted_labels_.count(id)) {
            return;
        }
        deleted_labels_.emplace(id);
        dirty_ = true;
        deleted_count_++;
        if (deleted_count_ > SPARSE_FORMAT_MAX_DELETE) {
            switch_format_ = true;
            auto bitmap_size = BitCnt2Byte(total_bit_count_);
            auto dirty_page_size = BitCnt2Byte(BitCnt2Page(total_bit_count_));
            bitmap_.resize(bitmap_size, 0);
            dirty_pages_.resize(dirty_page_size, 0);
            for (auto label : deleted_labels_) {
                BitmapSet(bitmap_, label);
            }
            deleted_labels_.clear();
        }
    } else {
        if (BitmapTest(bitmap_, id)) {
            return;
        }
        BitmapSet(bitmap_, id);
        dirty_ = true;
        if (!switch_format_) {
            BitmapSet(dirty_pages_, Bit2Page(id));
        }
    }
}

bool DeleteMap::IsDeleted(uint32_t id) const {
    if (id >= total_bit_count_) {
        THROW_CODE(InvalidParameter, "overflow: id {}, capacity {}", id,
                   total_bit_count_);
    }

    if (deleted_count_ == 0) {
        return false;
    }

    if (format_ == SPARSE) {
        return deleted_labels_.count(id);
    }

    if (format_ == DENSE) {
        return BitmapTest(bitmap_, id);
    }

    THROW_CODE(InvalidParameter, "illegal format {}", format_);
}

std::unique_ptr<DeleteMap> DeleteMap::Load(const std::string& dir,
                                           const std::string& chunk_id,
                                           uint32_t total_bit_count) {
    std::unique_ptr<DeleteMap> del_map =
        std::make_unique<DeleteMap>(total_bit_count, chunk_id);
    auto file_path = fmt::format("{}/{}", dir, del_map->GetFileName());
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

    // read format & delete cnt
    is.read(&buf[0], sizeof(uint32_t) * 2);
    if (!is) {
        THROW_CODE(IOException, "del map corrupted");
    }
    int remain = length - sizeof(uint32_t) * 2;

    del_map->deleted_count_ = *(reinterpret_cast<uint32_t*>(&buf[0]) + 1);
    switch (*reinterpret_cast<uint32_t*>(&buf[0])) {
        case SPARSE: {
            del_map->format_ = SPARSE;
            while (remain > PAGE_SIZE) {
                is.read(&buf[0], PAGE_SIZE);
                if (!is) {
                    THROW_CODE(IOException, "del map corrupted");
                }
                ParseSparseBuffer(buf, PAGE_SIZE, del_map->deleted_labels_);
                remain -= PAGE_SIZE;
            }
            if (remain) {
                is.read(&buf[0], remain);
                if (!is) {
                    THROW_CODE(IOException, "del map corrupted");
                }
                ParseSparseBuffer(buf, remain, del_map->deleted_labels_);
            }
            break;
        }
        case DENSE: {
            del_map->format_ = DENSE;
            auto bitmap_size = BitCnt2Byte(total_bit_count);
            auto dirty_page_size = BitCnt2Byte(BitCnt2Page(total_bit_count));
            int offset = 0;

            del_map->bitmap_.resize(bitmap_size);
            del_map->dirty_pages_.resize(dirty_page_size, 0);
            while (remain > PAGE_SIZE) {
                is.read(&buf[0], PAGE_SIZE);
                if (!is) {
                    THROW_CODE(IOException, "del map corrupted");
                }
                ParseDenseBuffer(buf, PAGE_SIZE,
                                 del_map->bitmap_.data() + offset);
                remain -= PAGE_SIZE;
                offset += PAGE_SIZE;
            }
            if (remain) {
                is.read(&buf[0], remain);
                if (!is) {
                    THROW_CODE(IOException, "del map corrupted");
                }
                ParseDenseBuffer(buf, PAGE_SIZE,
                                 del_map->bitmap_.data() + offset);
            }
            break;
        }
        default:
            THROW_CODE(IOException, "del map corrupted");
    }

    is.close();
    return std::move(del_map);
}

void DeleteMap::Flush(const std::string& dir) {
    if (!dirty_) {
        return;
    }

    std::filesystem::path target_path = fmt::format("{}/{}", dir, file_name_);
    if (format_ == SPARSE) {
        // for SPARSE format, we do a full write of the delete map file, this
        // file should be smaller thank 8K
        std::string buf;
        buf.append(reinterpret_cast<const char*>(&format_), sizeof(uint32_t));
        buf.append(reinterpret_cast<const char*>(&deleted_count_),
                   sizeof(uint32_t));
        for (auto label : deleted_labels_) {
            buf.append(reinterpret_cast<const char*>(&label), sizeof(uint32_t));
        }

        std::filesystem::path tempFilePath =
            std::filesystem::temp_directory_path() / file_name_;
        std::ofstream tmpFile(tempFilePath, std::ios::binary);
        if (!tmpFile) {
            THROW_CODE(IOException, "Failed to open tmpFile");
        }

        tmpFile.write(buf.data(), buf.size());
        if (!tmpFile) {
            THROW_CODE(IOException, "Failed to write to temporary file");
        }

        tmpFile.flush();
        tmpFile.close();

        std::filesystem::rename(tempFilePath, target_path);
    } else if (switch_format_) {
        // full write
        std::string buf;
        buf.append(reinterpret_cast<const char*>(&format_), sizeof(uint32_t));
        buf.append(reinterpret_cast<const char*>(&deleted_count_),
                   sizeof(uint32_t));

        std::filesystem::path tempFilePath =
            std::filesystem::temp_directory_path() / file_name_;
        std::ofstream tmpFile(tempFilePath, std::ios::binary);
        if (!tmpFile) {
            THROW_CODE(IOException, "Failed to open tmpFile");
        }

        tmpFile.write(buf.data(), buf.size());
        if (!tmpFile) {
            THROW_CODE(IOException, "Failed to write header to temporary file");
        }

        tmpFile.write(reinterpret_cast<char*>(bitmap_.data()),
                      bitmap_.size() * sizeof(uint8_t));
        if (!tmpFile) {
            THROW_CODE(IOException, "Failed to write bitmap to temporary file");
        }

        tmpFile.flush();
        tmpFile.close();

        std::filesystem::rename(tempFilePath, target_path);
        switch_format_ = false;
    } else {
        // do partial update
        if (!std::filesystem::exists(target_path)) {
            THROW_CODE(IOException,
                       "Delete map file not exists for partial update");
        }

        std::ofstream targetFile(target_path, std::ios::binary);
        if (!targetFile) {
            THROW_CODE(IOException, "Failed to open tmpFile");
        }

        // update delete count
        targetFile.seekp(sizeof(uint32_t));
        targetFile.write(reinterpret_cast<const char*>(&deleted_count_),
                         sizeof(uint32_t));

        if (!targetFile) {
            THROW_CODE(IOException,
                       "Failed to write delete count to temporary file");
        }

        auto bitmap_size = BitCnt2Byte(total_bit_count_);
        for (size_t i = 0; i < Bit2Page(total_bit_count_); i++) {
            if (BitmapTest(dirty_pages_, i)) {
                size_t length = (i + 1) * PAGE_SIZE > bitmap_size
                                    ? bitmap_size % PAGE_SIZE
                                    : PAGE_SIZE;
                // don't forget the two uint32_t header
                targetFile.seekp(sizeof(uint32_t) * 2 + i * PAGE_SIZE);
                targetFile.write(reinterpret_cast<const char*>(bitmap_.data() +
                                                               i * PAGE_SIZE),
                                 length);
            }
        }
        if (!targetFile) {
            THROW_CODE(IOException, "Failed to write bitmap to temporary file");
        }

        memset(dirty_pages_.data(), 0, dirty_pages_.size());
    }

    dirty_ = false;
}

}  // namespace embedding

}  // namespace graphdb
