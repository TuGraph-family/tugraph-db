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
// FieldExtractorV2 allows rapid schema changes and can be used to retrieve data within a record.
// However, using this class to set data in a record is not appropriate, as the design of
// FieldExtractorV2 causes individual attributes to impact others. Therefore, modifications and data
// setting should be performed at the schema level.

class FieldExtractorV2 : public FieldExtractorBase {
    friend class Schema;

    // The label may or may not be stored in the record, which affects subsequent offset
    // calculations.
    bool label_in_record_ = true;

    size_t count_offset_ = sizeof(LabelId);

    size_t nullarray_offset_ = sizeof(LabelId) + sizeof(FieldId);

 public:
    FieldExtractorV2() : FieldExtractorBase() {}

    FieldExtractorV2(const FieldExtractorV2& rhs) : FieldExtractorBase(rhs) {
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

    std::shared_ptr<FieldExtractorBase> Clone() const override {
        return std::make_shared<FieldExtractorV2>(*this);
    }

    bool DataInRecord(const Value &record) const override;

    Value GetDefaultValue() const override;

    // Get Field info. Check if it's null in record.
    bool GetIsNull(const Value& record) const override;

    // Set Field info.
    void SetLabelInRecord(bool label_in_record);

    // Set fields count in record.
    void SetRecordCount(Value& record, FieldId count) const {
        memcpy(record.Data() + count_offset_, &count, sizeof(FieldId));
    }

    // set is null in the record.
    void SetIsNull(const Value& record, bool is_null) const override;


    // test only. Set fixed data in record.
    ENABLE_IF_FIXED_FIELD(T, void)
    SetFixedSizeValue(Value& record, const T& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(IsFixedType());
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(sizeof(data), TypeSize());
        // copy the buffer so we don't accidentally overwrite memory
        record.Resize(record.Size());
        char* ptr = (char*)record.Data() + GetFieldOffset(record);
        ::lgraph::_detail::UnalignedSet<T>(ptr, data);
    }

    // set variable's offset, they are stored at fixed-data area.
    // test only.
    void SetVariableOffset(Value& record, FieldId id, DataOffset offset) const;

    // set fixed length data, only if length of the data in record equal its definition.
    // test only.
    void _SetFixedSizeValueRaw(Value& record, const Value& data) const;

    // for test only.
    void _SetVariableValueRaw(Value& record, const Value& data) const;

    // Get copy of data in the record.
    void GetCopyRaw(const Value& record, void* data, size_t size) const override;

    // Retrieve the starting position of the Field data for the given ID.
    // Note that both fixed-length and variable-length data are not distinguished here.
    size_t GetFieldOffset(const Value& record, FieldId id) const;

    // Get FieldOffset of this filed.
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

    FieldId GetFieldId() const override {
        return GetFieldSpec().id;
    }

    void SetFieldId(FieldId id) override {
        SetFieldSpecId(id);
    }
};

}  // namespace _detail
}  // namespace lgraph
