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

bool FieldExtractorV2::DataInRecord(const Value& record) const {
    if (GetFieldId() + 1 > GetRecordCount(record)) {
        return false;
    }
    return true;
}

Value FieldExtractorV2::GetInitedValue() const {
    Value v = field_data_helper::FieldDataToValueOfFieldType(GetInitedFieldData(), Type());
    return v;
}


bool FieldExtractorV2::GetIsNull(const Value& record) const {
    if (!IsOptional()) {
        return false;
    }
    // get the Kth bit from NullArray
    const char* arr = GetNullArray(record);
    return arr[GetFieldId() / 8] & (0x1 << (GetFieldId() % 8));
}

void FieldExtractorV2::SetLabelInRecord(bool label_in_record) {
    label_in_record_ = label_in_record;
    // refresh count_offset and nullarry_offset
    count_offset_ = (label_in_record ? sizeof(LabelId) : 0);
    nullarray_offset_ = count_offset_ + sizeof(FieldId);
}

void FieldExtractorV2::GetCopyRaw(const Value& record, void* data, size_t size) const {
    size_t off = GetFieldOffset(record);
    if (!IsFixedType()) {
        // Get variable data's offset.
        off = ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + off);
        // for variable value : | data-size | data-raw|
        FMA_DBG_ASSERT(off + size + sizeof(DataOffset) <= record.Size());

        memcpy(data, record.Data() + off + sizeof(DataOffset), size);
    } else {
        // for fixed type, size must get from GetDataSize()
        FMA_DBG_ASSERT(off + size <= record.Size());
        memcpy(data, record.Data() + off, size);
    }
}

FieldId FieldExtractorV2::GetRecordCount(const Value& record) const {
    return ::lgraph::_detail::UnalignedGet<FieldId>(record.Data() + count_offset_);
}

size_t FieldExtractorV2::GetDataSize(const Value& record) const {
    if (!IsFixedType()) {
        DataOffset var_offset =
            ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + GetFieldOffset(record));
        // The length is stored at the beginning of the variable-length field data area.
        return ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + var_offset);
    } else {
        int id_offset = 1;
        while (GetFieldOffset(record, GetFieldId() + id_offset) == 0) {
            id_offset++;
        }
        return GetFieldOffset(record, GetFieldId() + id_offset) -
               GetFieldOffset(record, GetFieldId());
    }
}

size_t FieldExtractorV2::GetFieldOffset(const Value& record, const FieldId id) const {
    const uint16_t count = GetRecordCount(record);
    if (0 == id) {
        // The starting position of Field0 is at the end of the offset section
        // which can be directly calculated.
        return nullarray_offset_ + (count + 7) / 8 + count * sizeof(DataOffset);
    }

    size_t offset = 0;
    offset = nullarray_offset_ + (count + 7) / 8 + (id - 1) * sizeof(DataOffset);
    return ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + offset);
}

size_t FieldExtractorV2::GetOffsetPosition(const Value& record, const FieldId id) const {
    // Field0 do not have offset.
    FMA_DBG_ASSERT(id > 0);
    const uint16_t count = GetRecordCount(record);
    return nullarray_offset_ + (count + 7) / 8 + (id - 1) * sizeof(DataOffset);
}

void* FieldExtractorV2::GetFieldPointer(const Value& record) const {
    if (!IsFixedType()) {
        DataOffset var_offset =
            ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + GetFieldOffset(record));
        // For variable data, return data-raw's pointer.
        return (char*)record.Data() + sizeof(uint32_t) + var_offset;
    }
    return (char*)record.Data() + GetFieldOffset(record);
}

void FieldExtractorV2::SetIsNull(const Value& record, bool is_null) const {
    if (!IsOptional()) {
        if (is_null) throw FieldCannotBeSetNullException(Name());
    }
    // set the Kth bit from NullArray
    char* arr = GetNullArray(record);
    if (is_null) {
        arr[GetFieldId() / 8] |= (0x1 << (GetFieldId() % 8));
    } else {
        arr[GetFieldId() / 8] &= ~(0x1 << (GetFieldId() % 8));
    }
}

void FieldExtractorV2::SetVariableOffset(Value& record, FieldId id, DataOffset offset) const {
    size_t off = GetFieldOffset(record, id);
    ::lgraph::_detail::UnalignedSet<DataOffset>(record.Data() + off, offset);
}

void FieldExtractorV2::_SetFixedSizeValueRaw(Value& record, const Value& data) const {
    // "Cannot call SetField(Value&, const T&) on a variable length field";
    FMA_DBG_ASSERT(IsFixedType());
    // "Type size mismatch"
    FMA_DBG_CHECK_EQ(data.Size(), TypeSize());
    FMA_DBG_CHECK_EQ(data.Size(), GetDataSize(record));
    // copy the buffer so we don't accidentally overwrite memory
    char* ptr = (char*)record.Data() + GetFieldOffset(record);
    memcpy(ptr, data.Data(), data.Size());
}

void FieldExtractorV2::_SetVariableValueRaw(Value& record, const Value& data) const {
    FMA_DBG_ASSERT(!IsFixedType());
    DataOffset foff = GetFieldOffset(record);
    DataOffset ff = ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + foff);
    ::lgraph::_detail::UnalignedSet<DataOffset>(record.Data() + ff, data.Size());
    memcpy(record.Data() + ff + sizeof(DataOffset), data.Data(), data.Size());
}

}  // namespace _detail
}  // namespace lgraph
