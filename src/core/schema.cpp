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
#include "core/vector_index.h"

namespace lgraph {
Schema::Schema(const Schema& rhs) {
    label_ = rhs.label_;
    label_id_ = rhs.label_id_;
    label_in_record_ = rhs.label_in_record_;
    deleted_ = rhs.deleted_;
    is_vertex_ = rhs.is_vertex_;
    fields_.resize(rhs.fields_.size());
    const int size = rhs.fields_.size();
    for (int i = 0; i < size; i++) {
        fields_[i] = rhs.fields_[i]->Clone();
    }
    name_to_idx_ = rhs.name_to_idx_;
    n_fixed_ = rhs.n_fixed_;
    n_variable_ = rhs.n_variable_;
    n_nullable_ = rhs.n_nullable_;
    v_offset_start_ = rhs.v_offset_start_;
    indexed_fields_ = rhs.indexed_fields_;
    blob_fields_ = rhs.blob_fields_;
    primary_field_ = rhs.primary_field_;
    temporal_field_ = rhs.temporal_field_;
    temporal_order_ = rhs.temporal_order_;
    edge_constraints_ = rhs.edge_constraints_;
    fulltext_fields_ = rhs.fulltext_fields_;
    edge_constraints_lids_ = rhs.edge_constraints_lids_;
    detach_property_ = rhs.detach_property_;
    fast_alter_schema = rhs.fast_alter_schema;
    property_table_ = rhs.property_table_;
    composite_index_map = rhs.composite_index_map;
    vector_index_fields_ = rhs.vector_index_fields_;
}

Schema& Schema::operator=(const Schema& rhs) {
    if (this == &rhs) return *this;
    label_ = rhs.label_;
    label_id_ = rhs.label_id_;
    label_in_record_ = rhs.label_in_record_;
    deleted_ = rhs.deleted_;
    is_vertex_ = rhs.is_vertex_;
    fields_.clear();
    fields_.resize(rhs.fields_.size());
    const int size = rhs.fields_.size();
    for (int i = 0; i <size; i++) {
        fields_[i] = rhs.fields_[i]->Clone();
    }
    name_to_idx_ = rhs.name_to_idx_;
    n_fixed_ = rhs.n_fixed_;
    n_variable_ = rhs.n_variable_;
    n_nullable_ = rhs.n_nullable_;
    v_offset_start_ = rhs.v_offset_start_;
    indexed_fields_ = rhs.indexed_fields_;
    blob_fields_ = rhs.blob_fields_;
    primary_field_ = rhs.primary_field_;
    temporal_field_ = rhs.temporal_field_;
    temporal_order_ = rhs.temporal_order_;
    edge_constraints_ = rhs.edge_constraints_;
    fulltext_fields_ = rhs.fulltext_fields_;
    edge_constraints_lids_ = rhs.edge_constraints_lids_;
    detach_property_ = rhs.detach_property_;
    fast_alter_schema = rhs.fast_alter_schema;
    property_table_ = rhs.property_table_;
    composite_index_map = rhs.composite_index_map;
    vector_index_fields_ = rhs.vector_index_fields_;
    return *this;
}

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
        auto prop = fe->GetConstRef(record);
        if (prop.Empty()) {
            continue;
        }
        if (fe->Type() != FieldType::FLOAT_VECTOR) {
            VertexIndex* index = fe->GetVertexIndex();
            FMA_ASSERT(index);
            // update field index
            if (!index->Delete(txn, prop, vid)) {
                THROW_CODE(InputError, "Failed to un-index vertex [{}] with field "
                                                    "value [{}:{}]: index value does not exist.",
                                                    vid, fe->Name(), fe->FieldToString(record));
            }
        }
    }
}

void Schema::DeleteVertexCompositeIndex(lgraph::KvTransaction& txn,
                                        lgraph::VertexId vid,
                                        const lgraph::Value& record) {
    for (const auto &kv : composite_index_map) {
        std::vector<std::string> ids;
        boost::split(ids, kv.first,
                     boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
        std::vector<std::string> fields;
        bool is_add_index = true;
        std::vector<Value> keys;
        for (int i = 0; i < (int)ids.size(); i++) {
            if (fields_[std::stoi(ids[i])]->GetIsNull(record)) {
                is_add_index = false;
                break;
            }
            keys.emplace_back(fields_[std::stoi(ids[i])]->GetConstRef(record));
        }
        if (!is_add_index) continue;
        auto composite_index = kv.second;
        if (!composite_index->Delete(txn,
                                  composite_index_helper::GenerateCompositeIndexKey(keys), vid)) {
            std::vector<std::string> field_names;
            std::vector<std::string> field_values;
            for (int i = 0; i < (int)ids.size(); i++) {
                field_names.push_back(fields_[std::stoi(ids[i])]->Name());
                field_values.push_back(fields_[std::stoi(ids[i])]->FieldToString(record));
            }
            THROW_CODE(InputError,
                       "Failed to index vertex [{}] with field value {}:{}: "
                       "index value already exists.",
                       vid, "[" + boost::join(field_names, ",") + "]",
                       "[" + boost::join(field_values, ",") + "]");
        }
    }
}

void Schema::DeleteCreatedVertexIndex(KvTransaction& txn, VertexId vid, const Value& record,
                                      const std::vector<size_t>& created) {
    for (auto& idx : created) {
        auto& fe = fields_[idx];
        auto prop = fe->GetConstRef(record);
        if (prop.Empty()) {
            continue;
        }
        VertexIndex* index = fe->GetVertexIndex();
        FMA_ASSERT(index);
        // the aim of this method is delete the index that has been created
        if (!index->Delete(txn, prop, vid)) {
            THROW_CODE(InputError, "Failed to un-index vertex [{}] with field "
                                                    "value [{}:{}]: index value does not exist.",
                                                    vid, fe->Name(), fe->FieldToString(record));
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
        if (fe->GetIsNull(record)) continue;
        entry.kvs.emplace_back(fe->Name(), fe->FieldToString(record));
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
        if (fe->GetIsNull(record)) continue;
        entry.kvs.emplace_back(fe->Name(), fe->FieldToString(record));
    }
    buffers.emplace_back(std::move(entry));
}

void Schema::AddVertexToIndex(KvTransaction& txn, VertexId vid, const Value& record,
                              std::vector<size_t>& created) {
    created.reserve(fields_.size());
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        auto prop = fe->GetConstRef(record);
        if (prop.Empty()) {
            continue;
        }
        if (fe->Type() != FieldType::FLOAT_VECTOR) {
            VertexIndex* index = fe->GetVertexIndex();
            FMA_ASSERT(index);
            // update field index
            if (!index->Add(txn, prop, vid)) {
                THROW_CODE(InputError,
                "Failed to index vertex [{}] with field value [{}:{}]: index value already exists.",
                vid, fe->Name(), fe->FieldToString(record));
            }
        }
        created.push_back(idx);
    }
}

void Schema::AddVertexToCompositeIndex(lgraph::KvTransaction& txn, lgraph::VertexId vid,
                                       const lgraph::Value& record,
                                       std::vector<std::string>& created) {
    created.reserve(composite_index_map.size());
    for (const auto &kv : composite_index_map) {
        std::vector<std::string> ids;
        boost::split(ids, kv.first, boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
        std::vector<std::string> fields;
        bool is_add_index = true;
        std::vector<Value> keys;
        for (int i = 0; i < (int)ids.size(); i++) {
            if (fields_[std::stoi(ids[i])]->GetIsNull(record)) {
                is_add_index = false;
                break;
            }
            keys.emplace_back(fields_[std::stoi(ids[i])]->GetConstRef(record));
        }
        if (!is_add_index) continue;
        auto composite_index = kv.second;
        if (!composite_index->Add(txn,
             composite_index_helper::GenerateCompositeIndexKey(keys), vid)) {
            std::vector<std::string> field_names;
            std::vector<std::string> field_values;
            for (int i = 0; i < (int)ids.size(); i++) {
                field_names.push_back(fields_[std::stoi(ids[i])]->Name());
                field_values.push_back(fields_[std::stoi(ids[i])]->FieldToString(record));
            }
            THROW_CODE(InputError,
                       "Failed to index vertex [{}] with field value {}:{}: "
                       "index value already exists.",
                       vid, "[" + boost::join(field_names, ",") + "]",
                       "[" + boost::join(field_values, ",") + "]");
        }
        created.push_back(kv.first);
    }
}

std::vector<std::vector<std::string>> Schema::GetRelationalCompositeIndexKey(
    const std::vector<size_t>& fields) {
    std::vector<std::vector<std::string>> result;
    std::unordered_set<std::string> visited;
    for (const auto &expected_id : fields) {
        for (const auto &kv : composite_index_map) {
            std::vector<std::string> field_ids;
            boost::split(field_ids, kv.first,
                         boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
            bool flag = false;
            for (const auto &id : field_ids) {
                if ((int)expected_id == std::stoi(id)) {
                    flag = true;
                    break;
                }
            }
            if (flag && !visited.count(kv.first)) {
                std::vector<std::string> field_names;
                for (const auto& id : field_ids) {
                        field_names.push_back(fields_[std::stoi(id)]->Name());
                }
                result.push_back(field_names);
                visited.insert(kv.first);
            }
        }
    }
    return result;
}

bool Schema::VertexUniqueIndexConflict(KvTransaction& txn, const Value& record) {
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        VertexIndex* index = fe->GetVertexIndex();
        FMA_ASSERT(index);
        if (!index->IsUnique()) continue;
        if (fe->GetIsNull(record)) continue;
        if (index->UniqueIndexConflict(txn, fe->GetConstRef(record))) {
            return true;
        }
    }
    return false;
}

void Schema::DeleteEdgeIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record) {
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        if (fe->GetIsNull(record)) continue;
        EdgeIndex* index = fe->GetEdgeIndex();
        FMA_ASSERT(index);
        // update field index
        if (!index->Delete(txn, fe->GetConstRef(record), euid)) {
            THROW_CODE(InputError,
                       "Failed to un-index edge with field "
                       "value [{}:{}]: index value does not exist.",
                       fe->Name(), fe->FieldToString(record));
        }
    }
}

void Schema::DeleteCreatedEdgeIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record,
                                    const std::vector<size_t>& created) {
    for (auto& idx : created) {
        auto& fe = fields_[idx];
        if (fe->GetIsNull(record)) continue;
        EdgeIndex* index = fe->GetEdgeIndex();
        FMA_ASSERT(index);
        // the aim of this method is delete the index that has been created
        if (!index->Delete(txn, fe->GetConstRef(record), euid)) {
            THROW_CODE(InputError,
                       "Failed to un-index edge with field "
                       "value [{}:{}]: index value does not exist.",
                       fe->Name(), fe->FieldToString(record));
        }
    }
}

void Schema::AddEdgeToIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record,
                            std::vector<size_t>& created) {
    created.reserve(fields_.size());
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        if (fe->GetIsNull(record)) continue;
        EdgeIndex* index = fe->GetEdgeIndex();
        FMA_ASSERT(index);
        // update field index
        if (!index->Add(txn, fe->GetConstRef(record), euid)) {
            THROW_CODE(InputError,
                       "Failed to index edge with field value [{}:{}]: index value already exists.",
                       fe->Name(), fe->FieldToString(record));
        }
        created.push_back(idx);
    }
}

void Schema::AddVectorToVectorIndex(KvTransaction& txn, VertexId vid, const Value& record) {
    for (auto& idx : vector_index_fields_) {
        auto& fe = fields_[idx];
        if (fe->GetIsNull(record)) continue;
        VectorIndex* index = fe->GetVectorIndex();
        if (index->GetIndexType() == "ivf_flat") return;
        auto dim = index->GetVecDimension();
        std::vector<std::vector<float>> floatvector;
        std::vector<int64_t> vids;
        floatvector.push_back(fe->GetConstRef(record).AsType<std::vector<float>>());
        vids.push_back(vid);
        if (floatvector.back().size() != (size_t)dim) {
            THROW_CODE(InputError, "vector index dimension mismatch, vector size:{}, dim:{}",
                       floatvector.back().size(), dim);
        }
        index->Add(floatvector, vids);
    }
}

void Schema::DeleteVectorIndex(KvTransaction& txn, VertexId vid, const Value& record) {
    for (auto& idx : vector_index_fields_) {
        auto& fe = fields_[idx];
        if (fe->GetIsNull(record)) continue;
        VectorIndex* index = fe->GetVectorIndex();
        if (index->GetIndexType() == "ivf_flat") return;
        index->Remove({vid});
    }
}

FieldData Schema::GetFieldDataFromField(const _detail::FieldExtractorBase* extractor,
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
    case FieldType::STRING:
        return FieldData(extractor->GetConstRef(record).AsString());
    case FieldType::BLOB:
        LOG_ERROR() << "BLOB cannot be obtained directly, use GetFieldDataFromField(Value, "
                     "Extractor, GetBlobKeyFunc)";
    case FieldType::POINT:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(PointWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(PointCartesian(EWKB));
            default:
                THROW_CODE(InputError, "invalid srid!\n");
        }
    }

    case FieldType::LINESTRING:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(LineStringWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(LineStringCartesian(EWKB));
            default:
                THROW_CODE(InputError, "invalid srid!\n");
        }
    }

    case FieldType::POLYGON:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(PolygonWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(PolygonCartesian(EWKB));
            default:
                THROW_CODE(InputError, "invalid srid!\n");
        }
    }

    case FieldType::SPATIAL:
    {
        std::string EWKB = extractor->GetConstRef(record).AsString();
        lgraph_api::SRID srid = lgraph_api::ExtractSRID(EWKB);
        switch (srid) {
            case lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "invalid srid!\n");
            case lgraph_api::SRID::WGS84:
                return FieldData(SpatialWgs84(EWKB));
            case lgraph_api::SRID::CARTESIAN:
                return FieldData(SpatialCartesian(EWKB));
            default:
                THROW_CODE(InputError, "invalid srid!\n");
        }
    }
    case FieldType::FLOAT_VECTOR:
    {
        return FieldData((extractor->GetConstRef(record)).AsType<std::vector<float>>());
    }
    case FieldType::NUL:
        LOG_ERROR() << "FieldType NUL";
    }
    return FieldData();
}

void Schema::ParseAndSet(Value& record, const FieldData& data,
                         _detail::FieldExtractorBase* extractor) const {
    if (!fast_alter_schema) {
        GetFieldExtractorV1(extractor)->ParseAndSet(record, data);
        return;
    }
    FieldId count = GetFieldExtractorV2(extractor)->GetRecordCount(record);
    if (!extractor->DataInRecord(record)) {
        Value new_prop = CreateEmptyRecord();
        for (const auto& field : name_to_idx_) {
            _detail::FieldExtractorV2* extr = GetFieldExtractorV2(GetFieldExtractor(field.first));
            extr->SetIsNull(new_prop, extr->GetIsNull(record));
            if (extr->IsFixedType()) {
                if (extr->GetFieldId() >= count && extr->HasDefaultValue()) {
                    if (extr->GetDefaultFieldData() == FieldData()) {
                        extr->SetIsNull(new_prop, true);
                        continue;
                    }
                    SetFixedSizeValue(new_prop,
                                      field_data_helper::FieldDataToValueOfFieldType(
                                          extr->GetDefaultFieldData(), extr->Type()),
                                      extr);
                    extr->SetIsNull(new_prop, false);
                } else if (extr->GetFieldId() < count) {
                    if (extr->GetIsNull(record)) {
                        extr->SetIsNull(new_prop, true);
                        continue;
                    }
                    SetFixedSizeValue(new_prop, extr->GetConstRef(record), extr);
                    extr->SetIsNull(new_prop, false);
                }
            } else {
                if (extr->GetFieldId() >= count && extr->HasDefaultValue()) {
                    if (extr->GetDefaultFieldData() == FieldData()) {
                        extr->SetIsNull(new_prop, true);
                        continue;
                    }
                    _SetVariableLengthValue(new_prop,
                                            field_data_helper::FieldDataToValueOfFieldType(
                                                extr->GetDefaultFieldData(), extr->Type()),
                                            extr);
                    extr->SetIsNull(new_prop, false);
                } else if (extr->GetFieldId() < count) {
                    if (extr->GetIsNull(record)) {
                        extr->SetIsNull(new_prop, true);
                        continue;
                    }
                    _SetVariableLengthValue(new_prop, extr->GetConstRef(record), extr);
                    extr->SetIsNull(new_prop, false);
                }
            }
        }
        record = new_prop;
    }

    bool data_is_null = data.type == FieldType::NUL;
    extractor->SetIsNull(record, data_is_null);
    if (data_is_null) return;

#define _SET_FIXED_TYPE_VALUE_FROM_FD(ft)                                                   \
    do {                                                                                    \
        if (data.type == extractor->Type()) {                                               \
            return SetFixedSizeValue(                                                       \
                record, field_data_helper::GetStoredValue<FieldType::ft>(data), extractor); \
        } else {                                                                            \
            typename field_data_helper::FieldType2StorageType<FieldType::ft>::type s;       \
            if (!field_data_helper::FieldDataTypeConvert<FieldType::ft>::Convert(data, s))  \
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());  \
            return SetFixedSizeValue(record, s, extractor);                                 \
        }                                                                                   \
    } while (0)

    switch (extractor->Type()) {
    case FieldType::BOOL:
        _SET_FIXED_TYPE_VALUE_FROM_FD(BOOL);
    case FieldType::INT8:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT8);
    case FieldType::INT16:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT16);
    case FieldType::INT32:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT32);
    case FieldType::INT64:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT64);
    case FieldType::DATE:
        _SET_FIXED_TYPE_VALUE_FROM_FD(DATE);
    case FieldType::DATETIME:
        _SET_FIXED_TYPE_VALUE_FROM_FD(DATETIME);
    case FieldType::FLOAT:
        _SET_FIXED_TYPE_VALUE_FROM_FD(FLOAT);
    case FieldType::DOUBLE:
        _SET_FIXED_TYPE_VALUE_FROM_FD(DOUBLE);

    case FieldType::STRING:
        if (data.type != FieldType::STRING)
            throw ParseIncompatibleTypeException(extractor->Name(), data.type, FieldType::STRING);
        return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
    case FieldType::BLOB:
        {
            // used in AlterLabel, when copying old blob value to new
            // In this case, the value must already be correctly formatted, so just copy it
            if (data.type != FieldType::BLOB)
                throw ParseIncompatibleTypeException(extractor->Name(), data.type, FieldType::BLOB);
            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
        }
    case FieldType::POINT:
        {
            // point type can only be converted from point and string;
            if (data.type != FieldType::POINT && data.type != FieldType::STRING)
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());
            FMA_DBG_ASSERT(extractor->IsFixedType());
            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, ::lgraph_api::SpatialType::POINT))
                throw ParseStringException(extractor->Name(), *data.data.buf, FieldType::POINT);

            record.Resize(record.Size());
            char* ptr =
                (char*)record.Data() + extractor->GetFieldOffset(record);
            memcpy(ptr, (*data.data.buf).data(), 50);
            return;
        }
    case FieldType::LINESTRING:
        {
            if (data.type != FieldType::LINESTRING && data.type != FieldType::STRING)
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());
            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, ::lgraph_api::SpatialType::LINESTRING))
                throw ParseStringException(extractor->Name(), *data.data.buf,
                                           FieldType::LINESTRING);

            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
        }
    case FieldType::POLYGON:
        {
            if (data.type != FieldType::POLYGON && data.type != FieldType::STRING)
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());
            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, ::lgraph_api::SpatialType::POLYGON))
                throw ParseStringException(extractor->Name(), *data.data.buf, FieldType::POLYGON);

            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
        }
    case FieldType::SPATIAL:
        {
            if (data.type != FieldType::SPATIAL && data.type != FieldType::STRING)
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());
            ::lgraph_api::SpatialType s;

            // throw ParseStringException in this function;
            try {
                s = ::lgraph_api::ExtractType(*data.data.buf);
            } catch (...) {
                throw ParseStringException(extractor->Name(), *data.data.buf, FieldType::SPATIAL);
            }

            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, s))
                throw ParseStringException(extractor->Name(), *data.data.buf, FieldType::SPATIAL);

            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
        }
    case FieldType::FLOAT_VECTOR:
        {
            if (data.type != FieldType::FLOAT_VECTOR)
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());

            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.vp), extractor);
        }
    default:
        LOG_ERROR() << "Data type " << field_data_helper::FieldTypeName(extractor->Type())
                    << " not handled";
    }
}

template <FieldType FT>
void Schema::_ParseStringAndSet(Value& record, const std::string& data,
                                ::lgraph::_detail::FieldExtractorBase* extractor) const {
    typedef typename field_data_helper::FieldType2CType<FT>::type CT;
    typedef typename field_data_helper::FieldType2StorageType<FT>::type ST;
    CT s{};
    size_t tmp = fma_common::TextParserUtils::ParseT<CT>(data.data(), data.data() + data.size(), s);
    if (_F_UNLIKELY(tmp != data.size())) throw ParseStringException(extractor->Name(), data, FT);
    return SetFixedSizeValue(record, static_cast<ST>(s), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::STRING>(
    Value& record, const std::string& data,
    ::lgraph::_detail::FieldExtractorBase* extractor) const {
    return _SetVariableLengthValue(record, Value::ConstRef(data), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::POINT>(
    Value& record, const std::string& data,
    ::lgraph::_detail::FieldExtractorBase* extractor) const {
    // check whether the point data is valid;
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::POINT))
        throw ParseStringException(extractor->Name(), data, FieldType::POINT);
    // FMA_DBG_CHECK_EQ(sizeof(data), field_data_helper::FieldTypeSize(def_.type));
    size_t Size = record.Size();
    record.Resize(Size);
    char* ptr = (char*)record.Data() + extractor->GetFieldOffset(record);
    memcpy(ptr, data.data(), 50);
}

template <>
void Schema::_ParseStringAndSet<FieldType::LINESTRING>(
    Value& record, const std::string& data,
    ::lgraph::_detail::FieldExtractorBase* extractor) const {
    // check whether the linestring data is valid;
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::LINESTRING))
        throw ParseStringException(extractor->Name(), data, FieldType::LINESTRING);
    return _SetVariableLengthValue(record, Value::ConstRef(data), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::POLYGON>(
    Value& record, const std::string& data,
    ::lgraph::_detail::FieldExtractorBase* extractor) const {
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::POLYGON))
        throw ParseStringException(extractor->Name(), data, FieldType::POLYGON);
    return _SetVariableLengthValue(record, Value::ConstRef(data), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::SPATIAL>(
    Value& record, const std::string& data,
    ::lgraph::_detail::FieldExtractorBase* extractor) const {
    ::lgraph_api::SpatialType s;
    // throw ParseStringException in this function;
    try {
        s = ::lgraph_api::ExtractType(data);
    } catch (...) {
        throw ParseStringException(extractor->Name(), data, FieldType::SPATIAL);
    }

    if (!::lgraph_api::TryDecodeEWKB(data, s))
        throw ParseStringException(extractor->Name(), data, FieldType::SPATIAL);
    return _SetVariableLengthValue(record, Value::ConstRef(data), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::FLOAT_VECTOR>(
    Value& record, const std::string& data,
    ::lgraph::_detail::FieldExtractorBase* extractor) const {
    std::vector<float> vec;
    // check if there are only numbers and commas
    std::regex nonNumbersAndCommas("[^0-9,.]");
    if (std::regex_search(data, nonNumbersAndCommas)) {
        throw ParseStringException(extractor->Name(), data, FieldType::FLOAT_VECTOR);
    }
    // Check if the string conforms to the following format : 1.000000,2.000000,3.000000,...
    std::regex vector("^(?:[-+]?\\d*(?:\\.\\d+)?)(?:,[-+]?\\d*(?:\\.\\d+)?){1,}$");
    if (!std::regex_match(data, vector)) {
        throw ParseStringException(extractor->Name(), data, FieldType::FLOAT_VECTOR);
    }
    // check if there are 1.000,,2.000 & 1.000,2.000,
    if (data.front() == ',' || data.back() == ',' || data.find(",,") != std::string::npos) {
        throw ParseStringException(extractor->Name(), data, FieldType::FLOAT_VECTOR);
    }
    std::regex pattern("-?[0-9]+\\.?[0-9]*");
    std::sregex_iterator begin_it(data.begin(), data.end(), pattern), end_it;
    while (begin_it != end_it) {
        std::smatch match = *begin_it;
        vec.push_back(std::stof(match.str()));
        ++begin_it;
    }
    if (vec.size() <= 0)
        throw ParseStringException(extractor->Name(), data, FieldType::FLOAT_VECTOR);
    return _SetVariableLengthValue(record, Value::ConstRef(vec), extractor);
}

/**
 * Parse the string data and set the field
 *
 * \param [in,out]  record  The record.
 * \param           data    The string representation of the data.
 */
void Schema::ParseAndSet(Value& record, const std::string& data,
                         ::lgraph::_detail::FieldExtractorBase* extractor) const {
    if (!fast_alter_schema) {
        GetFieldExtractorV1(extractor)->ParseAndSet(record, data);
        return;
    }
    if (data.empty() &&
        (extractor->IsFixedType() || extractor->Type() == FieldType::LINESTRING ||
         extractor->Type() == FieldType::POLYGON || extractor->Type() == FieldType::SPATIAL ||
         extractor->Type() == FieldType::FLOAT_VECTOR)) {
        extractor->SetIsNull(record, true);
        return;
    }
    // empty string is treated as non-NULL
    extractor->SetIsNull(record, false);
    switch (extractor->Type()) {
    case FieldType::BOOL:
        return _ParseStringAndSet<FieldType::BOOL>(record, data, extractor);
    case FieldType::INT8:
        return _ParseStringAndSet<FieldType::INT8>(record, data, extractor);
    case FieldType::INT16:
        return _ParseStringAndSet<FieldType::INT16>(record, data, extractor);
    case FieldType::INT32:
        return _ParseStringAndSet<FieldType::INT32>(record, data, extractor);
    case FieldType::INT64:
        return _ParseStringAndSet<FieldType::INT64>(record, data, extractor);
    case FieldType::FLOAT:
        return _ParseStringAndSet<FieldType::FLOAT>(record, data, extractor);
    case FieldType::DOUBLE:
        return _ParseStringAndSet<FieldType::DOUBLE>(record, data, extractor);
    case FieldType::DATE:
        return _ParseStringAndSet<FieldType::DATE>(record, data, extractor);
    case FieldType::DATETIME:
        return _ParseStringAndSet<FieldType::DATETIME>(record, data, extractor);
    case FieldType::STRING:
        return _ParseStringAndSet<FieldType::STRING>(record, data, extractor);
    case FieldType::BLOB:
        LOG_ERROR() << "ParseAndSet(Value, std::string) is not supposed to"
                       " be called directly. We should first parse blobs "
                       "into BlobValue and use SetBlobField(Value, FieldData)";
    case FieldType::POINT:
        return _ParseStringAndSet<FieldType::POINT>(record, data, extractor);
    case FieldType::LINESTRING:
        return _ParseStringAndSet<FieldType::LINESTRING>(record, data, extractor);
    case FieldType::POLYGON:
        return _ParseStringAndSet<FieldType::POLYGON>(record, data, extractor);
    case FieldType::SPATIAL:
        return _ParseStringAndSet<FieldType::SPATIAL>(record, data, extractor);
    case FieldType::FLOAT_VECTOR:
        return _ParseStringAndSet<FieldType::FLOAT_VECTOR>(record, data, extractor);
    case FieldType::NUL:
        LOG_ERROR() << "NUL FieldType";
    }
    LOG_ERROR() << "Data type " << field_data_helper::FieldTypeName(extractor->Type())
                << " not handled";
}

/**
 * Sets the value of the variable field in record. Valid only for variable-length fields.
 *
 * \param   record  The record.
 * \param   data    Value to be set.
 * \param   extr  The field extractor pointer.
 */
void Schema::_SetVariableLengthValue(Value& record, const Value& data,
                                     ::lgraph::_detail::FieldExtractorBase* extractor) const {
    _detail::FieldExtractorV2* extr = dynamic_cast<_detail::FieldExtractorV2*>(extractor);
    FMA_DBG_ASSERT(!extractor->IsFixedType());
    if (data.Size() > _detail::MAX_STRING_SIZE)
        throw DataSizeTooLargeException(extr->Name(), data.Size(), _detail::MAX_STRING_SIZE);
    size_t foff = extr->GetFieldOffset(record);
    char* rptr = (char*)record.Data();
    size_t variable_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(rptr + foff);
    size_t fsize = extr->GetDataSize(record);

    // realloc record with original size to make sure we own the memory
    record.Resize(record.Size());

    // move data to the correct position
    int32_t diff = data.Size() - fsize;
    if (diff > 0) {
        if (record.Size() + diff > _detail::MAX_PROP_SIZE) {
            throw RecordSizeLimitExceededException(extractor->Name(), record.Size() +diff,
                                       _detail::MAX_PROP_SIZE);
        }
        record.Resize(record.Size() + diff);
        rptr = (char*)record.Data();
        memmove(rptr + variable_offset + sizeof(DataOffset) + data.Size(),
                rptr + variable_offset + sizeof(DataOffset) + fsize,
                record.Size() - (variable_offset + sizeof(DataOffset) + data.Size()));
    } else {
        memmove(rptr + variable_offset + sizeof(DataOffset) + data.Size(),
                rptr + variable_offset + sizeof(DataOffset) + fsize,
                record.Size() - (variable_offset + sizeof(DataOffset) + fsize));
        record.Resize(record.Size() + diff);
    }

    // set data
    rptr = (char*)record.Data();
    // set data size
    ::lgraph::_detail::UnalignedSet<uint32_t>(rptr + variable_offset, data.Size());
    // set data value
    memcpy(rptr + variable_offset + sizeof(uint32_t), data.Data(), data.Size());

    // update offset of other veriable fields
    size_t count = extr->GetRecordCount(record);
    // adjust offset of other fields
    for (size_t i = extr->GetFieldId() + 1; i < count; i++) {
        if (fields_[i]->IsFixedType()) continue;
        if (fields_[i]->IsDeleted()) continue;
        size_t offset = extr->GetFieldOffset(record, i);
        size_t var_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(rptr + offset);
        ::lgraph::_detail::UnalignedSet<DataOffset>(rptr + offset, var_offset + diff);
    }
}

void Schema::CopyFieldsRaw(Value& dst, const std::vector<size_t> fids_in_dst,
                           const Schema* src_schema, const Value& src,
                           const std::vector<size_t> fids_in_src) {
    FMA_DBG_ASSERT(fids_in_dst.size() == fids_in_src.size());
    dst.Resize(dst.Size());
    for (size_t i = 0; i < fids_in_dst.size(); i++) {
        const _detail::FieldExtractorV1* dst_fe =
            GetFieldExtractorV1(GetFieldExtractor(fids_in_dst[i]));
        const _detail::FieldExtractorV1* src_fe =
            GetFieldExtractorV1(src_schema->GetFieldExtractor(fids_in_src[i]));
        dst_fe->CopyDataRaw(dst, src, src_fe);
    }
}

void Schema::SetFixedSizeValue(Value& record, const Value& data,
                               ::lgraph::_detail::FieldExtractorV2* extractor) const {
#define _SET_FIXED_FIELD(ft)                                                       \
    do {                                                                           \
        typename field_data_helper::FieldType2StorageType<FieldType::ft>::type sd; \
        extractor->ConvertData(&sd, data.Data(), sizeof(sd));                      \
        memcpy(ptr, &sd, sizeof(sd));                                              \
    } while (0)
    FMA_DBG_ASSERT(extractor->IsFixedType());
    auto* ptr = static_cast<char*>(extractor->GetFieldPointer(record));
    if (data.Size() == extractor->TypeSize()) {
        memcpy(ptr, data.Data(), data.Size());
    } else {
        switch (extractor->Type()) {
        case FieldType::INT8:
            _SET_FIXED_FIELD(INT8);
            break;
        case FieldType::INT16:
            _SET_FIXED_FIELD(INT16);
            break;
        case FieldType::INT32:
            _SET_FIXED_FIELD(INT32);
            break;
        case FieldType::INT64:
            _SET_FIXED_FIELD(INT64);
            break;
        case FieldType::FLOAT:
            _SET_FIXED_FIELD(FLOAT);
            break;
        case FieldType::DOUBLE:
            _SET_FIXED_FIELD(DOUBLE);
            break;
        default:
            LOG_ERROR() << "Error here";
        }
    }
}

void Schema::RefreshLayout() {
    if (fast_alter_schema) {
        RefreshLayoutForFastSchema();
        return;
    }
    // check field types
    // check if there is any blob
    blob_fields_.clear();
    for (size_t i = 0; i < fields_.size(); i++) {
        auto& f = fields_[i];
        if (f->Type() == FieldType::NUL) throw FieldCannotBeNullTypeException(f->Name());
        if (f->Type() == FieldType::BLOB) blob_fields_.push_back(i);
    }
    // if label is included in record, data starts after LabelId
    size_t data_start_off = label_in_record_ ? sizeof(LabelId) : 0;
    // setup name_to_fields
    name_to_idx_.clear();
    for (size_t i = 0; i < fields_.size(); i++) {
        auto f = (_detail::FieldExtractorV1*)fields_[i].get();
        f->SetFieldId(i);
        f->SetNullableArrayOff(data_start_off);
        if (_F_UNLIKELY(name_to_idx_.find(f->Name()) != name_to_idx_.end()))
            throw FieldAlreadyExistsException(f->Name());
        name_to_idx_[f->Name()] = i;
    }
    // layout nullable array
    n_nullable_ = 0;
    for (auto& f : fields_) {
        if (f->IsOptional()) {
            GetFieldExtractorV1(f.get())->SetNullableOff(n_nullable_);
            n_nullable_++;
        }
    }
    v_offset_start_ = data_start_off + (n_nullable_ + 7) / 8;
    // layout the fixed fields
    n_fixed_ = 0;
    n_variable_ = 0;
    for (auto& f : fields_) {
        if (field_data_helper::IsFixedLengthFieldType(f->Type())) {
            n_fixed_++;
            (static_cast<_detail::FieldExtractorV1*>(f.get()))->SetFixedLayoutInfo(v_offset_start_);
            v_offset_start_ += f->TypeSize();
        } else {
            n_variable_++;
        }
    }
    // now, layout the variable fields
    size_t vidx = 0;
    for (auto& f : fields_) {
        if (!field_data_helper::IsFixedLengthFieldType(f->Type()))
            (static_cast<_detail::FieldExtractorV1*>(f.get()))
                ->SetVLayoutInfo(v_offset_start_, n_variable_, vidx++);
    }
    // finally, check the indexed fields
    indexed_fields_.clear();
    bool found_primary = false;
    for (auto& f : fields_) {
        if (!f->GetVertexIndex() && !f->GetEdgeIndex()) continue;
        indexed_fields_.emplace_hint(indexed_fields_.end(), f->GetFieldId());
        if (f->Name() == primary_field_) {
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
        if (!f->FullTextIndexed()) continue;
        fulltext_fields_.emplace(f->GetFieldId());
    }
}

void Schema::RefreshLayoutForFastSchema() {
    FMA_ASSERT(fast_alter_schema);
    blob_fields_.clear();
    name_to_idx_.clear();
    for (size_t i = 0; i < fields_.size(); i++) {
        auto f = static_cast<_detail::FieldExtractorV2*>(fields_[i].get());
        if (f->IsDeleted()) continue;
        f->SetLabelInRecord(label_in_record_);
        if (f->Type() == FieldType::NUL) throw FieldCannotBeNullTypeException(f->Name());
        if (f->Type() == FieldType::BLOB) blob_fields_.push_back(i);
        if (_F_UNLIKELY(name_to_idx_.find(f->Name()) != name_to_idx_.end()))
            throw FieldAlreadyExistsException(f->Name());
        name_to_idx_[f->Name()] = i;
    }

    indexed_fields_.clear();
    bool found_primary = false;
    for (auto& f : fields_) {
        if (!f->GetVertexIndex() && !f->GetEdgeIndex()) continue;
        indexed_fields_.emplace_hint(indexed_fields_.end(), f->GetFieldId());
        if (f->Name() == primary_field_) {
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
        if (!f->FullTextIndexed()) continue;
        fulltext_fields_.emplace(f->GetFieldId());
    }
}

/**
 * Creates an empty record
 *
 * \param           size_hint   (Optional) Hint of size of the record, used to
 * reduce memory realloc.
 *
 * \return  A Value.
 */
Value Schema::CreateEmptyRecord(size_t size_hint) const {
    Value v(size_hint);
    if (!fast_alter_schema) {
        size_t min_size = v_offset_start_;
        if (n_variable_ > 0) min_size += sizeof(DataOffset) * (n_variable_ - 1);
        v.Resize(min_size);
        // first data is the LabelId
        if (label_in_record_) {
            ::lgraph::_detail::UnalignedSet<LabelId>(v.Data(), label_id_);
            // nullable bits
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
    } else {
        size_t num_fields = fields_.size();
        // [label] - count - null_array - offset_array
        size_t min_size =  (label_in_record_ ? sizeof(LabelId) : 0) +
                          sizeof(FieldId) + (num_fields + 7) / 8;
        // Fixed-value and Variable-value. Variable-value will store an offset at Fixed-value area
        // and assume the length of every variable value is 0;
        for (const auto& field : fields_) {
            min_size += sizeof(DataOffset);
            if (!field->IsDeleted()) {
                min_size += field->IsFixedType() ? field->TypeSize()
                                                 : (sizeof(DataOffset) + sizeof(uint32_t));
            }
        }

        v.Resize(min_size);

        char* ptr = v.Data();
        DataOffset offset = 0;

        // 1. Set label id.
        if (label_in_record_) {
            ::lgraph::_detail::UnalignedSet<LabelId>(ptr + offset, label_id_);
            offset += sizeof(LabelId);
        }

        // 2. Set fields count.
        ::lgraph::_detail::UnalignedSet<FieldId>(ptr + offset, static_cast<FieldId>(num_fields));
        offset += sizeof(FieldId);

        // 3. Set nullable array
        memset(ptr + offset, 0xFF, (num_fields + 7) / 8);
        offset += (num_fields + 7) / 8;

        if (num_fields == 0) return v;

        // 4. Set fields' offset.
        DataOffset offset_begin = offset;
        DataOffset data_offset = offset + num_fields * sizeof(DataOffset);  // data area begin.
        char* offset_ptr = ptr + offset_begin;                              // offset area begin.

        // field0 do not need to store its offset.
        for (size_t i = 1; i < num_fields; i++) {
            data_offset += fields_[i - 1]->IsDeleted()     ? 0
                           : fields_[i - 1]->IsFixedType() ? fields_[i - 1]->TypeSize()
                                                           : sizeof(DataOffset);
            ::lgraph::_detail::UnalignedSet<DataOffset>(offset_ptr, data_offset);
            offset_ptr += sizeof(DataOffset);
        }

        // the latest offset marks the end of the fixed-area.
        data_offset += fields_[num_fields - 1]->IsFixedType() ? fields_[num_fields - 1]->TypeSize()
                                                              : sizeof(DataOffset);
        ::lgraph::_detail::UnalignedSet<DataOffset>(offset_ptr, data_offset);

        // 5. Set variable fields offset. They are stored at fixed-area, and their sizes are all
        // zero.
        for (const auto& field : fields_) {
            if (!field->IsFixedType()) {
                if (field->IsDeleted()) continue;
                DataOffset var_offset = 0;  // variable fields offset.
                if (field->GetFieldId() == 0) {
                    var_offset = offset + num_fields * sizeof(DataOffset);
                } else {
                    var_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(
                        ptr + offset_begin + (field->GetFieldId() - 1) * sizeof(DataOffset));
                }

                ::lgraph::_detail::UnalignedSet<DataOffset>(ptr + var_offset, data_offset);
                ::lgraph::_detail::UnalignedSet<DataOffset>(ptr + data_offset, 0);
                data_offset += sizeof(DataOffset);
            }
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
    Value ret;
    bool found = property_table_->GetValue(
        txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid), ret);
    if (!found) {
        THROW_CODE(InternalError, "Get: vid {} is not found in the detached property table.", vid);
    }
    return ret;
}

void Schema::SetDetachedVertexProperty(KvTransaction& txn, VertexId vid, const Value& property) {
    auto ret = property_table_->SetValue(
        txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid), property);
    if (!ret) {
        THROW_CODE(InternalError, "Set: vid {} is not found in the detached property table.", vid);
    }
}

void Schema::DeleteDetachedVertexProperty(KvTransaction& txn, VertexId vid) {
    auto ret = property_table_->DeleteKey(
        txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid));
    if (!ret) {
        THROW_CODE(InternalError, "Delete: vid {} is not found in the detached property table.",
                   vid);
    }
}

Value Schema::GetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid) {
    Value ret;
    bool found = property_table_->GetValue(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), ret);
    if (!found) {
        THROW_CODE(InternalError, "Get: euid {} is not found in the detached property table.", eid);
    }
    return ret;
}

void Schema::SetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid,
                                     const Value& property) {
    auto ret = property_table_->SetValue(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), property);
    if (!ret) {
        THROW_CODE(InternalError, "Set: euid {} is not found in the detached property table.",
                   eid.ToString());
    }
}

void Schema::AddDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid,
                                     const Value& property) {
    auto ret = property_table_->AddKV(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), property);
    if (!ret) {
        THROW_CODE(InternalError, "Add: euid {} is found in the detached property table.",
                   eid.ToString());
    }
}

void Schema::DeleteDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid) {
    auto ret = property_table_->DeleteKey(
        txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid));
    if (!ret) {
        THROW_CODE(InternalError, "Delete: euid {} is not found in the detached property table.",
                   eid.ToString());
    }
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
    if (!fast_alter_schema) {
        for (size_t i = 0; i < n_fields; i++) {
            const FieldSpec& fs = fields[i];
            if (field_data_helper::IsFixedLengthFieldType(fs.type))
                fields_.push_back(std::make_shared<_detail::FieldExtractorV1>(fs));
        }
        for (size_t i = 0; i < n_fields; i++) {
            const FieldSpec& fs = fields[i];
            if (!field_data_helper::IsFixedLengthFieldType(fs.type))
                fields_.push_back(std::make_shared<_detail::FieldExtractorV1>(fs));
        }
    } else {
        for (size_t i = 0; i < n_fields; i++) {
            fields_.push_back(std::make_shared<_detail::FieldExtractorV2>(fields[i], i));
        }
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

    auto composite_index_key = GetRelationalCompositeIndexKey(del_ids);
    for (const auto &k : composite_index_key) {
        UnVertexCompositeIndex(k);
    }

    if (fast_alter_schema) {
        for (size_t del_id : del_ids) {
            fields_[del_id]->MarkDeleted();
        }
    } else {
        del_ids.push_back(fields_.size());
        size_t put_pos = del_ids.front();
        for (size_t i = 0; i < del_ids.size() - 1; i++) {
            for (size_t get_pos = del_ids[i] + 1; get_pos < del_ids[i + 1]; get_pos++) {
                fields_[put_pos++] = std::move(fields_[get_pos]);
            }
        }
        fields_.erase(fields_.begin() + put_pos, fields_.end());
    }
    RefreshLayout();
}

// add fields, assuming fields are already de-duplicated
void Schema::AddFields(const std::vector<FieldSpec>& add_fields) {
    using namespace import_v2;
    for (const auto& f : add_fields) {
        if (f.name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SKIP) ||
            f.name == KeyWordFunc::GetStrFromKeyWord(KeyWord::SRC_ID) ||
            f.name == KeyWordFunc::GetStrFromKeyWord(KeyWord::DST_ID)) {
            THROW_CODE(InputError,
                       "Label[{}]: Property name cannot be \"SKIP\" or \"SRC_ID\" or \"DST_ID\"",
                       label_);
        }
        if (_F_UNLIKELY(name_to_idx_.find(f.name) != name_to_idx_.end()))
            throw FieldAlreadyExistsException(f.name);
        if (fast_alter_schema) {
            fields_.push_back(
                std::make_shared<_detail::FieldExtractorV2>(FieldSpec(f), fields_.size()));
        } else {
            fields_.push_back(std::make_shared<_detail::FieldExtractorV1>(FieldSpec(f)));
        }
    }
    lgraph::CheckValidFieldNum(fields_.size());
    RefreshLayout();
}

// mod fields, assuming fields are already de-duplicated
void Schema::ModFields(const std::vector<FieldSpec>& mod_fields) {
    std::vector<size_t> mod_ids;
    for (auto& f : mod_fields) {
        auto it = name_to_idx_.find(f.name);
        if (_F_UNLIKELY(it == name_to_idx_.end())) throw FieldNotFoundException(f.name);
        size_t fid = it->second;
        UnVertexIndex(fid);
        UnEdgeIndex(fid);
        if (fast_alter_schema) {
            auto& extractor = fields_[fid];
            extractor.reset();
            extractor = std::make_shared<_detail::FieldExtractorV2>(f);
            extractor->SetFieldId(fid);
        } else {
            auto& extractor = fields_[fid];
            extractor.reset();
            extractor = std::make_shared<_detail::FieldExtractorV1>(f);
        }
        mod_ids.push_back(fid);
    }
    auto composite_index_key = GetRelationalCompositeIndexKey(mod_ids);
    for (const auto &k : composite_index_key) {
        UnVertexCompositeIndex(k);
    }
    RefreshLayout();
}

std::vector<const FieldSpec*> Schema::GetFieldSpecPtrs() const {
    std::vector<const FieldSpec*> schema;
    schema.reserve(fields_.size());
    for (auto& f : fields_) {
        schema.push_back(&f->GetFieldSpec());
    }
    return schema;
}

std::vector<FieldSpec> Schema::GetFieldSpecs() const {
    std::vector<FieldSpec> schema;
    schema.reserve(fields_.size());
    for (auto& f : fields_) {
        schema.emplace_back(f->GetFieldSpec());
    }
    return schema;
}

std::vector<FieldSpec> Schema::GetAliveFieldSpecs() const {
    std::vector<FieldSpec> schema;
    for (auto& f : fields_) {
        if (f->IsDeleted()) {
            continue;
        }
        schema.emplace_back(f->GetFieldSpec());
    }
    return schema;
}

std::map<std::string, FieldSpec> Schema::GetAliveFieldSpecsAsMap() const {
    std::map<std::string, FieldSpec> ret;
    // for FieldExtractorV1, sizeof(name_to_idx_) == sizeof(fields_)
    // for FieldExtractorV2, sizeof(name_to_idx_) <= sizeof(fields_)
    for (auto& kv : name_to_idx_) {
        ret.emplace_hint(ret.end(), std::make_pair(kv.first, fields_[kv.second]->GetFieldSpec()));
    }
    return ret;
}

std::map<std::string, FieldSpec> Schema::GetFieldSpecsAsMap() const {
    std::map<std::string, FieldSpec> ret;
    for (auto& field : fields_) {
        ret.emplace_hint(ret.end(), std::make_pair(field->Name(), field->GetFieldSpec()));
    }
    return ret;
}

std::map<std::string, FieldSpec> Schema::GetFiledSpecsAsMapIgnoreFieldId() const {
    std::map<std::string, FieldSpec> ret;
    for (auto& field : fields_) {
        FieldSpec field_spec = field->GetFieldSpec();
        field_spec.id = 0;
        ret.emplace_hint(ret.end(), std::make_pair(field_spec.name, field_spec));
    }
    return ret;
}

_detail::FieldExtractorBase* Schema::GetFieldExtractor(size_t field_num) const {
    if (_F_UNLIKELY(field_num >= fields_.size())) throw FieldNotFoundException(field_num);
    return fields_[field_num].get();
}

_detail::FieldExtractorBase* Schema::TryGetFieldExtractor(size_t field_num) const {
    if (_F_UNLIKELY(field_num >= fields_.size())) return nullptr;
    return fields_[field_num].get();
}

_detail::FieldExtractorBase* Schema::GetFieldExtractor(const std::string& field_name) const {
    auto it = name_to_idx_.find(field_name);
    if (_F_UNLIKELY(it == name_to_idx_.end())) throw FieldNotFoundException(field_name);
    return fields_[it->second].get();
}

_detail::FieldExtractorBase* Schema::TryGetFieldExtractor(const std::string& field_name) const {
    auto it = name_to_idx_.find(field_name);
    if (_F_UNLIKELY(it == name_to_idx_.end())) return nullptr;
    return fields_[it->second].get();
}

std::vector<CompositeIndexSpec> Schema::GetCompositeIndexSpec() const {
    std::vector<CompositeIndexSpec> compositeIndexSpecList;
    for (const auto &kv : composite_index_map) {
        std::vector<std::string> ids;
        boost::split(ids, kv.first, boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
        std::vector<std::string> fields;
        for (int i = 0; i < (int)ids.size(); i++) {
            fields.emplace_back(this->fields_[std::stoi(ids[i])]->Name());
        }
        compositeIndexSpecList.push_back({label_, fields, kv.second->type_});
    }
    return compositeIndexSpecList;
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
        ret.append(f->Name()).append("=").append(f->FieldToString(record));
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
    if (r != data.Size()) THROW_CODE(InternalError, "Failed to load schema from DB.");
}
}  // namespace lgraph
