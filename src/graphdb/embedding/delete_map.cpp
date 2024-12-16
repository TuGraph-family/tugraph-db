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

#include "delete_map.h"

#include <filesystem>
#include <fstream>

#include "common/exceptions.h"

namespace graphdb {

namespace embedding {

namespace {

const size_t SPARSE_FORMAT_MAX_DELETE = 2048;

inline size_t BitCnt2Byte(size_t bit_count) { return (bit_count + 7) >> 3; }
inline size_t Bit2Byte(size_t id) { return id >> 3; }

inline void BitmapSet(std::vector<uint8_t>& bitmap, size_t id) {
    bitmap[Bit2Byte(id)] |= (1 << (id & 7));
}

inline bool BitmapTest(const std::vector<uint8_t>& bitmap, size_t id) {
    return bitmap[Bit2Byte(id)] & (1 << (id & 7));
}

}  // namespace

DeleteMap::DeleteMap(int64_t total_bit_count)
    : total_bit_count_(total_bit_count) {}

bool DeleteMap::SetDelete(int64_t id) {
    if (id >= total_bit_count_) {
        THROW_CODE(InvalidParameter, "overflow: id {}, capacity {}", id,
                   total_bit_count_);
    }

    if (format_ == SPARSE) {
        if (deleted_labels_.count(id)) {
            // already deleted
            return false;
        }
        deleted_labels_.emplace(id);
        deleted_count_++;
        if (deleted_count_ > SPARSE_FORMAT_MAX_DELETE) {
            auto bitmap_size = BitCnt2Byte(total_bit_count_);
            bitmap_.resize(bitmap_size, 0);
            for (auto label : deleted_labels_) {
                BitmapSet(bitmap_, label);
            }
            deleted_labels_.clear();
        }
    } else {
        if (BitmapTest(bitmap_, id)) {
            // already deleted
            return false;
        }
        BitmapSet(bitmap_, id);
        deleted_count_++;
    }

    return true;
}

bool DeleteMap::IsDeleted(int64_t id) const {
    if (id >= total_bit_count_) {
        THROW_CODE(InvalidParameter, "overflow: id {}, maximum id {}", id,
                   total_bit_count_ - 1);
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

    THROW_CODE(InvalidParameter, "illegal format {}", (uint32_t)format_);
}

}  // namespace embedding

}  // namespace graphdb
