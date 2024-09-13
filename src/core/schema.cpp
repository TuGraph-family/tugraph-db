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
        if (fe.Type() != FieldType::FLOAT_VECTOR) {
            VertexIndex* index = fe.GetVertexIndex();
            FMA_ASSERT(index);
            // update field index
            if (!index->Delete(txn, fe.GetConstRef(record), vid)) {
                THROW_CODE(InputError, "Failed to un-index vertex [{}] with field "
                                                    "value [{}:{}]: index value does not exist.",
                                                    vid, fe.Name(), fe.FieldToString(record));
            }
        }
    }
}

void Schema::DeleteVertexCompositeIndex(lgraph::KvTransaction& txn, lgraph::VertexId vid,
                                        const lgraph::Value& record) {
    for (const auto& kv : composite_index_map) {
        std::vector<std::string> ids;
        boost::split(ids, kv.first, boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
        std::vector<std::string> fields;
        bool is_add_index = true;
        std::vector<Value> keys;
        for (int i = 0; i < (int)ids.size(); i++) {
            if (fields_[std::stoi(ids[i])].GetIsNull(record)) {
                is_add_index = false;
                break;
            }
            keys.emplace_back(fields_[std::stoi(ids[i])].GetConstRef(record));
        }
        if (!is_add_index) continue;
        auto composite_index = kv.second;
        if (!composite_index->Delete(txn, composite_index_helper::GenerateCompositeIndexKey(keys),
                                     vid)) {
            std::vector<std::string> field_names;
            std::vector<std::string> field_values;
            for (int i = 0; i < (int)ids.size(); i++) {
                field_names.push_back(fields_[std::stoi(ids[i])].Name());
                field_values.push_back(fields_[std::stoi(ids[i])].FieldToString(record));
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
        if (fe.GetIsNull(record)) continue;
        VertexIndex* index = fe.GetVertexIndex();
        FMA_ASSERT(index);
        // the aim of this method is delete the index that has been created
        if (!index->Delete(txn, fe.GetConstRef(record), vid)) {
            THROW_CODE(InputError,
                       "Failed to un-index vertex [{}] with field "
                       "value [{}:{}]: index value does not exist.",
                       vid, fe.Name(), fe.FieldToString(record));
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
        if (fe.Type() != FieldType::FLOAT_VECTOR) {
            VertexIndex* index = fe.GetVertexIndex();
            FMA_ASSERT(index);
            // update field index
            if (!index->Add(txn, fe.GetConstRef(record), vid)) {
                THROW_CODE(InputError,
                "Failed to index vertex [{}] with field value [{}:{}]: index value already exists.",
                vid, fe.Name(), fe.FieldToString(record));
            }
        }
        created.push_back(idx);
    }
}

void Schema::AddVertexToCompositeIndex(lgraph::KvTransaction& txn, lgraph::VertexId vid,
                                       const lgraph::Value& record,
                                       std::vector<std::string>& created) {
    created.reserve(composite_index_map.size());
    for (const auto& kv : composite_index_map) {
        std::vector<std::string> ids;
        boost::split(ids, kv.first, boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
        std::vector<std::string> fields;
        bool is_add_index = true;
        std::vector<Value> keys;
        for (int i = 0; i < (int)ids.size(); i++) {
            if (fields_[std::stoi(ids[i])].GetIsNull(record)) {
                is_add_index = false;
                break;
            }
            keys.emplace_back(fields_[std::stoi(ids[i])].GetConstRef(record));
        }
        if (!is_add_index) continue;
        auto composite_index = kv.second;
        if (!composite_index->Add(txn, composite_index_helper::GenerateCompositeIndexKey(keys),
                                  vid)) {
            std::vector<std::string> field_names;
            std::vector<std::string> field_values;
            for (int i = 0; i < (int)ids.size(); i++) {
                field_names.push_back(fields_[std::stoi(ids[i])].Name());
                field_values.push_back(fields_[std::stoi(ids[i])].FieldToString(record));
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
    for (const auto& expected_id : fields) {
        for (const auto& kv : composite_index_map) {
            std::vector<std::string> field_ids;
            boost::split(field_ids, kv.first,
                         boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
            bool flag = false;
            for (const auto& id : field_ids) {
                if ((int)expected_id == std::stoi(id)) {
                    flag = true;
                    break;
                }
            }
            if (flag && !visited.count(kv.first)) {
                std::vector<std::string> field_names;
                for (const auto& id : field_ids) {
                    field_names.push_back(fields_[std::stoi(id)].Name());
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
        VertexIndex* index = fe.GetVertexIndex();
        FMA_ASSERT(index);
        if (!index->IsUnique()) continue;
        if (fe.GetIsNull(record)) continue;
        if (index->UniqueIndexConflict(txn, fe.GetConstRef(record))) {
            return true;
        }
    }
    return false;
}

void Schema::DeleteEdgeIndex(KvTransaction& txn, const EdgeUid& euid, const Value& record) {
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        EdgeIndex* index = fe.GetEdgeIndex();
        FMA_ASSERT(index);
        // update field index
        if (!index->Delete(txn, fe.GetConstRef(record), euid)) {
            THROW_CODE(InputError,
                       "Failed to un-index edge with field "
                       "value [{}:{}]: index value does not exist.",
                       fe.Name(), fe.FieldToString(record));
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
            THROW_CODE(InputError,
                       "Failed to un-index edge with field "
                       "value [{}:{}]: index value does not exist.",
                       fe.Name(), fe.FieldToString(record));
        }
    }
}

bool Schema::EdgeUniqueIndexConflict(KvTransaction& txn, const Value& record) {
    for (auto& idx : indexed_fields_) {
        auto& fe = fields_[idx];
        EdgeIndex* index = fe.GetEdgeIndex();
        FMA_ASSERT(index);
        if (!index->IsUnique()) continue;
        if (fe.GetIsNull(record)) continue;
        if (index->UniqueIndexConflict(txn, fe.GetConstRef(record))) {
            return true;
        }
    }
    return false;
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
            THROW_CODE(InputError,
                       "Failed to index edge with field value [{}:{}]: index value already exists.",
                       fe.Name(), fe.FieldToString(record));
        }
        created.push_back(idx);
    }
}

void Schema::AddVectorToVectorIndex(KvTransaction& txn, VertexId vid, const Value& record) {
    for (auto& idx : vector_index_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        VectorIndex* index = fe.GetVectorIndex();
        auto dim = index->GetVecDimension();
        std::vector<std::vector<float>> floatvector;
        std::vector<int64_t> vids;
        floatvector.push_back(fe.GetConstRef(record).AsType<std::vector<float>>());
        vids.push_back(vid);
        if (floatvector.back().size() != (size_t)dim) {
            THROW_CODE(InputError,
                       "vector index dimension mismatch, vector size:{}, dim:{}",
                       floatvector.back().size(), dim);
        }
        index->Add(floatvector, vids, 1);
    }
}

void Schema::DeleteVectorIndex(KvTransaction& txn, VertexId vid, const Value& record) {
    for (auto& idx : vector_index_fields_) {
        auto& fe = fields_[idx];
        if (fe.GetIsNull(record)) continue;
        VectorIndex* index = fe.GetVectorIndex();
        std::vector<int64_t> vids;
        vids.push_back(vid);
        index->Add({}, vids, 0);
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

/**
 * Creates an empty record
 *
 * \param [in,out]  v           Value to store the result.
 * \param           size_hint   (Optional) Hint of size of the record, used to
 * reduce memory realloc.
 */
Value Schema::CreateEmptyRecord(size_t size_hint) const {
    Value v(size_hint);
    size_t min_size = ::lgraph::_detail::NULL_ARRAY_OFFSET + fields_.size() + 7 / 8 +
                      fields_.size() * sizeof(DataOffset);
    for (size_t i = 0; i < fields_.size(); i++) {
        if (!fields_[i].IsFixedType()) {
            min_size += sizeof(DataOffset);
        } else {
            min_size += fields_[i].TypeSize();
        }
    }
    v.Resize(min_size);

    // first data is Version
    ::lgraph::_detail::UnalignedSet<VersionId>(v.Data(), ::lgraph::_detail::SCHEMA_VERSION);
    // next data is label id
    if (label_in_record_) {
        ::lgraph::_detail::UnalignedSet<LabelId>(v.Data() + ::lgraph::_detail::LABEL_OFFSET,
                                                 label_id_);
    }

    // set Property Count
    ::lgraph::_detail::UnalignedSet<ProCount>(v.Data() + ::lgraph::_detail::COUNT_OFFSET,
                                              static_cast<ProCount>(fields_.size()));

    // nullbable bits
    memset(v.Data() + ::lgraph::_detail::NULL_ARRAY_OFFSET, 0xFF, (fields_.size() + 7) / 8);

    // initialize offsets
    DataOffset data_offset = ::lgraph::_detail::NULL_ARRAY_OFFSET + (fields_.size() + 7) / 8 +
                             fields_.size() * sizeof(DataOffset);
    DataOffset offset_begin = ::lgraph::_detail::NULL_ARRAY_OFFSET + (fields_.size() + 7) / 8;

    size_t num_fields = fields_.size();
    if (num_fields > 1) {
        char* data_ptr = v.Data() + offset_begin;
        for (size_t i = 1; i < num_fields; i++) {
            ::lgraph::_detail::UnalignedSet<DataOffset>(data_ptr, data_offset);
            data_ptr += sizeof(DataOffset);
            data_offset += fields_[i].IsFixedType() ? fields_[i].TypeSize() : sizeof(DataOffset);
        }
    }
    ::lgraph::_detail::UnalignedSet<DataOffset>(
        v.Data() + offset_begin + sizeof(DataOffset) * fields_.size(), data_offset);
    return v;
}

// parse data from FieldData and set field
// for BLOBs, only formatted data is allowed
// The reason for moving parseandset from FieldExtractor to Schema is
// Due to the current data layout, updating a Field may require obtaining the types of other Fields.
// Solely relying on Field Extractor lacks the information of other Fields.

void Schema::ParseAndSet(Value& record, const FieldData& data,
                         const _detail::FieldExtractor* extractor) const {
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
        _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
    case FieldType::BLOB:
        {
            // used in AlterLabel, when copying old blob value to new
            // In this case, the value must already be correctly formatted, so just copy it
            if (data.type != FieldType::BLOB)
                throw ParseIncompatibleTypeException(extractor->Name(), data.type, FieldType::BLOB);
            _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
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
                (char*)record.Data() + extractor->GetFieldOffset(record, extractor->GetFieldId());
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

            _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
        }
    case FieldType::POLYGON:
        {
            if (data.type != FieldType::POLYGON && data.type != FieldType::STRING)
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());
            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, ::lgraph_api::SpatialType::POLYGON))
                throw ParseStringException(extractor->Name(), *data.data.buf, FieldType::POLYGON);

            _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
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

            _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf), extractor);
        }
    case FieldType::FLOAT_VECTOR:
        {
            if (data.type != FieldType::FLOAT_VECTOR)
                throw ParseFieldDataException(extractor->Name(), data, extractor->Type());

            _SetVariableLengthValue(record, Value::ConstRef(*data.data.vp), extractor);
        }
    default:
        LOG_ERROR() << "Data type " << field_data_helper::FieldTypeName(extractor->Type())
                    << " not handled";
    }
}

/**
 * Sets the value of the field in record. Valid only for fixed-length fields.
 *
 * \param   record  The record.
 * \param   data    Value to be set.
 * \param   extractor  The field extractor pointer.
 */
ENABLE_IF_FIXED_FIELD(T, void)
Schema::SetFixedSizeValue(Value& record, const T& data,
                          const ::lgraph::_detail::FieldExtractor* extractor) const {
    // "Cannot call SetField(Value&, const T&) on a variable length field";
    FMA_DBG_ASSERT(!extractor->is_vfield_);
    // "Type size mismatch"
    FMA_DBG_CHECK_EQ(sizeof(data), extractor->TypeSize());
    // copy the buffer so we don't accidentally overwrite memory
    int data_size = extractor->GetDataSize(record);
    size_t offset = extractor->GetFieldOffset(record, extractor->GetFieldId());
    char* ptr = (char*)record.Data();
    if (_F_LIKELY(data_size == sizeof(data))) {
        record.Resize(record.Size());
        char* ptr = ptr + offset;
        ::lgraph::_detail::UnalignedSet<T>(ptr, data);
    } else {
        // If the data size differs, we need to resize the record:
        // 1. Move the data to the correct position.
        // 2. Modify the offset of the subsequent fields.

        // Move the data to the correct position.
        int diff = sizeof(data) - data_size;
        if (diff > 0) {
            record.Resize(record.Size() + diff);
            memmove(ptr + offset + sizeof(data), ptr + offset + data_size,
                    record.Size() - (offset + sizeof(data)));
        } else {
            memmove(ptr + offset + sizeof(data), ptr + offset + data_size,
                    record.Size() - (offset + data_size));
            record.Resize(record.Size() + diff);
        }
        ::lgraph::_detail::UnalignedSet<T>(ptr + offset, data);

        // Update the offset of the subsequent fields.
        for (ProCount i = extractor->GetFieldId() + 1; i < extractor->GetRecordCount(record) + 1;
             ++i) {
            size_t off = extractor->GetOffsetPosistion(record, i);
            size_t property_offset =
                ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + off);
            ::lgraph::_detail::UnalignedSet<DataOffset>(ptr + off, property_offset + diff);
        }

        // Update the offset of veriable length fields.
        for (ProCount i = extractor->GetRecordCount(record) + 1;
             i < extractor->GetRecordCount(record); i++) {
            if (fields_[i].IsFixedType()) continue;
            size_t off = extractor->GetFieldOffset(record, i);
            size_t property_offset =
                ::lgraph::_detail::UnalignedGet<DataOffset>(record.Data() + off);
            ::lgraph::_detail::UnalignedSet<DataOffset>(ptr + off, property_offset + diff);
        }
    }
}

/**
 * Sets the value of the variable field in record. Valid only for variable-length fields.
 *
 * \param   record  The record.
 * \param   data    Value to be set.
 * \param   extractor  The field extractor pointer.
 */
void Schema::_SetVariableLengthValue(Value& record, const Value& data,
                                     const ::lgraph::_detail::FieldExtractor* extractor) const {
    FMA_DBG_ASSERT(extractor->is_vfield_);
    if (data.Size() > _detail::MAX_STRING_SIZE)
        throw DataSizeTooLargeException(extractor->Name(), data.Size(), _detail::MAX_STRING_SIZE);
    size_t foff = extractor->GetFieldOffset(record, extractor->GetFieldId());
    char* rptr = (char*)record.Data();
    size_t variable_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(rptr + foff);
    size_t fsize = extractor->GetDataSize(record);

    // realloc record with original size to make sure we own the memory
    record.Resize(record.Size());

    // move data to the correct position
    int32_t diff = data.Size() + sizeof(uint32_t) - fsize;
    if (diff > 0) {
        record.Resize(record.Size() + diff);
        memmove(rptr + variable_offset + sizeof(data), rptr + variable_offset + fsize,
                record.Size() - (variable_offset + sizeof(data)));
    } else {
        memmove(rptr + variable_offset + sizeof(data), rptr + variable_offset + fsize,
                record.Size() - (variable_offset + fsize));
        record.Resize(record.Size() + diff);
    }

    // set data
    rptr = (char*)record.Data();
    // set data size
    ::lgraph::_detail::UnalignedSet<uint32_t>(rptr + variable_offset, data.Size());
    // set data value
    memcpy(rptr + variable_offset + sizeof(uint32_t), data.Data(), data.Size());

    // update offset of other veriable fields
    size_t count = extractor->GetRecordCount(record);
    // adjust offset of other fields
    for (size_t i = extractor->GetFieldId(); i < count; i++) {
        if (fields_[i].IsFixedType()) continue;
        size_t offset = extractor->GetFieldOffset(record, i);
        size_t var_offset = ::lgraph::_detail::UnalignedGet<DataOffset>(rptr + offset);
        ::lgraph::_detail::UnalignedSet<DataOffset>(rptr + offset, var_offset + diff);
    }
}
/**
 * Parse string data as type and set the field
 *
 * \tparam  T   Type into which the data will be parsed.
 * \param [in,out]  record  The record.
 * \param           data    The string representation of the data. If it is
 * NBytes or String, then the data is stored as-is.
 *
 * \return  ErrorCode::OK if succeeds
 *          FIELD_PARSE_FAILED.
 */
template <FieldType FT>
void Schema::_ParseStringAndSet(Value& record, const std::string& data,
                                const ::lgraph::_detail::FieldExtractor* extractor) const {
    typedef typename field_data_helper::FieldType2CType<FT>::type CT;
    typedef typename field_data_helper::FieldType2StorageType<FT>::type ST;
    CT s{};
    size_t tmp = fma_common::TextParserUtils::ParseT<CT>(data.data(), data.data() + data.size(), s);
    // error maybe there
    if (_F_UNLIKELY(tmp != data.size())) throw ParseStringException(extractor->Name(), data, FT);
    return SetFixedSizeValue(record, static_cast<ST>(s), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::STRING>(
    Value& record, const std::string& data,
    const ::lgraph::_detail::FieldExtractor* extractor) const {
    return _SetVariableLengthValue(record, Value::ConstRef(data), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::POINT>(
    Value& record, const std::string& data,
    const ::lgraph::_detail::FieldExtractor* extractor) const {
    // check whether the point data is valid;
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::POINT))
        throw ParseStringException(extractor->Name(), data, FieldType::POINT);
    // FMA_DBG_CHECK_EQ(sizeof(data), field_data_helper::FieldTypeSize(def_.type));
    size_t Size = record.Size();
    record.Resize(Size);
    char* ptr = (char*)record.Data() + extractor->GetFieldOffset(record, extractor->GetFieldId());
    memcpy(ptr, data.data(), 50);
}

template <>
void Schema::_ParseStringAndSet<FieldType::LINESTRING>(
    Value& record, const std::string& data,
    const ::lgraph::_detail::FieldExtractor* extractor) const {
    // check whether the linestring data is valid;
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::LINESTRING))
        throw ParseStringException(extractor->Name(), data, FieldType::LINESTRING);
    return _SetVariableLengthValue(record, Value::ConstRef(data), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::POLYGON>(
    Value& record, const std::string& data,
    const ::lgraph::_detail::FieldExtractor* extractor) const {
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::POLYGON))
        throw ParseStringException(extractor->Name(), data, FieldType::POLYGON);
    return _SetVariableLengthValue(record, Value::ConstRef(data), extractor);
}

template <>
void Schema::_ParseStringAndSet<FieldType::SPATIAL>(
    Value& record, const std::string& data,
    const ::lgraph::_detail::FieldExtractor* extractor) const {
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
    const ::lgraph::_detail::FieldExtractor* extractor) const {
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
                         const ::lgraph::_detail::FieldExtractor* extractor) const {
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

Value Schema::CreateRecordWithLabelId() const {
    Value v(sizeof(LabelId) + sizeof(VersionId));
    ::lgraph::_detail::UnalignedSet<VersionId>(v.Data(), ::lgraph::_detail::SCHEMA_VERSION);
    ::lgraph::_detail::UnalignedSet<LabelId>(v.Data() + ::lgraph::_detail::LABEL_OFFSET, label_id_);
    return v;
}

void Schema::AddDetachedVertexProperty(KvTransaction& txn, VertexId vid, const Value& property) {
    property_table_->AppendKv(txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid), property);
}

Value Schema::GetDetachedVertexProperty(KvTransaction& txn, VertexId vid) {
    Value ret;
    bool found =
        property_table_->GetValue(txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid), ret);
    if (!found) {
        THROW_CODE(InternalError, "Get: vid {} is not found in the detached property table.", vid);
    }
    return ret;
}

void Schema::SetDetachedVertexProperty(KvTransaction& txn, VertexId vid, const Value& property) {
    auto ret = property_table_->SetValue(txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid),
                                         property);
    if (!ret) {
        THROW_CODE(InternalError, "Set: vid {} is not found in the detached property table.", vid);
    }
}

void Schema::DeleteDetachedVertexProperty(KvTransaction& txn, VertexId vid) {
    auto ret = property_table_->DeleteKey(txn, graph::KeyPacker::CreateVertexPropertyTableKey(vid));
    if (!ret) {
        THROW_CODE(InternalError, "Delete: vid {} is not found in the detached property table.",
                   vid);
    }
}

Value Schema::GetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid) {
    Value ret;
    bool found =
        property_table_->GetValue(txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), ret);
    if (!found) {
        THROW_CODE(InternalError, "Get: euid {} is not found in the detached property table.", eid);
    }
    return ret;
}

void Schema::SetDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid,
                                     const Value& property) {
    auto ret =
        property_table_->SetValue(txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), property);
    if (!ret) {
        THROW_CODE(InternalError, "Set: euid {} is not found in the detached property table.",
                   eid.ToString());
    }
}

void Schema::AddDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid,
                                     const Value& property) {
    auto ret =
        property_table_->AddKV(txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid), property);
    if (!ret) {
        THROW_CODE(InternalError, "Add: euid {} is found in the detached property table.",
                   eid.ToString());
    }
}

void Schema::DeleteDetachedEdgeProperty(KvTransaction& txn, const EdgeUid& eid) {
    auto ret = property_table_->DeleteKey(txn, graph::KeyPacker::CreateEdgePropertyTableKey(eid));
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
    fields_.reserve(n_fields);
    for (size_t i = 0; i < n_fields; i++) {
        fields_.emplace_back(fields[i]);
    }
    std::sort(fields_.begin(), fields_.end(),
              [](const _detail::FieldExtractor& a, const _detail::FieldExtractor& b) {
                  return a.GetFieldId() < b.GetFieldId();
              });

    for (size_t i = 1; i < n_fields; i++) {
        if (fields_[i].GetFieldId() == fields_[i - 1].GetFieldId()) {
            throw FieldIdConflictException(fields_[i].Name(), fields_[i-1].Name());
        }
    }
    is_vertex_ = is_vertex;
    primary_field_ = primary;
    temporal_field_ = temporal;
    temporal_order_ = temporal_order;
    edge_constraints_ = edge_constraints;
}

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
    for (const auto& k : composite_index_key) {
        UnVertexCompositeIndex(k);
    }
    // just do logical delettion.
    for (size_t del_id : del_ids) {
        fields_[del_id].MarkDeleted();
    }
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
        fields_.push_back(_detail::FieldExtractor(f, fields_.size()));
    }
    lgraph::CheckValidFieldNum(fields_.size());
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
        auto& extractor = fields_[fid];
        extractor = _detail::FieldExtractor(f, fid);
        mod_ids.push_back(fid);
    }
    auto composite_index_key = GetRelationalCompositeIndexKey(mod_ids);
    for (const auto& k : composite_index_key) {
        UnVertexCompositeIndex(k);
    }
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

std::vector<CompositeIndexSpec> Schema::GetCompositeIndexSpec() const {
    std::vector<CompositeIndexSpec> compositeIndexSpecList;
    for (const auto& kv : composite_index_map) {
        std::vector<std::string> ids;
        boost::split(ids, kv.first, boost::is_any_of(_detail::COMPOSITE_INDEX_KEY_SEPARATOR));
        std::vector<std::string> fields;
        for (int i = 0; i < (int)ids.size(); i++) {
            fields.emplace_back(this->fields_[std::stoi(ids[i])].Name());
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
    if (r != data.Size()) THROW_CODE(InternalError, "Failed to load schema from DB.");
}
}  // namespace lgraph
