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

#include <unordered_map>

#include "fma-common/binary_buffer.h"
#include "fma-common/binary_read_write_helper.h"
#include "tools/lgraph_log.h"

#include "core/data_type.h"
#include "core/defs.h"
#include "core/kv_store.h"
#include "core/schema.h"
#include "core/type_convert.h"

namespace lgraph {
/** Manager for vertex or edge schemas.
 *  A maximum of 2^16 labels can be created. Schemas cannot be deleted.
 */
class SchemaManager {
    std::shared_ptr<KvTable> table_;
    std::vector<Schema> schemas_;
    std::unordered_map<std::string, size_t> name_to_idx_;
    bool label_in_record_ = true;

 public:
    /**
     * Opens or creates a table to store schema information.
     *
     * \param [in,out]  txn     The transaction.
     * \param [in,out]  store   The kv-store.
     * \param           name    The name of the table.
     *
     * \return  A kvstore::Table.
     */
    static std::unique_ptr<KvTable> OpenTable(
        KvTransaction& txn, KvStore& store, const std::string& name) {
        return store.OpenTable(txn, name, true, ComparatorDesc::DefaultComparator());
    }

    /**
     * Constructor
     *
     * \param [in,out]  txn     The transaction.
     * \param           table   The table.
     *
     * throws exception if the data in the table is corrupted
     */
    SchemaManager(KvTransaction& txn, std::shared_ptr<KvTable> table, bool label_in_record) {
        table_ = std::move(table);
        label_in_record_ = label_in_record;
        // load schemas from table
        size_t n = table_->GetKeyCount(txn);
        schemas_.resize(n);
        auto it = table_->GetIterator(txn);
        it->GotoFirstKey();
        while (it->IsValid()) {
            LabelId id = it->GetKey().AsType<LabelId>();
            Value v = it->GetValue();
            using namespace fma_common;
            BinaryBuffer buf(v.Data(), v.Size());
            schemas_[id].SetStoreLabelInRecord(label_in_record_);
            if (!BinaryRead(buf, schemas_[id])) {
                THROW_CODE(InternalError, "Invalid schema read from DB.");
            }
            it->Next();
        }
        for (size_t i = 0; i < schemas_.size(); i++) {
            name_to_idx_[schemas_[i].GetLabel()] = i;
        }
    }

    SchemaManager(const SchemaManager& rhs) {
        table_ = rhs.table_;
        schemas_ = rhs.schemas_;
        name_to_idx_ = rhs.name_to_idx_;
        label_in_record_ = rhs.label_in_record_;
    }

    SchemaManager& operator=(const SchemaManager& rhs) {
        if (this == &rhs) return *this;
        table_ = rhs.table_;
        schemas_ = rhs.schemas_;
        name_to_idx_ = rhs.name_to_idx_;
        label_in_record_ = rhs.label_in_record_;
        return *this;
    }

    SchemaManager(SchemaManager&& rhs) {
        table_ = std::move(rhs.table_);
        schemas_ = std::move(rhs.schemas_);
        name_to_idx_ = std::move(rhs.name_to_idx_);
        label_in_record_ = rhs.label_in_record_;
    }

    SchemaManager& operator=(SchemaManager&& rhs) {
        if (this == &rhs) return *this;
        table_ = std::move(rhs.table_);
        schemas_ = std::move(rhs.schemas_);
        name_to_idx_ = std::move(rhs.name_to_idx_);
        label_in_record_ = rhs.label_in_record_;
        return *this;
    }

    /**
     * Gets the label name of the record
     *
     * \param           record  The record.
     *
     * \return  The label.
     */
    const std::string& GetRecordLabel(const Value& record) const {
        FMA_DBG_ASSERT(label_in_record_);
        auto schema = GetSchema(record);
        FMA_DBG_ASSERT(schema);
        return schema->GetLabel();
    }

    size_t GetNumLabels() const { return name_to_idx_.size(); }

    std::vector<const std::string*> GetAllLabelPtrs() const {
        std::vector<const std::string*> lbs;
        lbs.reserve(schemas_.size());
        for (size_t i = 0; i < schemas_.size(); i++) {
            if (!schemas_[i].GetDeleted()) lbs.push_back(&schemas_[i].GetLabel());
        }
        return lbs;
    }

    void RefreshEdgeConstraintsLids(SchemaManager& vertex_sm) {
        for (auto& s : schemas_) {
            std::unordered_map<LabelId, std::unordered_set<LabelId>> lids;
            for (auto& constraint : s.GetEdgeConstraints()) {
                auto src_schema = vertex_sm.GetSchema(constraint.first);
                auto dst_schema = vertex_sm.GetSchema(constraint.second);
                // If schema is nullptr, means it has been deleted.
                // In this case, assign an illegal label id.
                LabelId src = src_schema ? src_schema->GetLabelId()
                                         : std::numeric_limits<LabelId>::max();
                LabelId dst = dst_schema ? dst_schema->GetLabelId()
                                         : std::numeric_limits<LabelId>::max();
                lids[src].insert(dst);
            }
            s.SetEdgeConstraintsLids(std::move(lids));
        }
    }

    std::vector<std::string> GetAllLabels() const {
        std::vector<std::string> lbs;
        lbs.reserve(schemas_.size());
        for (size_t i = 0; i < schemas_.size(); i++) {
            if (!schemas_[i].GetDeleted()) lbs.push_back(schemas_[i].GetLabel());
        }
        return lbs;
    }

    /**
     * Adds a label and set its schema
     *
     * \exception   ::lgraph::SchemaException   Thrown when a Schema error condition occurs.
     *
     * \param [in,out]  txn         The transaction.
     * \param           is_vertex   True if this is vertex label, otherwise it is edge label.
     * \param           label       The label.
     * \param           n_fields    The fields.
     * \param           fields      The fields in the labeled data.
     * \param           options     Cast to VertexOptions when is_vertex is true, else cast to
     * EdgeOptions.
     *
     * \return  True if it succeeds, false if the label already exists. Throws exception on error.
     */
    bool AddLabel(KvTransaction& txn, bool is_vertex, const std::string& label, size_t n_fields,
                  const FieldSpec* fields, const LabelOptions& options) {
        auto it = name_to_idx_.find(label);
        if (it != name_to_idx_.end()) return false;
        Schema* ls = nullptr;
        // reuse deleted schema if there is any
        for (auto& s : schemas_) {
            if (s.GetDeleted()) {
                s.SetDeleted(false);
                ls = &s;
                break;
            }
        }
        if (!ls) {
            schemas_.emplace_back(label_in_record_);
            ls = &schemas_.back();
            if (schemas_.size() > std::numeric_limits<LabelId>::max()) {
                THROW_CODE(InternalError,
                    "Number of labels exceeds limit: {}.\n", std::numeric_limits<LabelId>::max());
            }
            ls->SetLabelId((LabelId)(schemas_.size() - 1));
        }
        std::string primary, temporal;
        TemporalFieldOrder temporal_order;
        if (is_vertex) {
            primary = dynamic_cast<const VertexOptions&>(options).primary_field;
        }
        EdgeConstraints edge_constraints;
        if (!is_vertex) {
            edge_constraints = dynamic_cast<const EdgeOptions&>(options).edge_constraints;
            primary = dynamic_cast<const EdgeOptions&>(options).temporal_field;
            temporal = dynamic_cast<const EdgeOptions&>(options).temporal_field;
            temporal_order = dynamic_cast<const EdgeOptions&>(options).temporal_field_order;
        }
        ls->SetFastAlterSchema(options.fast_alter_schema);
        ls->SetSchema(is_vertex, n_fields, fields, primary, temporal, temporal_order,
                      edge_constraints);
        ls->SetLabel(label);
        ls->SetDetachProperty(options.detach_property);
        name_to_idx_.emplace_hint(it, label, ls->GetLabelId());
        // now write the modification to the kvstore
        using namespace fma_common;
        BinaryBuffer buf;
        BinaryWrite(buf, *ls);
        bool r = table_->SetValue(txn, Value::ConstRef(ls->GetLabelId()),
                                 Value(buf.GetBuf(), buf.GetSize()));
        FMA_DBG_ASSERT(r);
        return true;
    }

    bool AddLabel(KvTransaction& txn, bool is_vertex, const std::string& label,
                  const std::vector<FieldSpec>& fields, const LabelOptions& options) {
        return AddLabel(txn, is_vertex, label, fields.size(), fields.data(), options);
    }

    /**
     * Deletes the label described by label
     *
     * \param [in,out]  txn     The transaction.
     * \param           label   The label.
     *
     * \return  True if it succeeds, false if it fails.
     */
    bool DeleteLabel(KvTransaction& txn, const std::string& label) {
        auto it = name_to_idx_.find(label);
        if (it == name_to_idx_.end()) {
            return false;
        }
        Schema& schema = schemas_[it->second];
        name_to_idx_.erase(it);
        schema.ClearFields();
        schema.SetDeleted(true);
        using namespace fma_common;
        BinaryBuffer buf;
        BinaryWrite(buf, schema);
        bool r = table_->SetValue(txn, Value::ConstRef(schema.GetLabelId()),
                                 Value(buf.GetBuf(), buf.GetSize()));
        FMA_DBG_ASSERT(r);
        return true;
    }

    // replace the schema of label with new_schema
    LabelId AlterLabel(KvTransaction& txn, const std::string& label, const Schema& new_schema) {
        auto it = name_to_idx_.find(label);
        if (it == name_to_idx_.end())
            THROW_CODE(InputError, "Label [{}] does not exist.", label);
        size_t idx = it->second;
        Schema* news = &schemas_[idx];
        *news = new_schema;
        news->SetDeleted(false);
        news->SetLabel(label);
        news->SetLabelId(static_cast<LabelId>(idx));
        // write changes to db
        using namespace fma_common;
        {
            BinaryBuffer buf;
            BinaryWrite(buf, *news);
            bool r = table_->SetValue(txn, Value::ConstRef(news->GetLabelId()),
                                     Value(buf.GetBuf(), buf.GetSize()));
            FMA_DBG_ASSERT(r);
        }
        return news->GetLabelId();
    }

    LabelId GetLabelId(const std::string& label) const {
        auto it = name_to_idx_.find(label);
        if (it == name_to_idx_.end()) {
            THROW_CODE(InputError, "Label \"{}\" does not exist.", label);
        }
        return static_cast<LabelId>(it->second);
    }

    static LabelId GetRecordLabelId(const Value& record) {
        return ::lgraph::_detail::UnalignedGet<LabelId>(record.Data());
    }

    const _detail::FieldExtractorV1* GetExtractor(const Value& record,
                                                const std::string& field) const {
        auto ls = GetSchema(record);
        if (!ls) return nullptr;
        return ls->GetFieldExtractorV1(ls->GetFieldExtractor(field));
    }

    const Schema* GetSchema(const std::string& label) const {
        auto it = name_to_idx_.find(label);
        if (it == name_to_idx_.end()) return nullptr;
        return &schemas_[it->second];
    }

    Schema* GetSchema(const std::string& label) {
        auto it = name_to_idx_.find(label);
        if (it == name_to_idx_.end()) return nullptr;
        return &schemas_[it->second];
    }

    const Schema* GetSchema(const size_t label_num) const {
        if (label_num >= schemas_.size()) return nullptr;
        return &schemas_[label_num];
    }

    Schema* GetSchema(const size_t label_num) {
        if (label_num >= schemas_.size()) return nullptr;
        return &schemas_[label_num];
    }

    const Schema* GetSchema(const Value& record) const {
        FMA_DBG_ASSERT(label_in_record_);
        LabelId lid = GetRecordLabelId(record);
        if (lid >= schemas_.size()) return nullptr;
        return &schemas_[lid];
    }

    Schema* GetSchema(const Value& record) {
        FMA_DBG_ASSERT(label_in_record_);
        LabelId lid = GetRecordLabelId(record);
        if (lid >= schemas_.size()) return nullptr;
        return &schemas_[lid];
    }

    std::string DumpRecord(const Value& record) const {
        auto ls = GetSchema(record);
        FMA_DBG_ASSERT(ls);
        return ls->DumpRecord(record);
    }

    std::string DumpRecord(const Value& record, LabelId lid) const {
        auto ls = GetSchema(lid);
        FMA_DBG_ASSERT(ls);
        return ls->DumpRecord(record);
    }

    void Clear(KvTransaction& txn) {
        schemas_.clear();
        name_to_idx_.clear();
        table_->Drop(txn);
    }

    std::vector<IndexSpec> ListVertexIndexes() const {
        std::vector<IndexSpec> indexes;
        for (auto& schema : schemas_) {
            auto fids = schema.GetIndexedFields();
            for (auto& fid : fids) {
                auto* extractor = schema.GetFieldExtractor(fid);
                VertexIndex* index = extractor->GetVertexIndex();
                FMA_DBG_ASSERT(index);
                IndexSpec is;
                is.label = schema.GetLabel();
                is.field = extractor->Name();
                is.type = index->GetType();
                indexes.push_back(std::move(is));
            }
        }
        return indexes;
    }

    std::vector<CompositeIndexSpec> ListVertexCompositeIndexes() const {
        std::vector<CompositeIndexSpec> indexes;
        for (auto& schema : schemas_) {
            for (auto &spec : schema.GetCompositeIndexSpec()) {
                indexes.push_back(spec);
            }
        }
        return indexes;
    }

    std::vector<IndexSpec> ListEdgeIndexes() const {
        std::vector<IndexSpec> indexes;
        for (auto& schema : schemas_) {
            auto fids = schema.GetIndexedFields();
            for (auto& fid : fids) {
                auto* extractor = schema.GetFieldExtractor(fid);
                EdgeIndex* edge_index = extractor->GetEdgeIndex();
                FMA_DBG_ASSERT(edge_index);
                IndexSpec is;
                is.label = schema.GetLabel();
                is.field = extractor->Name();
                is.type = edge_index->GetType();
                indexes.push_back(std::move(is));
            }
        }
        return indexes;
    }

    template <typename T>
    std::vector<IndexSpec> ListIndexByLabel(const T& label) const {
        const Schema* schema = GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Label [{}] does not exist.", label);
        std::vector<IndexSpec> indexes;
        auto fids = schema->GetIndexedFields();
        for (auto& fid : fids) {
            auto* extractor = schema->GetFieldExtractor(fid);
            VertexIndex* index = extractor->GetVertexIndex();
            FMA_DBG_ASSERT(index);
            IndexSpec is;
            is.label = label;
            is.field = extractor->Name();
            is.type = index->GetType();
            indexes.push_back(std::move(is));
        }
        return indexes;
    }

    template <typename T>
    std::vector<IndexSpec> ListEdgeIndexByLabel(const T& label) const {
        const Schema* schema = GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Label [{}] does not exist.", label);
        std::vector<IndexSpec> indexes;
        auto fids = schema->GetIndexedFields();
        for (auto& fid : fids) {
            auto* extractor = schema->GetFieldExtractor(fid);
            EdgeIndex* edge_index = extractor->GetEdgeIndex();
            FMA_DBG_ASSERT(edge_index);
            IndexSpec is;
            is.label = label;
            is.field = extractor->Name();
            is.type = edge_index->GetType();
            indexes.push_back(std::move(is));
        }
        return indexes;
    }

    std::vector<CompositeIndexSpec> ListVertexCompositeIndexByLabel(
                                    const std::string &label) const {
        const Schema* schema = GetSchema(label);
        if (!schema)
            THROW_CODE(InputError, "Label [{}] does not exist.", label);
        return schema->GetCompositeIndexSpec();
    }

 private:
    static void SetRecordLabelId(Value& record, LabelId id) {
        ::lgraph::_detail::UnalignedSet<LabelId>(record.Data(), id);
    }
};
}  // namespace lgraph
