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
#include "core/field_extractor_v2.h"

namespace lgraph {
namespace _detail {

bool FieldExtractorV2::GetIsNull(const Value& record) const {
    if (!def_.optional) {
        return false;
    }
    // get the Kth bit from NullArray
    const char* arr = GetNullArray(record);
    return arr[def_.id / 8] & (0x1 << (def_.id % 8));
}

std::string FieldExtractorV2::FieldToString(const Value& record) const {
    if (GetIsNull(record)) return "\"null\"";
    std::string ret;

#define _COPY_FIELD_AND_RETURN_STR_(record, ft)                                       \
    do {                                                                              \
        typename field_data_helper::FieldType2StorageType<FieldType::ft>::type d = 0; \
        typedef typename field_data_helper::FieldType2CType<FieldType::ft>::type CT;  \
        GetCopy(record, d);                                                           \
        return fma_common::StringFormatter::Format("{}", static_cast<CT>(d));         \
    } while (0)

    switch (def_.type) {
    case FieldType::BOOL:
        _COPY_FIELD_AND_RETURN_STR_(record, BOOL);
    case FieldType::INT8:
        _COPY_FIELD_AND_RETURN_STR_(record, INT8);
    case FieldType::INT16:
        _COPY_FIELD_AND_RETURN_STR_(record, INT16);
    case FieldType::INT32:
        _COPY_FIELD_AND_RETURN_STR_(record, INT32);
    case FieldType::INT64:
        _COPY_FIELD_AND_RETURN_STR_(record, INT64);
    case FieldType::FLOAT:
        _COPY_FIELD_AND_RETURN_STR_(record, FLOAT);
    case FieldType::DOUBLE:
        _COPY_FIELD_AND_RETURN_STR_(record, DOUBLE);
    case FieldType::DATE:
        {
            int32_t i;
            GetCopy(record, i);
            return Date(i).ToString();
        }
    case FieldType::DATETIME:
        {
            int64_t i;
            GetCopy(record, i);
            return DateTime(i).ToString();
        }
    case FieldType::STRING:
        {
            std::string ret(GetDataSize(record), 0);
            GetCopyRaw(record, &ret[0], ret.size());
            return ret;
        }
    case FieldType::BLOB:
        {
            // std::string ret(GetDataSize(record), 0);
            // GetCopyRaw(record, &ret[0], ret.size());
            // return ::lgraph_api::base64::Encode(ret.substr(2));
            return fma_common::StringFormatter::Format("[BLOB]");
        }
    case FieldType::POINT:
    case FieldType::LINESTRING:
    case FieldType::POLYGON:
    case FieldType::SPATIAL:
        {
            std::string ret(GetDataSize(record), 0);
            GetCopyRaw(record, &ret[0], ret.size());
            return ret;
        }
    case FieldType::FLOAT_VECTOR:
        {
            std::string vec_str;
            for (size_t i = 0; i < record.AsType<std::vector<float>>().size(); i++) {
                auto floatnum = record.AsType<std::vector<float>>().at(i);
                if (record.AsType<std::vector<float>>().at(i) > 999999) {
                    vec_str += std::to_string(floatnum).substr(0, 7);
                } else {
                    vec_str += std::to_string(floatnum).substr(0, 8);
                }
                vec_str += ',';
            }
            if (!vec_str.empty()) {
                vec_str.pop_back();
            }
            return vec_str;
        }
    case lgraph_api::NUL:
        break;
    }
    LOG_ERROR() << "Data type " << field_data_helper::FieldTypeName(def_.type) << " not handled";
    return "";
}

void FieldExtractorV2::SetLabelInRecord(bool label_in_record) {
    label_in_record_ = label_in_record;
    // refresh count_offset and nullarry_offset
    count_offset_ = sizeof(VersionId) + (label_in_record ? sizeof(LabelId) : 0);
    nullarray_offset_ = count_offset_ + sizeof(FieldId);
}

void FieldExtractorV2::GetCopy(const Value& record, std::string& data) const {
    FMA_DBG_ASSERT(Type() != FieldType::BLOB);
    data.resize(GetDataSize(record));
    GetCopyRaw(record, &data[0], data.size());
}

void FieldExtractorV2::GetCopy(const Value& record, Value& data) const {
    data.Resize(GetDataSize(record));
    GetCopyRaw(record, data.Data(), data.Size());
}

void FieldExtractorV2::GetCopyRaw(const Value& record, void* data, size_t size) const {
    size_t off = GetFieldOffset(record, def_.id);
    if (is_vfield_) {
        off = ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + off);
        FMA_DBG_ASSERT(off + size + sizeof(DataOffset) <= record.Size());
        memcpy(data, record.Data() + off + sizeof(DataOffset), size);
    } else {
        FMA_DBG_ASSERT(off + size <= record.Size());
        memcpy(data, record.Data() + off, size);
    }
}

FieldId FieldExtractorV2::GetRecordCount(const Value& record) const {
    return ::lgraph::_detail::UnalignedGet<FieldId>(record.Data() + count_offset_);
}

size_t FieldExtractorV2::GetDataSize(const Value& record) const {
    if (is_vfield_) {
        DataOffset var_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(
            record.Data() + GetFieldOffset(record, def_.id));
        // The length is stored at the beginning of the variable-length field data area.
        return ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + var_offset);
    } else {
        return GetFieldOffset(record, def_.id + 1) - GetFieldOffset(record, def_.id);
    }
}
size_t FieldExtractorV2::GetFieldOffset(const Value& record, const FieldId id) const {
    const uint16_t count = GetRecordCount(record);
    if (0 == id) {
        // The starting position of Field0 is at the end of the offset section.
        return nullarray_offset_ + (count + 7) / 8 + count * sizeof(DataOffset);
    }

    size_t offset = 0;
    offset = nullarray_offset_ + (count + 7) / 8 + (id - 1) * sizeof(DataOffset);
    return ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + offset);
}

size_t FieldExtractorV2::GetOffsetPosition(const Value& record, const FieldId id) const {
    FMA_DBG_ASSERT(id > 0);
    const uint16_t count = GetRecordCount(record);
    return nullarray_offset_ + (count + 7) / 8 + (id - 1) * sizeof(DataOffset);
}

void* FieldExtractorV2::GetFieldPointer(const Value& record) const {
    if (is_vfield_) {
        DataOffset var_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(
            record.Data() + GetFieldOffset(record, def_.id));
        return (char*)record.Data() + sizeof(uint32_t) + var_offset;
    }
    return (char*)record.Data() + GetFieldOffset(record, def_.id);
}

void FieldExtractorV2::SetIsNull(const Value& record, bool is_null) const {
    if (!def_.optional) {
        if (is_null) throw FieldCannotBeSetNullException(Name());
    }
    // set the Kth bit from NullArray
    char* arr = GetNullArray(record);
    if (is_null) {
        arr[def_.id / 8] |= (0x1 << (def_.id % 8));
    } else {
        arr[def_.id / 8] &= ~(0x1 << (def_.id % 8));
    }
}

void FieldExtractorV2::SetVariableOffset(Value& record, FieldId id, DataOffset offset) const {
    size_t off = GetFieldOffset(record, id);
    ::lgraph::_detail::UnalignedSet<DataOffset>(record.Data() + off, offset);
}

void FieldExtractorV2::_SetFixedSizeValueRaw(Value& record, const Value& data) const {
    // "Cannot call SetField(Value&, const T&) on a variable length field";
    FMA_DBG_ASSERT(!is_vfield_);
    // "Type size mismatch"
    FMA_DBG_CHECK_EQ(data.Size(), field_data_helper::FieldTypeSize(def_.type));
    FMA_DBG_CHECK_EQ(data.Size(), GetDataSize(record));
    // copy the buffer so we don't accidentally overwrite memory
    char* ptr = (char*)record.Data() + GetFieldOffset(record, def_.id);
    memcpy(ptr, data.Data(), data.Size());
}

void FieldExtractorV2::_SetVariableValueRaw(Value& record, const Value& data) const {
    FMA_DBG_ASSERT(is_vfield_);
    DataOffset foff = GetFieldOffset(record, def_.id);
    DataOffset ff = ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + foff);
    ::lgraph::_detail::UnalignedSet<DataOffset>(record.Data() + ff, data.Size());
    memcpy(record.Data() + ff + sizeof(DataOffset), data.Data(), data.Size());
}

}  // namespace _detail
}  // namespace lgraph
