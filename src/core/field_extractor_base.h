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

// Base class for FieldExtractor, implementing all operations related to Field definitions,
// including type/index retrieval, attribute access and obtaining copies from data.

class FieldExtractorBase {
    friend class Schema;
    FieldSpec def_;
    bool is_vfield_ = false;
    std::unique_ptr<VertexIndex> vertex_index_;
    std::unique_ptr<EdgeIndex> edge_index_;
    // fulltext index
    bool fulltext_indexed_ = false;
    // vector index
    std::shared_ptr<VectorIndex> vector_index_;

 public:
    FieldExtractorBase() : vertex_index_(nullptr), edge_index_(nullptr), vector_index_(nullptr) {}
    virtual ~FieldExtractorBase();
    explicit FieldExtractorBase(const FieldSpec& def) {
        def_ = def;
        is_vfield_ = !field_data_helper::IsFixedLengthFieldType(def.type);
        vertex_index_ = nullptr;
        edge_index_ = nullptr;
        vector_index_ = nullptr;
    }

    FieldExtractorBase(const FieldExtractorBase& rhs) {
        def_ = rhs.GetFieldSpec();
        is_vfield_ = rhs.is_vfield_;
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        vector_index_ = rhs.vector_index_;
    }

    FieldExtractorBase(FieldExtractorBase&& rhs) noexcept {
        def_ = rhs.GetFieldSpec();
        is_vfield_ = rhs.is_vfield_;
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        vector_index_ = std::move(rhs.vector_index_);
        rhs.vertex_index_ = nullptr;
        rhs.edge_index_ = nullptr;
        rhs.vertex_index_ = nullptr;
    }

    FieldExtractorBase& operator=(FieldExtractorBase&& rhs) noexcept {
        if (this == &rhs) return *this;
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vertex_index_ = std::move(rhs.vertex_index_);
        edge_index_ = std::move(rhs.edge_index_);
        vector_index_ = std::move(rhs.vector_index_);
        return *this;
    }

    FieldExtractorBase& operator=(const FieldExtractorBase& rhs) {
        if (this == &rhs) return *this;
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = rhs.vector_index_;
        return *this;
    }

    virtual std::shared_ptr<FieldExtractorBase> Clone() const = 0;

    // Get field info and index info.
    const FieldSpec& GetFieldSpec() const { return def_; }

    const std::string& Name() const { return def_.name; }

    FieldType Type() const { return def_.type; }

    size_t TypeSize() const { return field_data_helper::FieldTypeSize(def_.type); }

    FieldData GetDefaultFieldData() const { return def_.default_value; }

    bool HasDefaultValue() const { return def_.set_default_value; }

    bool IsOptional() const { return def_.optional; }

    bool IsFixedType() const { return field_data_helper::IsFixedLengthFieldType(def_.type); }

    bool IsDeleted() const { return def_.deleted; }

    VertexIndex* GetVertexIndex() const { return vertex_index_.get(); }

    EdgeIndex* GetEdgeIndex() const { return edge_index_.get(); }

    bool FullTextIndexed() const { return fulltext_indexed_; }

    VectorIndex* GetVectorIndex() const { return vector_index_.get(); }

    // Set field info and index info.
    void SetVertexIndex(VertexIndex* index) { vertex_index_.reset(index); }

    void SetEdgeIndex(EdgeIndex* edgeindex) { edge_index_.reset(edgeindex); }

    void SetVectorIndex(VectorIndex* vectorindex) { vector_index_.reset(vectorindex); }

    void SetFullTextIndex(bool fulltext_indexed) { fulltext_indexed_ = fulltext_indexed; }

    void SetFieldSpecId(FieldId field_id) {def_.id = field_id;}

    void MarkDeleted() {
        def_.deleted = true;
        // free data when be marked deleted
        def_.default_value.~FieldData();
        def_.default_value = FieldData();
        def_.set_default_value = false;
    }

    void SetDefaultValue(const FieldData& data) {
        def_.default_value = FieldData(data);
        def_.set_default_value = true;
    }

    // record related. Get or modify record via field_extractor_base.

    // Get

    // Get data size in record, working for both variable and fixed data.
    size_t DataSize(const Value& record) const { return GetDataSize(record); }

    virtual bool GetIsNull(const Value& record) const = 0;

    virtual size_t GetDataSize(const Value& record) const = 0;

    virtual void* GetFieldPointer(const Value& record) const = 0;

    virtual void GetCopyRaw(const Value& record, void* data, size_t size) const = 0;

    virtual size_t GetFieldOffset(const Value& record) const = 0;

    virtual char* GetNullArray(const Value& record) const = 0;

    virtual bool DataInRecord(const Value& record) const = 0;

    virtual Value GetDefaultValue() const = 0;

    virtual FieldId GetFieldId() const = 0;

    virtual void SetFieldId(FieldId id) = 0;

    // Get copy from record.
    ENABLE_IF_FIXED_FIELD(T, void) GetCopy(const Value& record, T& data) const {
        FMA_DBG_ASSERT(field_data_helper::FieldTypeSize(def_.type) == sizeof(data));
        size_t offset = GetFieldOffset(record);
        size_t size = GetDataSize(record);
        // for Field_extractor_v1, size always equals sizeof(T)
        if (size == sizeof(data)) {
            memcpy(&data, (char*)record.Data() + offset, sizeof(data));
        } else {
            // For FieldExtractorV2, even with fixed-length data, there may be cases
            // where the data length in the record does not match the defined length,
            // requiring conversion.
            ConvertData(&data, (char*)record.Data() + offset, size);
        }
    }

    void GetCopy(const Value& record, std::string& data) const;

    void GetCopy(const Value& record, Value& data) const;

    Value GetConstRef(const Value& record) const;

    // Blob related.
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
        }
        throw ParseIncompatibleTypeException(Name(), fd.type, FieldType::BLOB);
    }

    // set record via field_extractor_base.
    virtual void SetIsNull(const Value& record, bool is_null) const = 0;

    std::string FieldToString(const Value& record) const;

    /**
     *  Convert data for integral and floating types.
     *  If we change the data type of floating-point or integer values
     *  (i.e., by altering their defined length), we need to adjust their return values accordingly.
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
};

}  // namespace _detail
}  // namespace lgraph
