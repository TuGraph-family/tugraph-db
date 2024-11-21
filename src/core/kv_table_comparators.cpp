/**
 * Copyright 2022 AntGroup CO., Ltd.
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
 */

#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "core/lmdb_exception.h"
#include "core/kv_table_comparators.h"

namespace lgraph {
namespace _detail {
/**
 * The comparators for fixed-length types.
 * Compare first by key, then by vid.
 */
template <FieldType DT>
struct KeyVidCompareFunc {
    static int func(const MDB_val* a, const MDB_val* b) {
        typedef typename ::lgraph::field_data_helper::FieldType2StorageType<DT>::type T;
        FMA_DBG_ASSERT(a->mv_size == sizeof(T) + VID_SIZE && b->mv_size == sizeof(T) + VID_SIZE);
        int r = field_data_helper::ValueCompare<DT>(a->mv_data, sizeof(T), b->mv_data, sizeof(T));
        if (r != 0) return r;
        int64_t a_vid = GetVid((char*)a->mv_data + sizeof(T));
        int64_t b_vid = GetVid((char*)b->mv_data + sizeof(T));
        return a_vid < b_vid ? -1 : a_vid > b_vid ? 1 : 0;
    }
};

template <FieldType DT>
struct KeyCompareFunc {
    static int func(const MDB_val* a, const MDB_val* b) {
        return field_data_helper::ValueCompare<DT>(a->mv_data, a->mv_size, b->mv_data, b->mv_size);
    }
};

/**
 * The comparator for bytes or strings.
 */
static int LexicalKeyVidCompareFunc(const MDB_val* a, const MDB_val* b) {
    int diff = 0;
    int len_diff = 0;
    unsigned int len = 0;

    len = static_cast<int>(a->mv_size) - VID_SIZE;
    len_diff = static_cast<int>(a->mv_size) - static_cast<int>(b->mv_size);
    if (len_diff > 0) {
        len = static_cast<int>(b->mv_size) - VID_SIZE;
        len_diff = 1;
    }

    diff = memcmp(a->mv_data, b->mv_data, len);
    if (diff) {
        return diff;
    } else if (len_diff) {
        return len_diff;
    } else {
        int64_t a_vid = GetVid((char*)a->mv_data + a->mv_size - VID_SIZE);
        int64_t b_vid = GetVid((char*)b->mv_data + b->mv_size - VID_SIZE);
        return a_vid < b_vid ? -1 : a_vid > b_vid ? 1 : 0;
    }
}

/**
 * The comparators for fixed-length types.
 * Compare first by key, then by src_vid, then by dst_vid, then by eid.
 */
template <FieldType DT>
struct KeyEuidCompareFunc {
    static int func(const MDB_val* a, const MDB_val* b) {
        typedef typename ::lgraph::field_data_helper::FieldType2StorageType<DT>::type T;
        FMA_DBG_ASSERT(a->mv_size == sizeof(T) + EUID_SIZE && b->mv_size == sizeof(T) + EUID_SIZE);
        int r = field_data_helper::ValueCompare<DT>(a->mv_data, sizeof(T), b->mv_data, sizeof(T));
        if (r != 0) return r;
        int64_t a_vid1 = GetVid((char*)a->mv_data + sizeof(T));
        int64_t a_vid2 = GetVid((char*)a->mv_data + sizeof(T) + VID_SIZE);
        int64_t a_lid = GetLabelId((char*)a->mv_data + sizeof(T) + LID_BEGIN);
        int64_t a_eid = GetEid((char*)a->mv_data + sizeof(T) + EID_BEGIN);
        int64_t b_vid1 = GetVid((char*)b->mv_data + sizeof(T));
        int64_t b_vid2 = GetVid((char*)b->mv_data + sizeof(T) + VID_SIZE);
        int64_t b_lid = GetLabelId((char*)a->mv_data + sizeof(T) + LID_BEGIN);
        int64_t b_eid = GetEid((char*)b->mv_data + sizeof(T) + EID_BEGIN);
        return a_vid1 < b_vid1   ? -1
               : a_vid1 > b_vid1 ? 1
               : a_vid2 < b_vid2 ? -1
               : a_vid2 > b_vid2 ? 1
               : a_lid < b_lid   ? -1
               : a_lid > b_lid   ? 1
               : a_eid < b_eid   ? -1
               : a_eid > b_eid   ? 1
                                 : 0;
    }
};

/**
 * The comparator for bytes or strings in KeyEuid.
 */
static int LexicalKeyEuidCompareFunc(const MDB_val* a, const MDB_val* b) {
    int diff;
    int len_diff;
    unsigned int len;

    len = static_cast<int>(a->mv_size) - EUID_SIZE;
    len_diff = static_cast<int>(a->mv_size) - static_cast<int>(b->mv_size);
    if (len_diff > 0) {
        len = static_cast<int>(b->mv_size) - EUID_SIZE;
        len_diff = 1;
    }

    diff = memcmp(a->mv_data, b->mv_data, len);
    if (diff) {
        return diff;
    } else if (len_diff) {
        return len_diff;
    } else {
        int64_t a_vid1 = GetVid((char*)a->mv_data + a->mv_size - EUID_SIZE);
        int64_t a_vid2 = GetVid((char*)a->mv_data + a->mv_size - EUID_SIZE + VID_SIZE);
        int64_t a_lid = GetLabelId((char*)a->mv_data + a->mv_size - EUID_SIZE + LID_BEGIN);
        int64_t a_eid = GetEid((char*)a->mv_data + a->mv_size - EUID_SIZE + EID_BEGIN);
        int64_t b_vid1 = GetVid((char*)b->mv_data + b->mv_size - EUID_SIZE);
        int64_t b_vid2 = GetVid((char*)b->mv_data + b->mv_size - EUID_SIZE + VID_SIZE);
        int64_t b_lid = GetLabelId((char*)b->mv_data + b->mv_size - EUID_SIZE + LID_BEGIN);
        int64_t b_eid = GetEid((char*)b->mv_data + b->mv_size - EUID_SIZE + EID_BEGIN);
        return a_vid1 < b_vid1   ? -1
               : a_vid1 > b_vid1 ? 1
               : a_vid2 < b_vid2 ? -1
               : a_vid2 > b_vid2 ? 1
               : a_lid < b_lid   ? -1
               : a_lid > b_lid   ? 1
               : a_eid < b_eid   ? -1
               : a_eid > b_eid   ? 1
                                 : 0;
    }
}

template <FieldType DT>
struct KeyBothVidCompareFunc {
    static int func(const MDB_val* a, const MDB_val* b) {
        typedef typename ::lgraph::field_data_helper::FieldType2StorageType<DT>::type T;
        FMA_DBG_ASSERT(a->mv_size == (sizeof(T) + VID_SIZE * 2)
                       && b->mv_size == (sizeof(T) + VID_SIZE * 2));
        int r = field_data_helper::ValueCompare<DT>(a->mv_data, sizeof(T), b->mv_data, sizeof(T));
        if (r != 0) return r;
        int64_t a_vid1 = GetVid((char*)a->mv_data + sizeof(T));
        int64_t a_vid2 = GetVid((char*)a->mv_data + sizeof(T) + VID_SIZE);
        int64_t b_vid1 = GetVid((char*)b->mv_data + sizeof(T));
        int64_t b_vid2 = GetVid((char*)b->mv_data + sizeof(T) + VID_SIZE);
        return a_vid1 < b_vid1   ? -1
               : a_vid1 > b_vid1 ? 1
               : a_vid2 < b_vid2 ? -1
               : a_vid2 > b_vid2 ? 1
                                 : 0;
    }
};

static int LexicalKeyBothVidCompareFunc(const MDB_val* a, const MDB_val* b) {
    int diff;
    int len_diff;
    unsigned int len;

    len = static_cast<int>(a->mv_size) - VID_SIZE * 2;
    len_diff = static_cast<int>(a->mv_size) - static_cast<int>(b->mv_size);
    if (len_diff > 0) {
        len = static_cast<int>(b->mv_size) - VID_SIZE * 2;
        len_diff = 1;
    }

    diff = memcmp(a->mv_data, b->mv_data, len);
    if (diff == 0 && len_diff == 0) {
        int64_t a_vid1 = GetVid((char*)a->mv_data + a->mv_size - VID_SIZE * 2);
        int64_t a_vid2 = GetVid((char*)a->mv_data + a->mv_size - VID_SIZE);
        int64_t b_vid1 = GetVid((char*)b->mv_data + b->mv_size - VID_SIZE * 2);
        int64_t b_vid2 = GetVid((char*)b->mv_data + b->mv_size - VID_SIZE);
        return a_vid1 < b_vid1   ? -1
               : a_vid1 > b_vid1 ? 1
               : a_vid2 < b_vid2 ? -1
               : a_vid2 > b_vid2 ? 1
                                 : 0;
    }
    return static_cast<int>(diff ? diff : len_diff < 0 ? -1 : len_diff);
}


struct CompositeKeyCompare {
    inline static std::vector<FieldType> data_types = {};
    static int CompositeKeyWithVidCompareFunc(const MDB_val* a, const MDB_val* b) {
        MDB_val pa{a->mv_size - VID_SIZE, a->mv_data};
        MDB_val pb{b->mv_size - VID_SIZE, b->mv_data};
        int res = CompositeKeyCompareFunc(&pa, &pb);
        if (res != 0)
            return res;
        int64_t a_vid = GetVid((char*)a->mv_data + a->mv_size - VID_SIZE);
        int64_t b_vid = GetVid((char*)b->mv_data + b->mv_size - VID_SIZE);
        return a_vid < b_vid ? -1 : a_vid > b_vid ? 1 : 0;
    }

    static int CompositeKeyCompareFunc(const MDB_val* a, const MDB_val* b) {
        int len = data_types.size();
        std::vector<int16_t> offset_a(len + 1), offset_b(len + 1);
        for (int i = 1; i < len; ++i) {
            memcpy(&offset_a[i], (char*)a->mv_data + (i - 1) * 2, 2);
            memcpy(&offset_b[i], (char*)b->mv_data + (i - 1) * 2, 2);
        }
        offset_a[len] = a->mv_size - (len - 1) * 2;
        offset_b[len] = b->mv_size - (len - 1) * 2;
        for (int i = 0; i < len; ++i) {
            MDB_val pa, pb;
            int res;
            switch (data_types[i]) {
            case FieldType::BOOL:
                pa = {sizeof(bool), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(bool), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::BOOL>::func(&pa, &pb);
                break;
            case FieldType::INT8:
                pa = {sizeof(int8_t), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(int8_t), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::INT8>::func(&pa, &pb);
                break;
            case FieldType::INT16:
                pa = {sizeof(int16_t), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(int16_t), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::INT16>::func(&pa, &pb);
                break;
            case FieldType::INT32:
                pa = {sizeof(int32_t), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(int32_t), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::INT32>::func(&pa, &pb);
                break;
            case FieldType::DATE:
                pa = {sizeof(int32_t), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(int32_t), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::DATE>::func(&pa, &pb);
                break;
            case FieldType::INT64:
                pa = {sizeof(int64_t), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(int64_t), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::INT64>::func(&pa, &pb);
                break;
            case FieldType::DATETIME:
                pa = {sizeof(int64_t), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(int64_t), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::DATETIME>::func(&pa, &pb);
                break;
            case FieldType::FLOAT:
                pa = {sizeof(float), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(float), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::FLOAT>::func(&pa, &pb);
                break;
            case FieldType::DOUBLE:
                pa = {sizeof(double), (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {sizeof(double), (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::DOUBLE>::func(&pa, &pb);
                break;
            case FieldType::STRING:
                pa = {static_cast<size_t>(offset_a[i + 1] - offset_a[i]),
                      (char*)a->mv_data + (len - 1) * 2 + offset_a[i]};
                pb = {static_cast<size_t>(offset_b[i + 1] - offset_b[i]),
                      (char*)b->mv_data + (len - 1) * 2 + offset_b[i]};
                res = _detail::KeyCompareFunc<FieldType::STRING>::func(&pa, &pb);
                break;
            case FieldType::BLOB:
                THROW_CODE(KvException, "Blob fields cannot act as key.");
            default:
                THROW_CODE(KvException, "Unknown data type: {}", data_types[i]);
            }
            if (res != 0)
                return res;
        }
        return 0;
    }
};

}  // namespace _detail

// NOLINT
KeySortFunc GetKeyComparator(const ComparatorDesc& desc) {
    // useful macro
#define RETURN_TYPED_COMPARATOR(type)                                        \
    if (desc.comp_type == ComparatorDesc::SINGLE_TYPE) {                     \
        return _detail::KeyCompareFunc<FieldType::type>::func;               \
    } else if (desc.comp_type == ComparatorDesc::TYPE_AND_VID) {             \
        return _detail::KeyVidCompareFunc<FieldType::type>::func;            \
    } else if (desc.comp_type == ComparatorDesc::TYPE_AND_EUID) {            \
        return _detail::KeyEuidCompareFunc<FieldType::type>::func;           \
    } else if (desc.comp_type == ComparatorDesc::BOTH_SIDE_VID) {            \
        return _detail::KeyBothVidCompareFunc<FieldType::type>::func;        \
    }                                                                        \

    // here is the logic
    switch (desc.comp_type) {
    case ComparatorDesc::BYTE_SEQ:
        return nullptr;
    case ComparatorDesc::SINGLE_TYPE:
    case ComparatorDesc::TYPE_AND_VID:
    case ComparatorDesc::TYPE_AND_EUID:
    case ComparatorDesc::BOTH_SIDE_VID:
        switch (desc.data_type) {
        case FieldType::BOOL:
            RETURN_TYPED_COMPARATOR(BOOL);
        case FieldType::INT8:
            RETURN_TYPED_COMPARATOR(INT8);
        case FieldType::INT16:
            RETURN_TYPED_COMPARATOR(INT16);
        case FieldType::INT32:
            RETURN_TYPED_COMPARATOR(INT32);
        case FieldType::INT64:
            RETURN_TYPED_COMPARATOR(INT64);
        case FieldType::DATE:
            RETURN_TYPED_COMPARATOR(DATE);
        case FieldType::DATETIME:
            RETURN_TYPED_COMPARATOR(DATETIME);
        case FieldType::FLOAT:
            RETURN_TYPED_COMPARATOR(FLOAT);
        case FieldType::DOUBLE:
            RETURN_TYPED_COMPARATOR(DOUBLE);
        case FieldType::STRING:
            if (desc.comp_type == ComparatorDesc::SINGLE_TYPE)
                return nullptr;
            else if (desc.comp_type == ComparatorDesc::TYPE_AND_VID)
                return _detail::LexicalKeyVidCompareFunc;
            else if (desc.comp_type == ComparatorDesc::TYPE_AND_EUID)
                return _detail::LexicalKeyEuidCompareFunc;
            else if (desc.comp_type == ComparatorDesc::BOTH_SIDE_VID)
                return _detail::LexicalKeyBothVidCompareFunc;
        case FieldType::BLOB:
            THROW_CODE(KvException, "Blob fields cannot act as key.");
        case FieldType::FLOAT_VECTOR:
            THROW_CODE(KvException, "Float vector fields cannot act as key.");
        default:
            THROW_CODE(KvException, "Unknown data type: {}", desc.data_type);
        }
    case ComparatorDesc::GRAPH_KEY:
        return nullptr;
    case ComparatorDesc::COMPOSITE_KEY:
        {
            _detail::CompositeKeyCompare::data_types = desc.data_types;
            return _detail::CompositeKeyCompare::CompositeKeyCompareFunc;
        }
    case ComparatorDesc::COMPOSITE_KEY_AND_VID:
        {
            _detail::CompositeKeyCompare::data_types = desc.data_types;
            return _detail::CompositeKeyCompare::CompositeKeyWithVidCompareFunc;
        }
    default:
        THROW_CODE(KvException, "Unrecognized comparator type: {}", desc.comp_type);
    }
}
}  // namespace lgraph
