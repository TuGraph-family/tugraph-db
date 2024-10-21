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
class FieldExtractor_v2 {
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
    FieldExtractor_v2() : vertex_index_(nullptr), edge_index_(nullptr) {}

    ~FieldExtractor_v2() {}

    FieldExtractor_v2(const FieldExtractor& rhs) {
        def_ = rhs.def_;
        is_vfield_ = rhs.is_vfield_;
        vertex_index_.reset(rhs.vertex_index_ ? new VertexIndex(*rhs.vertex_index_) : nullptr);
        edge_index_.reset(rhs.edge_index_ ? new EdgeIndex(*rhs.edge_index_) : nullptr);
        fulltext_indexed_ = rhs.fulltext_indexed_;
        vector_index_ = rhs.vector_index_;
        nullarray_offset_ = rhs.nullarray_offset_;
        count_offset_ = rhs.count_offset_;
    }

    FieldExtractor& operator=(const FieldExtractor& rhs) {
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
        count_offset_ = rhs.count_offset_;
        nullarray_offset_ = rhs.nullarray_offset_;
    }

    FieldExtractor& operator=(FieldExtractor&& rhs) noexcept {
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
}

}  // namespace _detail
}  // namespace lgraph
