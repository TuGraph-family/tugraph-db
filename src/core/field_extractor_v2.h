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
#include "lgraph/lgraph_types.h"

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
    FieldSpecV2 def_;
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

    explict FieldExtractorV2(const FieldExtractorV2& rhs) {
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = rhs.vector_index_;
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
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

    FieldExtractorV2(FieldExtractorV2&& rhs) noexcept {
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

    const FieldSpec& GetFieldSpec() const { return def_; }

    bool GetIsNull(const Value& record) const;

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

    void SetLabelInRecord(const bool label_in_record) const;

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
    ENABLE_IF_FIXED_FIELD(T, void) GetCopy(const Value& record, T& data) const;

    /**
     * Extracts a copy of field into the string.
     *
     * \param           record  The record.
     * \param [in,out]  data    The result data.
     *
     * Assert fails if data is corrupted.
     */
    void GetCopy(const Value& record, std::string& data) const

        /**
         * Extracts field data from the record
         *
         * \param           record  The record.
         * \param [in,out]  data    The result.
         *
         * Assert fails if data is corrupted.
         */
        void GetCopy(const Value& record, Value& data) const;

    /**
     *  Convert data for integral and floating types.
     *  If we change the data type of floating-point or integer values
     *  (i.e., by altering their defined length), we need to adjust their values accordingly.
     *  For example, when converting from INT64 to INT8 (a relatively rare operation),
     *  we need to return an appropriate value within the range of the new type.
     *  This approach allows us to retain the original value when modifying the data type,
     *  without requiring a complete scan of the data to generate a new field.
     */
    ENABLE_IF_FIXED_FIELD(T, void) static ConvertData(T* dst, const char* data, size_t size);
}

}  // namespace _detail
}  // namespace lgraph
