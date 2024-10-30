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

#include "core/blob_manager.h"
#include "core/field_data_helper.h"
#include "core/vertex_index.h"
#include "core/edge_index.h"
#include "core/schema_common.h"
#include "core/vector_index.h"
#include "core/vsag_hnsw.h"

namespace lgraph {
class Schema;

namespace _detail {

#define ENABLE_IF_FIXED_FIELD(_TYPE_, _RT_) \
    template <typename _TYPE_>              \
    typename std::enable_if<                \
        std::is_integral<_TYPE_>::value || std::is_floating_point<_TYPE_>::value, _RT_>::type

/** A field extractor can be used to get a field in the record. */
class FieldExtractorV2 {
    friend class lgraph::Schema;
    // type information
    FieldSpec def_;
    // is variable property field
    bool is_vfield_ = false;
    bool label_in_record_ = true;
    // index
    std::unique_ptr<VertexIndex> vertex_index_;
    std::unique_ptr<EdgeIndex> edge_index_;
    // fulltext index
    bool fulltext_indexed_ = false;
    // vector index
    std::shared_ptr<VectorIndex> vector_index_;
    size_t count_offset_ = sizeof(VersionId) + sizeof(LabelId);
    size_t nullarray_offset_ = sizeof(VersionId) + sizeof(LabelId) + sizeof(FieldId);

 public:
    FieldExtractorV2() : vertex_index_(nullptr), edge_index_(nullptr) {}

    ~FieldExtractorV2() {}

    explicit FieldExtractorV2(const FieldExtractorV2& rhs) {
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = rhs.vector_index_;
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
    }

    explicit FieldExtractorV2(FieldExtractorV2&& rhs) noexcept {
        def_ = std::move(rhs.def_);
        is_vfield_ = rhs.is_vfield_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        rhs.vertex_index_ = nullptr;
        rhs.edge_index_ = nullptr;
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = std::move(rhs.vector_index_);
        rhs.vector_index_ = nullptr;
        count_offset_ = rhs.count_offset_;
        nullarray_offset_ = rhs.nullarray_offset_;
    }

    explicit FieldExtractorV2(const FieldSpec& d) noexcept : def_(d) {
        is_vfield_ = !field_data_helper::IsFixedLengthFieldType(d.type);
        vertex_index_ = nullptr;
        edge_index_ = nullptr;
    }

    FieldExtractorV2(const FieldSpec& d, FieldId id) noexcept : def_(d) {
        is_vfield_ = !field_data_helper::IsFixedLengthFieldType(d.type);
        vertex_index_ = nullptr;
        edge_index_ = nullptr;
        SetFieldId(id);
    }

    FieldExtractorV2& operator=(const FieldExtractorV2& rhs) {
        if (this == &rhs) return *this;
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = rhs.vector_index_;
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
        return *this;
    }

    FieldExtractorV2& operator=(FieldExtractorV2&& rhs) noexcept {
        if (this == &rhs) return *this;
        def_ = std::move(rhs.def_);
        is_vfield_ = rhs.is_vfield_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = std::move(rhs.vector_index_);
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
        return *this;
    }

    // get

    const FieldSpec& GetFieldSpec() const { return def_; }

    bool GetIsNull(const Value& record) const;

    FieldData GetDefaultValue() const { return def_.default_value; }

    FieldData GetInitedValue() const { return def_.init_value; }

    bool HasDefaultValue() const { return def_.set_default_value; }

    bool HasInitedValue() const { return def_.inited_value; }

    const std::string& Name() const { return def_.name; }

    FieldType Type() const { return def_.type; }

    size_t TypeSize() const { return field_data_helper::FieldTypeSize(def_.type); }

    size_t DataSize(const Value& record) const { return GetDataSize(record); }

    bool IsOptional() const { return def_.optional; }

    bool IsFixedType() const { return field_data_helper::IsFixedLengthFieldType(def_.type); }

    bool IsDeleted() const { return def_.deleted; }

    VertexIndex* GetVertexIndex() const { return vertex_index_.get(); }

    EdgeIndex* GetEdgeIndex() const { return edge_index_.get(); }

    bool FullTextIndexed() const { return fulltext_indexed_; }

    VectorIndex* GetVectorIndex() const { return vector_index_.get(); }

    FieldId GetFieldId() const { return def_.id; }

    template <typename GetBlobByKeyFunc>
    Value GetBlobConstRef(const Value& record, const GetBlobByKeyFunc& get_blob_by_key) const {
        FMA_DBG_ASSERT(Type() == FieldType::BLOB);
        if (GetIsNull(record)) return Value();
        Value v((char*)GetFieldPointer(record), GetDataSize(record));
        if (BlobManager::IsLargeBlob(v)) {
            return get_blob_by_key(BlobManager::GetLargeBlobKey(v));
        } else {
            return BlobManager::GetSmallBlobContent(v);
        }
    }

    inline Value ParseBlob(const std::string& str, bool& is_null) const {
        // string input is always seen as non-NULL
        is_null = false;
        // decode str as base64
        std::string decoded;
        if (!::lgraph_api::base64::TryDecode(str.data(), str.size(), decoded))
            throw ParseStringException(Name(), str, Type());
        return Value(decoded);
    }

    // get a const ref of raw blob data
    inline Value ParseBlob(const FieldData& fd, bool& is_null) const {
        if (fd.type == FieldType::NUL) {
            is_null = true;
            return Value();
        }
        is_null = false;
        if (fd.type == FieldType::BLOB) {
            return Value::ConstRef(*fd.data.buf);
        }
        if (fd.type == FieldType::STRING) {
            std::string decoded;
            const std::string& s = *fd.data.buf;
            if (!::lgraph_api::base64::TryDecode(s.data(), s.size(), decoded))
                throw ParseStringException(Name(), s, Type());
            return Value(decoded);
        } else {
            throw ParseIncompatibleTypeException(Name(), fd.type, FieldType::BLOB);
            return Value();
        }
    }

    /**
     * Print the string representation of the field. For digital types, it prints
     * it into ASCII string; for NBytes and String, it just copies the content of
     * the field into the string.
     *
     * \param   record  The record.
     *
     * \return  String representation of the field.
     */
    std::string FieldToString(const Value& record) const;

    // set
    void SetDefaultValue(const FieldData& data) {
        def_.default_value = FieldData(data);
        def_.set_default_value = true;
    }

    void SetInitValue(const FieldData& data) {
        def_.init_value = FieldData(data);
        def_.inited_value = true;
    }

    void MarkDeleted() {
        def_.deleted = true;
        // free data space when marked deleted
        def_.init_value.~FieldData();
        def_.default_value.~FieldData();
        def_.inited_value = false;
        def_.set_default_value = false;
    }

    void SetLabelInRecord(bool label_in_record);

    void SetRecordCount(Value& record, FieldId count) const {
        memcpy(record.Data() + count_offset_, &count, sizeof(FieldId));
    }

    // set null in the record.
    void SetIsNull(const Value& record, bool is_null) const;

    /**
     * Extract a field from record into data of type T. T must be fixed-length
     * type.
     *
     * \param   record  The record in which fields are stored.
     * \param   data    Place where the extracted data will be stored.
     *
     * Assert fails if data is corrupted.
     */
    ENABLE_IF_FIXED_FIELD(T, void) GetCopy(const Value& record, T& data) const {
        FMA_DBG_ASSERT(field_data_helper::FieldTypeSize(def_.type) == sizeof(T));
        size_t offset = GetFieldOffset(record, def_.id);
        size_t size = GetDataSize(record);
        if (size == sizeof(T)) {
            memcpy(&data, (char*)record.Data() + offset, sizeof(T));
        } else {
            ConvertData(&data, (char*)record.Data() + offset, size);
        }
    }

    /**
     * Extracts a copy of field into the string.
     *
     * \param           record  The record.
     * \param [in,out]  data    The result data.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopy(const Value& record, std::string& data) const;

    /**
     * Extracts field data from the record
     *
     * \param           record  The record.
     * \param [in,out]  data    The result.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopy(const Value& record, Value& data) const;

    void GetCopyRaw(const Value& record, void* data, size_t size) const;

    // Gets a const reference of the field.
    // Formatted data is returned for blob, which means [is_large_blob] [blob_data | blob_key]
    Value GetConstRef(const Value& record) const {
        if (GetIsNull(record)) return Value();
        return Value((char*)GetFieldPointer(record), GetDataSize(record));
    }

    /**
     *  Convert data for integral and floating types.
     *  If we change the data type of floating-point or integer values
     *  (i.e., by altering their defined length), we need to adjust their values accordingly.
     *  For example, when converting from INT64 to INT8 (a relatively rare operation),
     *  we need to return an appropriate value within the range of the new type.
     *  This approach allows us to retain the original value when modifying the data type,
     *  without requiring a complete scan of the data to generate a new field.
     */
    ENABLE_IF_FIXED_FIELD(T, void) ConvertData(T* dst, const char* data, size_t size) const {
        if (std::is_integral_v<T>) {
            int64_t temp = 0;
            switch (size) {
            case 1:
                temp = *reinterpret_cast<const int8_t*>(data);
                break;
            case 2:
                temp = *reinterpret_cast<const int16_t*>(data);
                break;
            case 4:
                temp = *reinterpret_cast<const int32_t*>(data);
                break;
            case 8:
                temp = *reinterpret_cast<const int64_t*>(data);
                break;
            default:
                FMA_ASSERT(false) << "Invalid size";
            }

            if (temp > std::numeric_limits<T>::max()) {
                *dst = std::numeric_limits<T>::max();
            } else if (temp < std::numeric_limits<T>::min()) {
                *dst = std::numeric_limits<T>::min();
            } else {
                *dst = static_cast<T>(temp);
            }
        } else if (std::is_floating_point_v<T>) {
            switch (size) {
            case 4:
                *dst = static_cast<T>(*reinterpret_cast<const float*>(data));
                break;
            case 8:
                *dst = static_cast<T>(*reinterpret_cast<const double*>(data));
                break;
            default:
                FMA_ASSERT(false) << "Invalid size";
            }
        }
    }

    /** Retrieve the starting position of the Field data for the given ID.
     *  Note that both fixed-length and variable-length data are not distinguished here.
     */
    size_t GetFieldOffset(const Value& record, const FieldId id) const;

    // return the position of the field's offset.
    size_t GetOffsetPosition(const Value& record, const FieldId id) const;

    // return field num in the record.
    FieldId GetRecordCount(const Value& record) const;

    // set variable's offset, they are stored at fixed-data area.
    void SetVariableOffset(Value& record, FieldId id, DataOffset offset) const;

    // set fixed length data, only if length of the data in record equal its definition.
    void _SetFixedSizeValueRaw(Value& record, const Value& data) const;

    // for test only.
    void _SetVariableValueRaw(Value& record, const Value& data) const;

    ENABLE_IF_FIXED_FIELD(T, void)
    SetFixedSizeValue(Value& record, const T& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(!is_vfield_);
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(sizeof(data), field_data_helper::FieldTypeSize(def_.type));
        // copy the buffer so we don't accidentally overwrite memory
        record.Resize(record.Size());
        char* ptr = (char*)record.Data() + GetFieldOffset(record, def_.id);
        ::lgraph::_detail::UnalignedSet<T>(ptr, data);
    }

 private:
    void SetVertexIndex(VertexIndex* index) { vertex_index_.reset(index); }

    void SetEdgeIndex(EdgeIndex* edgeindex) { edge_index_.reset(edgeindex); }

    void SetVectorIndex(VectorIndex* vectorindex) { vector_index_.reset(vectorindex); }

    void SetFullTextIndex(bool fulltext_indexed) { fulltext_indexed_ = fulltext_indexed; }

    void SetFieldId(uint16_t n) { def_.id = n; }

    // return null array pointer.
    char* GetNullArray(const Value& record) const { return record.Data() + nullarray_offset_; }

    size_t GetDataSize(const Value& record) const;

    void* GetFieldPointer(const Value& record) const;
};

}  // namespace _detail
}  // namespace lgraph
