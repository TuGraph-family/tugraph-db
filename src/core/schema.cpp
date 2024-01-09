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

#include "fma-common/string_formatter.h"

#include "core/vertex_index.h"
#include "core/edge_index.h"
#include "core/schema.h"
#include "import/import_config_parser.h"

namespace lgraph {

void Schema::DeleteEdgeFullTextIndex(EdgeUid euid, std::vector<FTIndexEntry>& buffers) {
    if (fulltext_fields_.empty()) {
        return;
    }
    FTIndexEntry entry;
    entry.type = FTIndexEntryType::DELETE_EDGE;
    entry.vid1 = euid.src;
    entry.vid2 = euid.dst;
    entry.lid = euid.lid;
    entry.eid = euid.eid;
    buffers.emplace_back(std::move(entry));
}

void Schema::DeleteVertexFullTextIndex(VertexId vid, std::vector<FTIndexEntry>& buffers) {
    if (fulltext_fields_.empty()) {
        return;
    }
    FTIndexEntry entry;
    entry.type = FTIndexEntryType::DELETE_VERTEX;
    entry.vid1 = vid;
    buffers.emplace_back(std::move(entry));
}

void Schema::DeleteVertexIndex(KvTransaction& txn, VertexId vid, const Value& record) {
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        VertexIndex* index = fe.GetVertexIndex();
        FMA_ASSERT(index);
        // update field index
        if (!index->Delete(txn, fe.GetConstRef(record), vid)) {
            throw InputError(FMA_FMT("Failed to un-index vertex [{}] with field "
                                                    "value [{}:{}]: index value does not exist.",
                                                    vid, fe.Name(), fe.FieldToString(record)));
        }
    }
}

void Schema::DeleteCreatedVertexIndex(KvTransaction& txn, VertexId vid, const Value& record,
                                      const std::vector<size_t>& created) {
    for (auto& idx : created) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        VertexIndex* index = fe.GetVertexIndex();
        FMA_ASSERT(index);
        // the aim of this method is delete the index that has been created
        if (!index->Delete(txn, fe.GetConstRef(record), vid)) {
            throw InputError(FMA_FMT("Failed to un-index vertex [{}] with field "
                                                    "value [{}:{}]: index value does not exist.",
                                                    vid, fe.Name(), fe.FieldToString(record)));
        }
    }
}

void Schema::AddEdgeToFullTextIndex(EdgeUid euid, const Value& record,
                                    std::vector<FTIndexEntry>& buffers) {
    if (fulltext_fields_.empty()) {
        return;
    }
    FTIndexEntry entry;
    entry.type = FTIndexEntryType::ADD_EDGE;
    entry.vid1 = euid.src;
    entry.vid2 = euid.dst;
    entry.eid = euid.eid;
    entry.lid = euid.lid;
    for (auto& idx : fulltext_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        entry.kvs.emplace_back(fe.Name(), fe.FieldToString(record));
    }
    buffers.emplace_back(std::move(entry));
}

void Schema::AddVertexToFullTextIndex(VertexId vid, const Value& record,
                                      std::vector<FTIndexEntry>& buffers) {
    if (fulltext_fields_.empty()) {
        return;
    }
    FTIndexEntry entry;
    entry.type = FTIndexEntryType::ADD_VERTEX;
    entry.vid1 = vid;
    entry.lid = label_id_;
    for (auto& idx : fulltext_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        entry.kvs.emplace_back(fe.Name(), fe.FieldToString(record));
    }
    buffers.emplace_back(std::move(entry));
}

void Schema::AddVertexToIndex(KvTransaction& txn, VertexId vid, const Value& record,
                              std::vector<size_t>& created) {
    created.reserve(fields_.size());
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        VertexIndex* index = fe.GetVertexIndex();
        FMA_ASSERT(index);
        // update field index
        if (!index->Add(txn, fe.GetConstRef(record), vid)) {
            throw InputError(FMA_FMT(
                "Failed to index vertex [{}] with field value [{}:{}]: index value already exists.",
                vid, fe.Name(), fe.FieldToString(record)));
        }
        created.push_back(idx);
    }
}

void Schema::DeleteEdgeIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record) {
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        EdgeIndex* index = fe.GetEdgeIndex();
        FMA_ASSERT(index);
        // update field index
        if (!index->Delete(txn, fe.GetConstRef(record), euid)) {
            throw InputError(FMA_FMT("Failed to un-index edge with field "
                                                    "value [{}:{}]: index value does not exist.",
                                                    fe.Name(), fe.FieldToString(record)));
        }
    }
}

void Schema::DeleteCreatedEdgeIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record,
                                    const std::vector<size_t>& created) {
    for (auto& idx : created) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        EdgeIndex* index = fe.GetEdgeIndex();
        FMA_ASSERT(index);
        // the aim of this method is delete the index that has been created
        if (!index->Delete(txn, fe.GetConstRef(record), euid)) {
            throw InputError(FMA_FMT("Failed to un-index edge with field "
                                                    "value [{}:{}]: index value does not exist.",
                                                    fe.Name(), fe.FieldToString(record)));
        }
    }
}

void Schema::AddEdgeToIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record,
                            std::vector<size_t>& created) {
    created.reserve(fields_.size());
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        EdgeIndex* index = fe.GetEdgeIndex();
        FMA_ASSERT(index);
        // update field index
        if (!index->Add(txn, fe.GetConstRef(record), euid)) {
            throw InputError(FMA_FMT(
                "Failed to index edge with field value [{}:{}]: index value already exists.",
                fe.Name(), fe.FieldToString(record)));
        }
        created.push_back(idx);
    }
}

FieldData Schema::GetFieldDataFromField(const _detail::FieldExtractor* extractor,
                                        const Value& record) const {
#define _GET_COPY_AND_RETURN_FD(ft)                                                    \
    do {                                                                               \
        typename field_data_helper::FieldType2StorageType<FieldType::ft>::type sd;     \
        extractor->GetCopy(record, sd);                                                \
        return FieldData(                                                              \
            static_cast<field_data_helper::FieldType2CType<FieldType::ft>::type>(sd)); \
    } while (0)

    switch (extractor->Type()) {
    case FieldType::BOOL:
        _GET_COPY_AND_RETURN_FD(BOOL);
    case FieldType::INT8:
        _GET_COPY_AND_RETURN_FD(INT8);
    case FieldType::INT16:
        _GET_COPY_AND_RETURN_FD(INT16);
    case FieldType::INT32:
        _GET_COPY_AND_RETURN_FD(INT32);
    case FieldType::INT64:
        _GET_COPY_AND_RETURN_FD(INT64);
    case FieldType::DATE:
        _GET_COPY_AND_RETURN_FD(DATE);
    case FieldType::DATETIME:
        _GET_COPY_AND_RETURN_FD(DATETIME);
    case FieldType::FLOAT:
        _GET_COPY_AND_RETURN_FD(FLOAT);
    case FieldType::DOUBLE:
        _GET_COPY_AND_RETURN_FD(DOUBLE);
    case FieldType::VECTOR:
        {
            std::vector<float> vec;
            std::string str(extractor->GetConstRef(record).AsString());
            std::regex pattern("-?[0-9]+\\.?[0-9]*");
            std::sregex_iterator begin_it(str.begin(), str.end(), pattern), end_it;
            while (begin_it != end_it) 
            {  
                std::smatch match = *begin_it;  
                vec.push_back(std::stof(match.str()));  
                ++begin_it; 
            }    
            return FieldData(vec);
        }
    case FieldType::STRING:
        return FieldData(extractor->GetConstRef(record).AsString());
    case FieldType::BLOB:
        FMA_ERR() << "BLOB cannot be obtained directly, use GetFieldDataFromField(Value, "
                     "Extractor, GetBlobKeyFunc)";
    case FieldType::POINT:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                throw InputError("invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(PointWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(PointCartesian(EWKB));
            default:
                throw InputError("invalid srid!\n");
        }
    }

    case FieldType::LINESTRING:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                throw InputError("invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(LineStringWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(LineStringCartesian(EWKB));
            default:
                throw InputError("invalid srid!\n");
        }
    }

    case FieldType::POLYGON:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                throw InputError("invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(PolygonWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(PolygonCartesian(EWKB));
            default:
                throw InputError("invalid srid!\n");
        }
    }

    case FieldType::SPATIAL:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                throw InputError("invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(SpatialWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(SpatialCartesian(EWKB));
            default:
                throw InputError("invalid srid!\n");
        }
    }
    case FieldType::NUL:
        FMA_ERR() << "FieldType NUL";
    }
    return FieldData();
}

void Schema::CopyFieldsRaw(Value& dst, const std::vector<size_t> fids_in_dst,
                           const Schema* src_schema, const Value& src,
                           const std::vector<size_t> fids_in_src) {
    FMA_DBG_ASSERT(fids_in_dst.size() == fids_in_src.size());
    dst.Resize(dst.Size());
    for (size_t i = 0; i < fids_in_dst.size(); i++) {
        const _detail::FieldExtractor* dst_fe = GetFieldExtractor(fids_in_dst[i]);
        const _detail::FieldExtractor* src_fe = src_schema->GetFieldExtractor(fids_in_src[i]);
        dst_fe->CopyDataRaw(dst, src, src_fe);
    }
}

void Schema::RefreshLayout() {
    // check field types
    // check if there is any blob
    blob_fields_.clear();
    for (size_t i = 0; i < fields_.size(); i++) {
        auto& f = fields_[i];
        if (f.Type() == FieldType::NUL) throw FieldCannotBeNullTypeException(f.Name());
        if (f.Type() == FieldType::BLOB) blob_fields_.push_back(i);
    }
    // if label is included in record, data starts after LabelId
    size_t data_start_off = label_in_record_ ? sizeof(LabelId) : 0;
    // setup name_to_fields
    name_to_idx_.clear();
    for (size_t i = 0; i < fields_.size(); i++) {
        auto& f = fields_[i];
        f.SetFieldId(i);
        f.SetNullableArrayOff(data_start_off);
        if (_F_UNLIKELY(name_to_idx_.find(f.Name()) != name_to_idx_.end()))
            throw FieldAlreadyExistsException(f.Name());
        name_to_idx_[f.Name()] = i;
    }
    // layout nullable array
    n_nullable_ = 0;
    for (auto& f : fields_) {
        if (f.IsOptional()) {
            f.SetNullableOff(n_nullable_);
            n_nullable_++;
        }
    }
    v_offset_start_ = data_start_off + (n_nullable_ + 7) / 8;
    // layout the fixed fields
    n_fixed_ = 0;
    n_variable_ = 0;
    for (auto& f : fields_) {
        if (field_data_helper::IsFixedLengthFieldType(f.Type())) {
            n_fixed_++;
            f.SetFixedLayoutInfo(v_offset_start_);
            v_offset_start_ += f.TypeSize();
        } else {
            n_variable_++;
        }
    }
    // now, layout the variable fields
    size_t vidx = 0;
    for (auto& f : fields_) {
        if (!field_data_helper::IsFixedLengthFieldType(f.Type()))
            f.SetVLayoutInfo(v_offset_start_, n_variable_, vidx++);
    }
    // finally, check the indexed fields
    indexed_fields_.clear();
    bool found_primary = false;
    for (auto& f : fields_) {
        if (!f.GetVertexIndex() && !f.GetEdgeIndex()) continue;
        indexed_fields_.emplace_hint(indexed_fields_.end(), f.GetFieldId());
        if (f.Name() == primary_field_) {
            FMA_ASSERT(!found_primary);
            found_primary = true;
        }
    }
    // vertex must have primary property
    if (is_vertex_ && !indexed_fields_.empty()) {
        FMA_ASSERT(found_primary);
    }

    fulltext_fields_.clear();
    for (auto& f : fields_) {
        if (!f.FullTextIndexed()) continue;
        fulltext_fields_.emplace(f.GetFieldId());
    }
}

/**
 * Creates an empty record
 *
 * \param [in,out]  v           Value to store the result.
 * \param           size_hint   (Optional) Hint of size of the record, used to
 * reduce memory realloc.
 */
Value Schema::CreateEmptyRecord(size_t size_hint) const {
    Value v(size_hint);
    size_t min_size = v_offset_start_;
    if (n_variable_ > 0) min_size += sizeof(DataOffset) * (n_variable_ - 1);
    v.Resize(min_size);
    // first data is the LabelId
    if (label_in_record_) {
        ::lgraph::_detail::UnalignedSet<LabelId>(v.Data(), label_id_);
        // nullbable bits
        memset(v.Data() + sizeof(LabelId), 0xFF, (n_nullable_ + 7) / 8);
    } else {
        // nullbable bits
        memset(v.Data(), 0xFF, (n_nullable_ + 7) / 8);
    }
    // initialize variable length array offsets
    if (n_variable_ > 0) {
        char* offsets = v.Data() + v_offset_start_;
        for (size_t i = 1; i < n_variable_; i++) {
            ::lgraph::_detail::UnalignedSet<DataOffset>(offsets + sizeof(DataOffset) * (i - 1),
                                                        static_cast<DataOffset>(min_size));
        }
    }
    return v;
}

Value Schema::CreateRecordWithLabelId() const {
    Value v(sizeof(LabelId));
    ::lgraph::_detail::UnalignedSet<LabelId>(v.Data(), label_id_);
    return v;
}

void Schema::AddDetachedVertexProperty(KvTransaction& txn, VertexId vid, const Value& property) {
    property_table_->AppendKv(
        txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid), property);
}

Value Schema::GetDetachedVertexProperty(KvTransaction& txn, VertexId vid) {
    return property_table_->GetValue(
        txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid));
}

void Schema::SetDetachedVertexProperty(KvTransaction& txn, VertexId vid, const Value& property) {
    auto ret = property_table_->SetValue(
        txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid), property);
    FMA_ASSERT(ret);
}

void Schema::DeleteDetachedVertexProperty(KvTransaction& txn, VertexId vid) {
    auto ret = property_table_->DeleteKey(
        txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid));
    FMA_ASSERT(ret);
}

Value Schema::GetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid) {
    return property_table_->GetValue(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid));
}

void Schema::SetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid,
                                     const Value& property) {
    auto ret = property_table_->SetValue(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), property);
    FMA_ASSERT(ret);
}

void Schema::AddDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid,
                                     const Value& property) {
    auto ret = property_table_->AddKV(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), property);
    FMA_ASSERT(ret);
}

void Schema::DeleteDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid) {
    auto ret = property_table_->DeleteKey(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid));
    FMA_ASSERT(ret);
}

// clear fields, other contents are kept untouched
void Schema::ClearFields() {
    label_.clear();
    fields_.clear();
    name_to_idx_.clear();
    n_fixed_ = 0;
    n_variable_ = 0;
    n_nullable_ = 0;
    v_offset_start_ = 0;
    indexed_fields_.clear();
    blob_fields_.clear();
    primary_field_.clear();
    edge_constraints_.clear();
    is_vertex_ = false;
}

/**
 * Sets the schema. Schema is constructed with empty contents.
 *
 * \param           fields  The field definitions.
 * \param [in,out]  errors  Error message. Each error will be APPENDED into
 * errors as a line separated by "\n".
 *
 * throws exception if there is error in the schema definition.
 */

void Schema::SetSchema(bool is_vertex, size_t n_fields, const FieldSpec* fields,
                       const std::string& primary, const std::string& temporal,
                       const TemporalFieldOrder& temporal_order,
                       const EdgeConstraints& edge_constraints) {
    lgraph::CheckValidFieldNum(n_fields);
    fields_.clear();
    name_to_idx_.clear();
    // assign id to fields, starting from fixed length types
    // then variable length types
    fields_.reserve(n_fields);
    for (size_t i = 0; i < n_fields; i++) {
        const FieldSpec& fs = fields[i];
        if (field_data_helper::IsFixedLengthFieldType(fs.type)) fields_.emplace_back(fs);
    }
    for (size_t i = 0; i < n_fields; i++) {
        const FieldSpec& fs = fields[i];
        if (!field_data_helper::IsFixedLengthFieldType(fs.type))
            fields_.push_back(_detail::FieldExtractor(fs));
    }
    is_vertex_ = is_vertex;
    primary_field_ = primary;
    temporal_field_ = temporal;
    temporal_order_ = temporal_order;
    edge_constraints_ = edge_constraints;
    RefreshLayout();
}

// del fields, assuming fields is already de-duplicated
void Schema::DelFields(const std::vector<std::string>& del_fields) {
    if (_F_UNLIKELY(del_fields.empty())) return;
    if (is_vertex_) {
        // vertex must has primary field, and can not be deleted
        FMA_ASSERT(!primary_field_.empty());
        auto ret = std::find_if(del_fields.begin(), del_fields.end(),
                                [this](auto& fd) { return this->primary_field_ == fd; });
        if (ret != del_fields.end()) {
            throw FieldCannotBeDeletedException(primary_field_);
        }
    } else {
        // if edge has temporal field, can not be deleted
        if (!temporal_field_.empty()) {
            auto ret = std::find_if(del_fields.begin(), del_fields.end(),
                                    [this](auto& fd) { return this->temporal_field_ == fd; });
            if (ret != del_fields.end()) {
                throw FieldCannotBeDeletedException(temporal_field_);
            }
        }
    }
    std::vector<size_t> del_ids = GetFieldIds(del_fields);
    std::sort(del_ids.begin(), del_ids.end());
    for (auto& id : del_ids) {
        UnVertexIndex(id);
        UnEdgeIndex(id);
    }
    del_ids.push_back(fields_.size());
    size_t put_pos = del_ids.front();
    for (size_t i = 0; i < del_ids.size() - 1; i++) {
        for (size_t get_pos = del_ids[i] + 1; get_pos < del_ids[i + 1]; get_pos++) {
            fields_[put_pos++] = std::move(fields_[get_pos]);
        }
    }
    fields_.erase(fields_.begin() + put_pos, fields_.end());
    RefreshLayout();
}

// add fields, assuming fields are already de-duplicated
void Schema::AddFields(const std::vector<FieldSpec>& add_fields) {
    using namespace import_v2;
    for (const auto& f : add_fields) {
        if (f.name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) ||
            f.name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) ||
            f.name == KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
            throw InputError(FMA_FMT(
                "Label[{}]: Property name cannot be \"SKIP\" or \"SRC_ID\" or \"DST_ID\"", label_));
        }
        if (_F_UNLIKELY(name_to_idx_.find(f.name) != name_to_idx_.end()))
            throw FieldAlreadyExistsException(f.name);
        fields_.push_back(_detail::FieldExtractor(f));
    }
    lgraph::CheckValidFieldNum(fields_.size());
    RefreshLayout();
}

// mod fields, assuming fields are already de-duplicated
void Schema::ModFields(const std::vector<FieldSpec>& mod_fields) {
    for (auto& f : mod_fields) {
        auto it = name_to_idx_.find(f.name);
        if (_F_UNLIKELY(it == name_to_idx_.end())) throw FieldNotFoundException(f.name);
        size_t fid = it->second;
        UnVertexIndex(fid);
        UnEdgeIndex(fid);
        auto& extractor = fields_[fid];
        extractor = _detail::FieldExtractor(f);
    }
    RefreshLayout();
}

std::vector<const FieldSpec*> Schema::GetFieldSpecPtrs() const {
    std::vector<const FieldSpec*> schema;
    schema.reserve(fields_.size());
    for (auto& f : fields_) {
        schema.push_back(&f.GetFieldSpec());
    }
    return schema;
}

std::vector<FieldSpec> Schema::GetFieldSpecs() const {
    std::vector<FieldSpec> schema;
    schema.reserve(fields_.size());
    for (auto& f : fields_) {
        schema.emplace_back(f.GetFieldSpec());
    }
    return schema;
}

std::map<std::string, FieldSpec> Schema::GetFieldSpecsAsMap() const {
    std::map<std::string, FieldSpec> ret;
    for (auto& kv : name_to_idx_) {
        ret.emplace_hint(ret.end(), std::make_pair(kv.first, fields_[kv.second].GetFieldSpec()));
    }
    return ret;
}

const _detail::FieldExtractor* Schema::GetFieldExtractor(size_t field_num) const {
    if (_F_UNLIKELY(field_num >= fields_.size())) throw FieldNotFoundException(field_num);
    return &fields_[field_num];
}

const _detail::FieldExtractor* Schema::TryGetFieldExtractor(size_t field_num) const {
    if (_F_UNLIKELY(field_num >= fields_.size())) return nullptr;
    return &fields_[field_num];
}

const _detail::FieldExtractor* Schema::GetFieldExtractor(const std::string& field_name) const {
    auto it = name_to_idx_.find(field_name);
    if (_F_UNLIKELY(it == name_to_idx_.end())) throw FieldNotFoundException(field_name);
    return &fields_[it->second];
}

const _detail::FieldExtractor* Schema::TryGetFieldExtractor(const std::string& field_name) const {
    auto it = name_to_idx_.find(field_name);
    if (_F_UNLIKELY(it == name_to_idx_.end())) return nullptr;
    return &fields_[it->second];
}

size_t Schema::GetFieldId(const std::string& name) const {
    auto fe = GetFieldExtractor(name);
    return fe->GetFieldId();
}

bool Schema::TryGetFieldId(const std::string& name, size_t& fid) const {
    auto it = name_to_idx_.find(name);
    if (it == name_to_idx_.end()) return false;
    fid = it->second;
    return true;
}

std::vector<size_t> Schema::GetFieldIds(const std::vector<std::string>& names) const {
    std::vector<size_t> ret;
    ret.reserve(names.size());
    for (auto& name : names) {
        ret.push_back(GetFieldId(name));
    }
    return ret;
}

std::string Schema::DumpRecord(const Value& record) const {
    std::string ret = "{";
    for (size_t i = 0; i < fields_.size(); i++) {
        auto& f = fields_[i];
        ret.append(f.Name()).append("=").append(f.FieldToString(record));
        if (i != fields_.size() - 1) ret.append(", ");
    }
    ret.append("}");
    return ret;
}

/**
 * Stores the schema into a Value
 *
 * \return  A Value.
 */
Value Schema::StoreSchema() {
    // This is an infrequent operation, so we just use BinaryBuffer to do it
    using namespace fma_common;
    BinaryBuffer buf;
    BinaryWrite(buf, *this);
    Value v;
    v.Copy(Value(buf.GetBuf(), buf.GetSize()));
    return v;
}

/**
 * Loads the schema from a serialized buffer given in data
 *
 * \param   data    The buffer storing the serialized schema.
 *
 * throws exception if there is error in the schema definition.
 */
void Schema::LoadSchema(const Value& data) {
    // This is an infrequent operation, so we just use BinaryBuffer to do it
    using namespace fma_common;
    BinaryBuffer buf((char*)data.Data(), data.Size());
    size_t r = BinaryRead(buf, *this);
    if (r != data.Size()) throw ::lgraph::InternalError("Failed to load schema from DB.");
}
}  // namespace lgraph
