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
#include <algorithm>
#include <string>
#include <unordered_map>
#include <vector>

#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"
#include "fma-common/string_formatter.h"
#include "fma-common/text_parser.h"
#include "fma-common/type_traits.h"

#include "core/blob_manager.h"
#include "core/data_type.h"
#include "core/field_extractor.h"
#include "core/schema_common.h"
#include "core/value.h"
#include "core/full_text_index.h"
#include "core/composite_index.h"

namespace lgraph {
class Schema;
class LightningGraph;
class IndexManager;
class SchemaManager;

#define IS_FIELD_TYPE(T) fma_common::IsType<T, std::string, size_t>::value
#define IS_LABEL_TYPE(T) fma_common::IsType<T, std::string, size_t>::value
#define IS_DATA_TYPE(T) fma_common::IsType<T, std::string, FieldData>::value

/** A schema is the description of data types in one record.
 ** The record is layout as the following:
 **     [LabelId] [Null-array] [Fixed-fields] [V-offsets] [V-data]
 ** in which:
 **     LabelId:        indicates the label of the record, different
 **                     label has different schema.
                        LabelId is left out for edges since edges are
                        sorted by LabelId so it becomes part of the key.
 **     Null-array:     records whether a field is null. Each nullable
 **                     field takes one bit
 **     Fixed-fields:   stores all the fixed-length fields one by one
 **     V-offsets:      stores the offsets of the variable-length fields.
 **                     Note that only the offsets from field 1 to N-1
 **                     are recorded, since the first offset is obvious.
 **     V-data:         stores the data of the variable-length fields
*/
class Schema {
    friend class SchemaManager;
    friend class Transaction;
    std::string label_;
    LabelId label_id_ = 0;
    bool label_in_record_ = true;  // whether to store label id in record
    bool deleted_ = false;
    bool is_vertex_ = false;

    std::vector<_detail::FieldExtractor> fields_;
    std::unordered_map<std::string, size_t> name_to_idx_;
    size_t n_fixed_ = 0;
    size_t n_variable_ = 0;
    size_t n_nullable_ = 0;
    size_t v_offset_start_ = 0;

    std::unordered_set<size_t> indexed_fields_;
    std::vector<size_t> blob_fields_;
    std::string primary_field_{};
    // temporal_field_ should be int64.
    std::string temporal_field_{};
    TemporalFieldOrder temporal_order_{};
    // When Schema is VERTEX type, `edge_constraints_` is always empty.
    EdgeConstraints edge_constraints_;
    std::unordered_set<size_t> fulltext_fields_;
    std::unordered_map<LabelId, std::unordered_set<LabelId>> edge_constraints_lids_;
    bool detach_property_ = false;
    std::shared_ptr<KvTable> property_table_;
    std::unordered_map<std::string, std::shared_ptr<CompositeIndex>> composite_index_map;
    std::unordered_set<size_t> vector_index_fields_;

    void SetStoreLabelInRecord(bool b) { label_in_record_ = b; }

    void SetLabel(const std::string& label) { label_ = label; }

    void SetLabelId(LabelId lid) { label_id_ = lid; }

    void SetDetachProperty(bool detach) { detach_property_ = detach; }

    void SetDeleted(bool deleted) { deleted_ = deleted; }

    bool GetDeleted() const { return deleted_; }

    std::string GetCompositeIndexMapKey(const std::vector<std::string> &fields) {
        std::string res = std::to_string(name_to_idx_[fields[0]]);
        int n = fields.size();
        for (int i = 1; i < n; ++i) {
            res += _detail::COMPOSITE_INDEX_KEY_SEPARATOR +
                   std::to_string(name_to_idx_[fields[i]]);
        }
        return res;
    }

    std::string GetCompositeIndexMapKey(const std::vector<size_t> &field_ids) {
        std::string res = std::to_string(field_ids[0]);
        int n = field_ids.size();
        for (int i = 1; i < n; ++i) {
            res += _detail::COMPOSITE_INDEX_KEY_SEPARATOR +
                   std::to_string(field_ids[i]);
        }
        return res;
    }

 public:
    typedef BlobManager::BlobKey BlobKey;

    Schema() {}

    explicit Schema(bool label_in_record) : label_in_record_(label_in_record) {}

    Schema(const Schema& rhs) = default;

    Schema& operator=(const Schema& rhs) = default;

    Schema(Schema&& rhs) = default;

    Schema& operator=(Schema&& rhs) = default;

    explicit Schema(const Value& data) { LoadSchema(data); }

    /**
     * Stores the schema into a Value
     *
     * \return  A Value.
     */
    Value StoreSchema();

    /**
     * Loads the schema from a serialized buffer given in data
     *
     * \param   data    The buffer storing the serialized schema.
     *
     * throws exception if there is error in the schema definition.
     */
    void LoadSchema(const Value& data);

    // clear fields, other contents are kept untouched
    void ClearFields();

    /**
     * Sets the schema. Schema is constructed with empty contents.
     *
     * \param           fields  The field definitons.
     *
     * throws exception if there is error in the schema definition.
     */
    void SetSchema(bool is_vertex, size_t n_fields, const FieldSpec* fields,
                   const std::string& primary, const std::string& temporal,
                   const TemporalFieldOrder& temporal_order,
                   const EdgeConstraints& edge_constraints);

    void SetSchema(bool is_vertex, const std::vector<FieldSpec>& fields, const std::string& primary,
                   const std::string& temporal, const TemporalFieldOrder& temporal_order,
                   const EdgeConstraints& edge_constraints) {
        SetSchema(is_vertex, fields.size(), fields.data(), primary, temporal, temporal_order,
                  edge_constraints);
    }

    void SetEdgeConstraintsLids(std::unordered_map<LabelId, std::unordered_set<LabelId>> lids) {
        edge_constraints_lids_ = std::move(lids);
    }

    const std::unordered_map<LabelId, std::unordered_set<LabelId>>&
    GetEdgeConstraintsLids() const {
        return edge_constraints_lids_;
    }

    // del fields, assuming fields is already de-duplicated
    void DelFields(const std::vector<std::string>& del_fields);

    // add fields, assuming fields are already de-duplicated
    void AddFields(const std::vector<FieldSpec>& add_fields);

    // mod fields, assuming fields are already de-duplicated
    void ModFields(const std::vector<FieldSpec>& mod_fields);

    const std::vector<_detail::FieldExtractor>& GetFields() const { return fields_; }

    //-----------------------
    // const accessors
    const std::string& GetLabel() const { return label_; }
    const std::string& GetTemporalField() const { return temporal_field_; }
    const std::string& GetPrimaryField() const { return primary_field_; }
    bool DetachProperty() const { return detach_property_; }
    bool HasTemporalField() const { return !temporal_field_.empty(); }
    TemporalFieldOrder GetTemporalOrder() const { return temporal_order_; }
    const EdgeConstraints& GetEdgeConstraints() const { return edge_constraints_; }
    void SetEdgeConstraints(const EdgeConstraints& edge_constraints) {
        edge_constraints_ = edge_constraints;
    }
    void SetPropertyTable(std::shared_ptr<KvTable> t) { property_table_ = std::move(t); }
    KvTable& GetPropertyTable() { return *property_table_; }
    void AddDetachedVertexProperty(KvTransaction& txn, VertexId vid, const Value& property);
    Value GetDetachedVertexProperty(KvTransaction& txn, VertexId vid);
    void SetDetachedVertexProperty(KvTransaction& txn, VertexId vid, const Value& property);
    void DeleteDetachedVertexProperty(KvTransaction& txn, VertexId vid);
    void AddDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid, const Value& property);
    Value GetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid);
    void SetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid, const Value& property);
    void DeleteDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid);

    LabelId GetLabelId() const { return label_id_; }

    bool IsVertex() const { return is_vertex_; }

    bool HasBlob() const { return !blob_fields_.empty(); }

    std::vector<const FieldSpec*> GetFieldSpecPtrs() const;

    std::vector<FieldSpec> GetFieldSpecs() const;

    std::map<std::string, FieldSpec> GetFieldSpecsAsMap() const;

    size_t GetNumFields() const { return fields_.size(); }

    const _detail::FieldExtractor* GetFieldExtractor(size_t field_num) const;
    const _detail::FieldExtractor* TryGetFieldExtractor(size_t field_num) const;

    const _detail::FieldExtractor* GetFieldExtractor(const std::string& field_name) const;
    const _detail::FieldExtractor* TryGetFieldExtractor(const std::string& field_name) const;

    size_t GetFieldId(const std::string& name) const;

    size_t GetTemporalFieldId() const { return GetFieldId(GetTemporalField()); }

    int GetTemporalPos(const size_t n_fields, const std::string* field_names) const {
        std::string name = GetTemporalField();
        int ret = -1;
        for (size_t i = 0; i < n_fields; ++i) {
            if (field_names[i] == name) ret = static_cast<int>(i);
        }
        return ret;
    }

    int GetTemporalPos(const size_t n_fields, const size_t* field_ids) const {
        size_t id = GetTemporalFieldId();
        int ret = -1;
        for (size_t i = 0; i < n_fields; ++i) {
            if (field_ids[i] == id) ret = static_cast<int>(i);
        }
        return ret;
    }

    bool TryGetFieldId(const std::string& name, size_t& fid) const;

    std::vector<size_t> GetFieldIds(const std::vector<std::string>& names) const;

    std::string DumpRecord(const Value& record) const;

    // get several fields. Schema must not have blob fields
    template <typename FieldT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), std::vector<FieldData>>::type GetFields(
        Value& record, size_t n_fields, const FieldT* fields) const {
        FMA_DBG_ASSERT(!HasBlob());
        std::vector<FieldData> fds;
        fds.reserve(n_fields);
        for (size_t i = 0; i < n_fields; i++) {
            const _detail::FieldExtractor* fe = GetFieldExtractor(fields[i]);
            if (fe->GetIsNull(record)) return FieldData();
            fds.push_back(GetFieldDataFromField(fe, record));
        }
        return fds;
    }

    // get several fields. Schema can have blob fields
    template <typename FieldT, typename GetBlobByKeyFunc>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), std::vector<FieldData>>::type GetFieldsWithBlob(
        Value& record, size_t n_fields, const FieldT* fields, const GetBlobByKeyFunc& f) const {
        FMA_DBG_ASSERT(HasBlob());
        std::vector<FieldData> fds;
        fds.reserve(n_fields);
        for (size_t i = 0; i < n_fields; i++) {
            const _detail::FieldExtractor* fe = GetFieldExtractor(fields[i]);
            if (fe->GetIsNull(record)) return FieldData();
            fds.push_back(GetFieldDataFromField(fe, record));
        }
        return fds;
    }

    // sets non-blob field
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), void>::type SetField(
        Value& record, const FieldT& name_or_num, const DataT& value) const {
        auto extractor = GetFieldExtractor(name_or_num);
        FMA_DBG_ASSERT(extractor->Type() != FieldType::BLOB);
        extractor->ParseAndSet(record, value);
    }

    // sets blob field
    template <typename FieldT, typename DataT, typename OnLargeBlobFunc>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), void>::type SetBlobField(
        Value& record, const FieldT& name_or_num, const DataT& value,
        const OnLargeBlobFunc& on_large_blob) const {
        auto extractor = GetFieldExtractor(name_or_num);
        FMA_DBG_ASSERT(extractor->Type() == FieldType::BLOB);
        extractor->ParseAndSet(record, value, on_large_blob);
    }

    //// get non-blob field
    // template <typename FieldT>
    // typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type GetField(
    //    const Value& record, const FieldT& field_name_or_num) const {
    //    auto extractor = GetFieldExtractor(field_name_or_num);
    //    if(extractor->GetIsNull(record)) return FieldData();
    //    return GetFieldDataFromField(extractor, record);
    //}

    // get blob field
    template <typename FieldT, typename GetBlobByKeyFunc>
    typename std::enable_if<IS_FIELD_TYPE(FieldT), FieldData>::type GetField(
        const Value& record, const FieldT& field_name_or_num,
        const GetBlobByKeyFunc& get_blob) const {
        auto extractor = TryGetFieldExtractor(field_name_or_num);
        if (!extractor) return FieldData();
        if (extractor->GetIsNull(record)) return FieldData();
        if (_F_UNLIKELY(extractor->Type() == FieldType::BLOB))
            return GetFieldDataFromBlobField(extractor, record, get_blob);
        else
            return GetFieldDataFromField(extractor, record);
    }

    // Create a record given properties as string or FieldData.
    // For string input,
    template <typename FieldT, typename DataT>
    typename std::enable_if<IS_FIELD_TYPE(FieldT) && IS_DATA_TYPE(DataT), Value>::type CreateRecord(
        size_t n_fields, const FieldT* fields, const DataT* values) const {
        FMA_DBG_ASSERT(!HasBlob());
        // TODO(anyone): optimize
        Value v = CreateEmptyRecord();
        std::vector<bool> is_set(fields_.size(), false);
        for (size_t i = 0; i < n_fields; i++) {
            const FieldT& name_or_num = fields[i];
            const DataT& data = values[i];
            const _detail::FieldExtractor* extr = GetFieldExtractor(name_or_num);
            is_set[extr->GetFieldId()] = true;
            extr->ParseAndSet(v, data);
        }
        for (size_t i = 0; i < fields_.size(); i++) {
            auto& f = fields_[i];
            if (_F_UNLIKELY(!f.IsOptional() && !is_set[i]))
                throw FieldCannotBeSetNullException(f.Name());
        }
        return v;
    }

    // create property with data, which contains blobs in raw format.
    // Everytime a large blob is found, on_large_blob is called given the blob value.
    // on_large_blob should write out the blob content and return its corresponding BlobKey.
    template <typename FT, typename DT, typename StoreLargeBlobFunc>
    Value CreateRecordWithBlobs(size_t n_fields, const FT* fields, const DT* values,
                                const StoreLargeBlobFunc& on_large_blob) {
        FMA_DBG_ASSERT(HasBlob());
        Value prop = CreateEmptyRecord();
        std::vector<bool> is_set(fields_.size(), false);
        for (size_t i = 0; i < n_fields; i++) {
            const FT& name_or_num = fields[i];
            const DT& data = values[i];
            const _detail::FieldExtractor* extr = GetFieldExtractor(name_or_num);
            is_set[extr->GetFieldId()] = true;
            if (_F_UNLIKELY(extr->Type() == FieldType::BLOB)) {
                extr->ParseAndSetBlob(prop, data, on_large_blob);
            } else {
                extr->ParseAndSet(prop, data);
            }
        }
        for (size_t i = 0; i < fields_.size(); i++) {
            auto& f = fields_[i];
            if (_F_UNLIKELY(!f.IsOptional() && !is_set[i]))
                throw FieldCannotBeSetNullException(f.Name());
        }
        return prop;
    }

    // copy field values from src to dst
    // dst must be a record created with this schema
    // fields are assumed to have the same type
    void CopyFieldsRaw(Value& dst, const std::vector<size_t> fids_in_dst, const Schema* src_schema,
                       const Value& src, const std::vector<size_t> fids_in_src);

    //----------------------
    // index related
    void MarkVertexIndexed(size_t field_idx, VertexIndex* index) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        indexed_fields_.insert(field_idx);
        fields_[field_idx].SetVertexIndex(index);
    }

    void MarkEdgeIndexed(size_t field_idx, EdgeIndex* edge_index) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        indexed_fields_.insert(field_idx);
        fields_[field_idx].SetEdgeIndex(edge_index);
    }

    void MarkVectorIndexed(size_t field_idx, VectorIndex* index) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        vector_index_fields_.insert(field_idx);
        fields_[field_idx].SetVectorIndex(index);
    }

    bool IsVertexIndex(size_t field_idx) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        return fields_[field_idx].GetVertexIndex() == nullptr;
    }

    bool IsEdgeIndex(size_t field_idx) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        return fields_[field_idx].GetEdgeIndex() == nullptr;
    }

    bool IsVectorIndex(size_t field_idx) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        return fields_[field_idx].GetVectorIndex() == nullptr;
    }

    void UnVertexIndex(size_t field_idx) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        indexed_fields_.erase(field_idx);
        fields_[field_idx].SetVertexIndex(nullptr);
    }

    void UnEdgeIndex(size_t field_idx) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        indexed_fields_.erase(field_idx);
        fields_[field_idx].SetEdgeIndex(nullptr);
    }

    void UnVertexCompositeIndex(const std::vector<std::string> &fields) {
        composite_index_map.erase(GetCompositeIndexMapKey(fields));
    }

    void UnVectorIndex(size_t field_idx) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        vector_index_fields_.erase(field_idx);
        fields_[field_idx].SetVectorIndex(nullptr);
    }

    void MarkFullTextIndexed(size_t field_idx, bool fulltext_indexed) {
        FMA_DBG_ASSERT(field_idx < fields_.size());
        if (!fulltext_indexed) {
            fulltext_fields_.erase(field_idx);
        } else {
            fulltext_fields_.emplace(field_idx);
        }
        fields_[field_idx].SetFullTextIndex(fulltext_indexed);
    }

    const std::unordered_set<size_t>& GetIndexedFields() const { return indexed_fields_; }
    const std::unordered_set<size_t>& GetFullTextFields() const { return fulltext_fields_; }

    std::vector<CompositeIndexSpec> GetCompositeIndexSpec() const;

    void DeleteVertexIndex(KvTransaction& txn, VertexId vid, const Value& record);

    void DeleteEdgeIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record);

    void DeleteVertexCompositeIndex(KvTransaction& txn, VertexId vid, const Value& record);

    void DeleteCreatedEdgeIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record,
                                const std::vector<size_t>& created);

    void DeleteVertexFullTextIndex(VertexId vid, std::vector<FTIndexEntry>& buffers);

    void DeleteEdgeFullTextIndex(EdgeUid euid, std::vector<FTIndexEntry>& buffers);

    /**
     * Delete the residual indexes of the vertex at `vid` (excluding the unique index value).
     * Note: Currently this function is only used to delete and clean up residual indexes
     * after AddVertexToIndex fails.
     */
    void DeleteCreatedVertexIndex(KvTransaction& txn, VertexId vid, const Value& record,
                                  const std::vector<size_t>& created);

    void AddVertexToIndex(KvTransaction& txn, VertexId vid, const Value& record,
                          std::vector<size_t>& created);

    void AddVertexToCompositeIndex(KvTransaction& txn, VertexId vid, const Value& record,
                          std::vector<std::string >& created);
    bool VertexUniqueIndexConflict(KvTransaction& txn, const Value& record);

    void AddEdgeToIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record,
                        std::vector<size_t>& created);
    bool EdgeUniqueIndexConflict(KvTransaction& txn, const Value& record);

    void AddVectorToVectorIndex(KvTransaction& txn, VertexId vid, const Value& record);

    void DeleteVectorIndex(KvTransaction& txn, VertexId vid, const Value& record);

    void AddVertexToFullTextIndex(VertexId vid, const Value& record,
                                  std::vector<FTIndexEntry>& buffers);
    void AddEdgeToFullTextIndex(EdgeUid euid, const Value& record,
                                std::vector<FTIndexEntry>& buffers);

    void SetCompositeIndex(const std::vector<std::string> &fields, CompositeIndex* index) {
        composite_index_map.emplace(GetCompositeIndexMapKey(fields),
                                    std::make_shared<CompositeIndex>(*index));
    }

    CompositeIndex* GetCompositeIndex(const std::vector<std::string> &fields) {
        auto it = composite_index_map.find(GetCompositeIndexMapKey(fields));
        if (it == composite_index_map.end()) return nullptr;
        return it->second.get();
    }

    CompositeIndex* GetCompositeIndex(const std::vector<size_t> &field_ids) {
        auto it = composite_index_map.find(GetCompositeIndexMapKey(field_ids));
        if (it == composite_index_map.end()) return nullptr;
        return it->second.get();
    }

    std::vector<std::vector<std::string>> GetRelationalCompositeIndexKey(
        const std::vector<size_t> &fields);

    //----------------------
    // serialize/deserialize
    template <typename StreamT>
    size_t Deserialize(StreamT& buf) {
        size_t bytes_read = 0;
        size_t s = BinaryRead(buf, label_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, label_id_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, label_in_record_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, deleted_);
        if (!s) return 0;
        bytes_read += s;
        std::vector<FieldSpec> fds;
        s = BinaryRead(buf, fds);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, is_vertex_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, primary_field_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, temporal_field_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, temporal_order_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, edge_constraints_);
        if (!s) return 0;
        bytes_read += s;
        s = BinaryRead(buf, detach_property_);
        if (!s) return 0;
        bytes_read += s;
        SetSchema(is_vertex_, fds, primary_field_, temporal_field_, temporal_order_,
                  edge_constraints_);
        return bytes_read;
    }

    template <typename StreamT>
    size_t Serialize(StreamT& buf) const {
        return BinaryWrite(buf, label_) + BinaryWrite(buf, label_id_) +
               BinaryWrite(buf, label_in_record_) + BinaryWrite(buf, deleted_) +
               BinaryWrite(buf, GetFieldSpecs()) + BinaryWrite(buf, is_vertex_) +
               BinaryWrite(buf, primary_field_) + BinaryWrite(buf, temporal_field_) +
               BinaryWrite(buf, temporal_order_) + BinaryWrite(buf, edge_constraints_) +
               BinaryWrite(buf, detach_property_);
    }

    std::string ToString() const { return fma_common::ToString(GetFieldSpecsAsMap()); }

    /**
     * Creates an empty record
     *
     * \param [in,out]  v           Value to store the result.
     * \param           size_hint   (Optional) Hint of size of the record, used to
     * reduce memory realloc.
     */
    Value CreateEmptyRecord(size_t size_hint = 0) const;

    // only used for property detach
    Value CreateRecordWithLabelId() const;

 protected:
    FieldData GetFieldDataFromField(const _detail::FieldExtractor* extractor,
                                    const Value& record) const;

    template <typename GetBlobFunc>
    FieldData GetFieldDataFromBlobField(const _detail::FieldExtractor* extractor,
                                        const Value& record, const GetBlobFunc& get_blob) const {
        return FieldData::Blob(extractor->GetBlobConstRef(record, get_blob).AsString());
    }

    void RefreshLayout();
};  // Schema
}  // namespace lgraph
