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

#pragma once

#include "core/field_extractor_base.h"

#include "core/field_data_helper.h"
#include "core/vertex_index.h"
#include "core/vector_index.h"
#include "core/vsag_hnsw.h"

namespace lgraph {
class Schema;

namespace _detail {
/** A field extractor can be used to get a field in the record. */
class FieldExtractorV2 : public FieldExtractorBase {
    friend class lgraph::Schema;
    bool label_in_record_ = true;
    size_t count_offset_ = sizeof(VersionId) + sizeof(LabelId);
    size_t nullarray_offset_ = sizeof(VersionId) + sizeof(LabelId) + sizeof(FieldId);

 public:
    FieldExtractorV2() : FieldExtractorBase() {}

    FieldExtractorV2(const FieldExtractorV2& rhs) : FieldExtractorBase(rhs.GetFieldSpec()) {
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
    }

    FieldExtractorV2(FieldExtractorV2&& rhs) noexcept : FieldExtractorBase(std::move(rhs)) {
        count_offset_ = rhs.count_offset_;
        nullarray_offset_ = rhs.nullarray_offset_;
    }

    FieldExtractorV2& operator=(const FieldExtractorV2& rhs) {
        if (this == &rhs) return *this;
        FieldExtractorBase::operator=(rhs);
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
        return *this;
    }

    FieldExtractorV2& operator=(FieldExtractorV2&& rhs) noexcept {
        if (this == &rhs) return *this;
        FieldExtractorBase::operator=(std::move(rhs));
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
        return *this;
    }

    explicit FieldExtractorV2(const FieldSpec& d) noexcept : FieldExtractorBase(d) {}

    FieldExtractorV2(const FieldSpec& d, const FieldId id) noexcept : FieldExtractorBase(d) {
        SetFieldId(id);
    }

    ~FieldExtractorV2() override = default;

    // Get FieldExtractor.
    bool GetIsNull(const Value& record) const override;
    // Set FieldExtractor.
    void SetLabelInRecord(bool label_in_record);
    // Set record.
    void SetRecordCount(Value& record, FieldId count) const {
        memcpy(record.Data() + count_offset_, &count, sizeof(FieldId));
    }

    // set null in the record.
    void SetIsNull(const Value& record, bool is_null) const override;

    ENABLE_IF_FIXED_FIELD(T, void)
    SetFixedSizeValue(Value& record, const T& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(!IsFixedType());
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(sizeof(data), TypeSize());
        // copy the buffer so we don't accidentally overwrite memory
        record.Resize(record.Size());
        char* ptr = (char*)record.Data() + GetFieldOffset(record);
        ::lgraph::_detail::UnalignedSet<T>(ptr, data);
    }

    // set variable's offset, they are stored at fixed-data area.
    void SetVariableOffset(Value& record, FieldId id, DataOffset offset) const;
    // set fixed length data, only if length of the data in record equal its definition.
    void _SetFixedSizeValueRaw(Value& record, const Value& data) const;

    // for test only.
    void _SetVariableValueRaw(Value& record, const Value& data) const;
    // Get record.
    void GetCopyRaw(const Value& record, void* data, size_t size) const override;
    /** Retrieve the starting position of the Field data for the given ID.
     *  Note that both fixed-length and variable-length data are not distinguished here.
     */
    size_t GetFieldOffset(const Value& record, const FieldId id) const;

    size_t GetFieldOffset(const Value& record) const override {
        return GetFieldOffset(record, GetFieldId());
    }

    // return the position of the field's offset.
    size_t GetOffsetPosition(const Value& record, FieldId id) const;

    // return field num in the record.
    FieldId GetRecordCount(const Value& record) const;

    // return null array pointer.
    char* GetNullArray(const Value& record) const override {
        return record.Data() + nullarray_offset_;
    }

    size_t GetDataSize(const Value& record) const override;

    void* GetFieldPointer(const Value& record) const override;
};

}  // namespace _detail
}  // namespace lgraph
