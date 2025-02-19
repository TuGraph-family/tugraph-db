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

#include "core/blob_manager.h"
#include "core/field_data_helper.h"
#include "core/vertex_index.h"
#include "core/vector_index.h"
#include "core/vsag_hnsw.h"

namespace lgraph {
class Schema;

namespace _detail {

/** A field extractor can be used to get/set a field in the record. */

// FieldExtractorV1 is the initial implementation of FieldExtractor, allowing data to be set
// and retrieved within a record. However, this approach results in exceptionally high costs
// for schema alterations.

class FieldExtractorV1 : public FieldExtractorBase {
    friend class lgraph::Schema;
    // layout
    bool is_vfield_ = false;
    size_t field_id_ = 0;
    union {
        size_t data_off = 0;
        struct {
            size_t idx;  // index of this field in all the vfields
            size_t v_offs;
            size_t last_idx;
        };
    } offset_;
    size_t nullable_array_off_ = 0;  // offset of nullable array in record
    size_t null_bit_off_ = 0;

 public:
    FieldExtractorV1() : FieldExtractorBase() {}

    FieldExtractorV1(const FieldExtractorV1& rhs) : FieldExtractorBase(rhs) {
        is_vfield_ = !rhs.IsFixedType();
        offset_ = rhs.offset_;
        nullable_array_off_ = rhs.nullable_array_off_;
        null_bit_off_ = rhs.null_bit_off_;
        field_id_ = rhs.field_id_;
    }

    FieldExtractorV1(FieldExtractorV1&& rhs) noexcept : FieldExtractorBase(std::move(rhs)) {
        is_vfield_ = !rhs.IsFixedType();
        offset_ = rhs.offset_;
        null_bit_off_ = rhs.null_bit_off_;
        nullable_array_off_ = rhs.nullable_array_off_;
        field_id_ = rhs.field_id_;
    }

    FieldExtractorV1& operator=(const FieldExtractorV1& rhs) {
        if (this == &rhs) return *this;
        FieldExtractorBase::operator=(rhs);
        is_vfield_ = rhs.IsFixedType();
        offset_ = rhs.offset_;
        null_bit_off_ = rhs.null_bit_off_;
        nullable_array_off_ = rhs.nullable_array_off_;
        field_id_ = rhs.field_id_;
        return *this;
    }

    FieldExtractorV1& operator=(FieldExtractorV1&& rhs) noexcept {
        if (this == &rhs) return *this;
        FieldExtractorBase::operator=(std::move(rhs));
        is_vfield_ = rhs.IsFixedType();
        offset_ = rhs.offset_;
        null_bit_off_ = rhs.null_bit_off_;
        nullable_array_off_ = rhs.nullable_array_off_;
        field_id_ = rhs.field_id_;
        return *this;
    }

    ~FieldExtractorV1() override = default;

    std::shared_ptr<FieldExtractorBase> Clone() const override {
        return std::make_shared<FieldExtractorV1>(*this);
    }

    // for test only
    explicit FieldExtractorV1(const FieldSpec& d) noexcept : FieldExtractorBase(d) {
        null_bit_off_ = 0;
        is_vfield_ = !field_data_helper::IsFixedLengthFieldType(d.type);
        if (is_vfield_) SetVLayoutInfo(d.optional ? 1 : 0, 1, 0);
    }

    bool DataInRecord(const Value& record) const override {
        return true;
    }

    Value GetDefaultValue() const override {
        return Value();
    }

    bool GetIsNull(const Value& record) const override;

    // parse a string as input and then set field in record
    // cannot be used for blobs since they need formatting
    void ParseAndSet(Value& record, const std::string& data) const;

    // get FieldData as input and then set field in record
    // used for blobs *only* in case of AlterLabel, when we need to
    // copy old data into new format
    void ParseAndSet(Value& record, const FieldData& data) const;

    // parse and set a blob
    // data can be string or FieldData
    // store_blob is a function of type std::function<BlobKey(const Value&)>
    template <typename DataT, typename StoreBlobAndGetKeyFunc>
    void ParseAndSetBlob(Value& record, const DataT& data,
                         const StoreBlobAndGetKeyFunc& store_blob) const {
        FMA_DBG_ASSERT(Type() == FieldType::BLOB);
        bool is_null;
        Value v = FieldExtractorBase::ParseBlob(data, is_null);
        SetIsNull(record, is_null);
        if (is_null) return;
        if (v.Size() <= _detail::MAX_IN_PLACE_BLOB_SIZE) {
            _SetVariableLengthValue(record, BlobManager::ComposeSmallBlobData(v));
        } else {
            BlobManager::BlobKey key = store_blob(v);
            v.Clear();
            _SetVariableLengthValue(record, BlobManager::ComposeLargeBlobData(key));
        }
    }

    void CopyDataRaw(Value& dst_record, const Value& src_record,
                     const FieldExtractorV1* extr) const {
        if (extr->GetIsNull(src_record)) {
            SetIsNull(dst_record, true);
            return;
        }
        SetIsNull(dst_record, false);
        if (is_vfield_) {
            _SetVariableLengthValue(dst_record, extr->GetConstRef(src_record));
        } else {
            _SetFixedSizeValueRaw(dst_record, extr->GetConstRef(src_record));
        }
    }


    FieldId GetFieldId() const override {
        return field_id_;
    }

    void SetFieldId(FieldId id) override { field_id_ = id; }

 private:
    void SetFixedLayoutInfo(size_t offset) {
        is_vfield_ = false;
        offset_.data_off = offset;
    }

    void SetVLayoutInfo(size_t voff, size_t nv, size_t idx) {
        is_vfield_ = true;
        offset_.v_offs = voff;
        offset_.last_idx = nv - 1;
        offset_.idx = idx;
    }

    void SetNullableOff(size_t offset) { null_bit_off_ = offset; }

    void SetNullableArrayOff(size_t offset) { nullable_array_off_ = offset; }

    //-----------------------
    // record accessors

    template <FieldType FT>
    void _ParseStringAndSet(Value& record, const std::string& data) const;

    /**
     * Sets the value of the field in the record, assuming it is not a null value.
     * data should not be empty for fixed field
     *
     * \param [in,out]  record  The record.
     * \param           data    The data.
     *
     * \return  ErrorCode::OK if succeeds, or
     *          FIELD_CANNOT_BE_NULL
     *          DATA_SIZE_TOO_LARGE
     */
    void _SetVariableLengthValue(Value& record, const Value& data) const;

    /**
     * Sets the value of the field in record. Valid only for fixed-length fields.
     *
     * \param   record  The record.
     * \param   data    Value to be set.
     *
     * \return   ErrorCode::OK if succeeds.
     */
    ENABLE_IF_FIXED_FIELD(T, void)
    SetFixedSizeValue(Value& record, const T& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(!is_vfield_);
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(sizeof(data), TypeSize());
        // copy the buffer so we don't accidentally overwrite memory
        record.Resize(record.Size());
        char* ptr = (char*)record.Data() + offset_.data_off;
        ::lgraph::_detail::UnalignedSet<T>(ptr, data);
    }

    void _SetFixedSizeValueRaw(Value& record, const Value& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(!is_vfield_);
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(data.Size(), TypeSize());
        // copy the buffer so we don't accidentally overwrite memory
        char* ptr = (char*)record.Data() + offset_.data_off;
        memcpy(ptr, data.Data(), data.Size());
    }

    // set field value to null
    void SetIsNull(const Value& record, bool is_null) const override;

    /**
     * Extracts field data from the record to the buffer pointed to by data. This
     * is for internal use only, the size MUST match the data size.
     *
     * \param           record  The record.
     * \param [in,out]  data    If non-null, the data.
     * \param           size    Size of field, must be equal to field size.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopyRaw(const Value& record, void* data, size_t size) const override {
        size_t off = GetFieldOffset(record);
        FMA_DBG_ASSERT(off + size <= record.Size());
        memcpy(data, record.Data() + off, size);
    }

    char* GetNullArray(const Value& record) const override {
        return record.Data() + nullable_array_off_;
    }

    size_t GetDataSize(const Value& record) const override {
        if (is_vfield_) {
            return GetNextOffset(record) - GetFieldOffset(record);
        } else {
            return TypeSize();
        }
    }

    size_t GetFieldOffset(const Value& record) const override {
        if (is_vfield_) {
            size_t off =
                (offset_.idx == 0)
                    ? (offset_.v_offs + sizeof(DataOffset) * (offset_.last_idx))
                    : ::lgraph::_detail::UnalignedGet<DataOffset>(
                          record.Data() + offset_.v_offs + (offset_.idx - 1) * sizeof(DataOffset));
            return off;
        } else {
            return offset_.data_off;
        }
    }

    size_t GetNextOffset(const Value& record) const {
        if (is_vfield_) {
            size_t off =
                (offset_.idx == offset_.last_idx)
                    ? record.Size()
                    : ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + offset_.v_offs +
                                                                  offset_.idx * sizeof(DataOffset));
            return off;
        } else {
            return offset_.data_off + TypeSize();
        }
    }

    void* GetFieldPointer(const Value& record) const override {
        return (char*)record.Data() + GetFieldOffset(record);
    }
};

}  // namespace _detail

}  // namespace lgraph
