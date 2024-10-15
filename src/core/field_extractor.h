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

/** A field extractor can be used to get/set a field in the record. */
class FieldExtractor {
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
    FieldExtractor() : vertex_index_(nullptr), edge_index_(nullptr) {}

    ~FieldExtractor() {}

    FieldExtractor(const FieldExtractor& rhs) {
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = rhs.vector_index_;
    }

    FieldExtractor& operator=(const FieldExtractor& rhs) {
        if (this == &rhs) return *this;
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = rhs.vector_index_;
        return *this;
    }

    FieldExtractor(FieldExtractor&& rhs) noexcept {
        def_ = std::move(rhs.def_);
        is_vfield_ = rhs.is_vfield_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        rhs.vertex_index_ = nullptr;
        rhs.edge_index_ = nullptr;
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = std::move(rhs.vector_index_);
        rhs.vector_index_ = nullptr;
    }

    FieldExtractor& operator=(FieldExtractor&& rhs) noexcept {
        if (this == &rhs) return *this;
        def_ = std::move(rhs.def_);
        is_vfield_ = rhs.is_vfield_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = std::move(rhs.vector_index_);
        return *this;
    }

    // for test only
    explicit FieldExtractor(const FieldSpec& d) noexcept : def_(d) {
        is_vfield_ = !field_data_helper::IsFixedLengthFieldType(d.type);
        vertex_index_ = nullptr;
        edge_index_ = nullptr;
    }

    explicit FieldExtractor(const FieldSpec& d, const FieldId id) noexcept : def_(d) {
        is_vfield_ = !field_data_helper::IsFixedLengthFieldType(d.type);
        vertex_index_ = nullptr;
        edge_index_ = nullptr;
        SetFieldId(id);
    }

    const FieldSpec& GetFieldSpec() const { return def_; }

    bool GetIsNull(const Value& record) const {
        if (!def_.optional) {
            return false;
        }
        // get the Kth bit from NullArray
        const char* arr = GetNullArray(record);
        return arr[def_.id / 8] & (0x1 << (def_.id % 8));
    }

    FieldData GetDefaultValue() const { return def_.default_value; }

    FieldData GetInitedValue() const { return def_.init_value; }

    bool HasDefaultValue() const { return def_.set_default_value; }

    bool HasInitedValue() const { return def_.inited_value; }

    void SetDefaultValue(const FieldData& data) {
        def_.default_value = FieldData(data);
        def_.set_default_value = true;
    }

    void SetInitValue(const FieldData& data) {
        def_.init_value = FieldData(data);
        def_.inited_value = true;
    }

    void SetLabelInRecord(const bool label_in_record) {
        label_in_record_ = label_in_record;
        count_offset_ = sizeof(VersionId) + label_in_record ? sizeof(LabelId) : 0;
        nullarray_offset_ = count_offset_ + sizeof(FieldId);
    }

    void MarkDeleted() {
        def_.deleted = true;
        // free data space when marked deleted
        def_.init_value.~FieldData();
        def_.default_value.~FieldData();
        def_.inited_value = false;
        def_.set_default_value = false;
    }

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
     *  Convert data for integral and floating types.
     *  If we change the data type of floating-point or integer values
     *  (i.e., by altering their defined length), we need to adjust their values accordingly.
     *  For example, when converting from INT64 to INT8 (a relatively rare operation),
     *  we need to return an appropriate value within the range of the new type.
     *  This approach allows us to retain the original value when modifying the data type,
     *  without requiring a complete scan of the data to generate a new field.
     */

    ENABLE_IF_FIXED_FIELD(T, void) static ConvertData(T* dst, const char* data, size_t size) {
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

    /**
     * Extracts a copy of field into the string.
     *
     * \param           record  The record.
     * \param [in,out]  data    The result data.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopy(const Value& record, std::string& data) const {
        FMA_DBG_ASSERT(Type() != FieldType::BLOB);
        data.resize(GetDataSize(record));
        GetCopyRaw(record, &data[0], data.size());
    }

    /**
     * Extracts field data from the record
     *
     * \param           record  The record.
     * \param [in,out]  data    The result.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopy(const Value& record, Value& data) const {
        data.Resize(GetDataSize(record));
        GetCopyRaw(record, data.Data(), data.Size());
    }

    // Gets a const reference of the field.
    // Formatted data is returned for blob, which means [is_large_blob] [blob_data | blob_key]
    Value GetConstRef(const Value& record) const {
        if (GetIsNull(record)) return Value();
        return Value((char*)GetFieldPointer(record), GetDataSize(record));
    }

    // gets a const ref to the blob content
    // get_blob_by_key is a function that accepts BlobKey and returns Value containing blob content
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

    const std::string& Name() const { return def_.name; }

    FieldType Type() const { return def_.type; }

    size_t TypeSize() const { return field_data_helper::FieldTypeSize(def_.type); }

    size_t DataSize(const Value& record) const { return GetDataSize(record); }

    bool IsOptional() const { return def_.optional; }

    bool IsFixedType() const { return field_data_helper::IsFixedLengthFieldType(def_.type); }

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

    VertexIndex* GetVertexIndex() const { return vertex_index_.get(); }

    EdgeIndex* GetEdgeIndex() const { return edge_index_.get(); }

    bool FullTextIndexed() const { return fulltext_indexed_; }

    VectorIndex* GetVectorIndex() const { return vector_index_.get(); }

    uint16_t GetFieldId() const { return def_.id; }

 private:
    void SetVertexIndex(VertexIndex* index) { vertex_index_.reset(index); }

    void SetEdgeIndex(EdgeIndex* edgeindex) { edge_index_.reset(edgeindex); }

    void SetVectorIndex(VectorIndex* vectorindex) { vector_index_.reset(vectorindex); }

    void SetFullTextIndex(bool fulltext_indexed) { fulltext_indexed_ = fulltext_indexed; }

    void SetFieldId(uint16_t n) { def_.id = n; }

    //-----------------------
    // record accessors

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

    template <FieldType FT>
    void _ParseStringAndSet(Value& record, const std::string& data) const;

    void SetVariableOffset(Value& record, FieldId id, DataOffset offset) const {
        size_t off = GetFieldOffset(record, id);
        ::lgraph::_detail::UnalignedSet<DataOffset>(record.Data() + off, offset);
    }

    void _SetFixedSizeValueRaw(Value& record, const Value& data) const {
        // "Cannot call SetField(Value&, const T&) on a variable length field";
        FMA_DBG_ASSERT(!is_vfield_);
        // "Type size mismatch"
        FMA_DBG_CHECK_EQ(data.Size(), field_data_helper::FieldTypeSize(def_.type));
        FMA_DBG_CHECK_EQ(data.Size(), GetDataSize(record));
        // copy the buffer so we don't accidentally overwrite memory
        char* ptr = (char*)record.Data() + GetFieldOffset(record, def_.id);
        memcpy(ptr, data.Data(), data.Size());
    }

    // set field value to null
    void SetIsNull(const Value& record, const bool is_null) const {
        if (!def_.optional) {
            if (is_null) throw FieldCannotBeSetNullException(Name());
            return;
        }
        // set the Kth bit from NullArray
        char* arr = GetNullArray(record);
        if (is_null) {
            arr[def_.id / 8] |= (0x1 << (def_.id % 8));
        } else {
            arr[def_.id / 8] &= ~(0x1 << (def_.id % 8));
        }
    }

    /**
     * Extracts field data from the record to the buffer pointed to by data. This
     * is for internal use only, the size MUST match the data size defined in schema.
     *
     * \param           record  The record.
     * \param [in,out]  data    If non-null, the data.
     * \param           size    Size of field, must be equal to field size.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopyRaw(const Value& record, void* data, size_t size) const {
        size_t off = GetFieldOffset(record, def_.id);
        FMA_DBG_ASSERT(off + size <= record.Size());
        memcpy(data, record.Data() + off, size);
    }

    char* GetNullArray(const Value& record) const { return record.Data() + nullarray_offset_; }

    size_t GetDataSize(const Value& record) const {
        if (is_vfield_) {
            DataOffset var_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(
                record.Data() + GetFieldOffset(record, def_.id));
            DataOffset var_data_offset =
                ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + var_offset);
            // The length is stored at the beginning of the variable-length field data area.
            return ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + var_data_offset);
        } else {
            return GetFieldOffset(record, def_.id + 1) - GetFieldOffset(record, def_.id);
        }
    }

    FieldId GetRecordCount(const Value& record) const {
        return ::lgraph::_detail::UnalignedGet<FieldId>(record.Data() + count_offset_);
    }

    /** Retrieve the starting position of the Field data for the given ID.
     *  Note that both fixed-length and variable-length data are not distinguished here.
     */
    size_t GetFieldOffset(const Value& record, const FieldId id) const {
        const uint16_t count = GetRecordCount(record);
        if (0 == id) {
            // The starting position of Field0 is at the end of the offset section.
            return nullarray_offset_ + (count + 7) / 8 + count * sizeof(DataOffset);
        }

        size_t offset = 0;
        offset = nullarray_offset_ + (count + 7) / 8 + (id - 1) * sizeof(DataOffset);
        return ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + offset);
    }

    size_t GetOffsetPosition(const Value& record, const FieldId id) const {
        const FieldId count = GetRecordCount(record);
        if (0 == id) {
            return 0;
        }
        return nullarray_offset_ + (count + 7) / 8 + (id - 1) * sizeof(DataOffset);
    }
    void* GetFieldPointer(const Value& record) const {
        return (char*)record.Data() + GetFieldOffset(record, def_.id);
    }
};

}  // namespace _detail

}  // namespace lgraph
