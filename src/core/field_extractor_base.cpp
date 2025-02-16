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
#include "core/field_extractor_base.h"

namespace lgraph {

namespace _detail {

FieldExtractorBase::~FieldExtractorBase() = default;

void FieldExtractorBase::GetCopy(const Value& record, std::string& data) const {
    FMA_DBG_ASSERT(Type() != FieldType::BLOB);
    if (!DataInRecord(record)) {
        const Value v = GetDefaultValue();
        data.resize(v.Size());
        memcpy(&data[0], v.Data(), v.Size());
        return;
    }
    data.resize(GetDataSize(record));
    GetCopyRaw(record, &data[0], data.size());
}

void FieldExtractorBase::GetCopy(const Value& record, Value& data) const {
    if (!DataInRecord(record)) {
        data = GetDefaultValue();
        return;
    }
    data.Resize(GetDataSize(record));
    GetCopyRaw(record, data.Data(), data.Size());
}

Value FieldExtractorBase::GetConstRef(const Value& record) const {
    if (!DataInRecord(record)) {
        return GetDefaultValue();
    }
    if (GetIsNull(record)) return Value();
    return Value((char*)GetFieldPointer(record), GetDataSize(record));
}

std::string FieldExtractorBase::FieldToString(const Value& record) const {
    if (GetIsNull(record)) return "\"null\"";
    std::string ret;

#define COPY_FIELD_AND_RETURN_STR_(record, ft)                                       \
    do {                                                                              \
        typename field_data_helper::FieldType2StorageType<FieldType::ft>::type d = 0; \
        typedef typename field_data_helper::FieldType2CType<FieldType::ft>::type CT;  \
        GetCopy(record, d);                                                           \
        return fma_common::StringFormatter::Format("{}", static_cast<CT>(d));         \
    } while (0)

    switch (def_.type) {
    case FieldType::BOOL:
        COPY_FIELD_AND_RETURN_STR_(record, BOOL);
    case FieldType::INT8:
        COPY_FIELD_AND_RETURN_STR_(record, INT8);
    case FieldType::INT16:
        COPY_FIELD_AND_RETURN_STR_(record, INT16);
    case FieldType::INT32:
        COPY_FIELD_AND_RETURN_STR_(record, INT32);
    case FieldType::INT64:
        COPY_FIELD_AND_RETURN_STR_(record, INT64);
    case FieldType::FLOAT:
        COPY_FIELD_AND_RETURN_STR_(record, FLOAT);
    case FieldType::DOUBLE:
        COPY_FIELD_AND_RETURN_STR_(record, DOUBLE);
    case FieldType::DATE:
        {
            int32_t i;
            GetCopy(record, i);
            return Date(i).ToString();
        }
    case FieldType::DATETIME:
        {
            int64_t i;
            GetCopy(record, i);
            return DateTime(i).ToString();
        }
    case FieldType::STRING:
        {
            std::string ret(GetDataSize(record), 0);
            GetCopyRaw(record, &ret[0], ret.size());
            return ret;
        }
    case FieldType::BLOB:
        {
            return fma_common::StringFormatter::Format("[BLOB]");
        }
    case FieldType::POINT:
    case FieldType::LINESTRING:
    case FieldType::POLYGON:
    case FieldType::SPATIAL:
        {
            std::string ret(GetDataSize(record), 0);
            GetCopyRaw(record, &ret[0], ret.size());
            return ret;
        }
    case FieldType::FLOAT_VECTOR:
        {
            std::string vec_str;
            for (size_t i = 0; i < record.AsType<std::vector<float>>().size(); i++) {
                auto floatnum = record.AsType<std::vector<float>>().at(i);
                if (record.AsType<std::vector<float>>().at(i) > 999999) {
                    vec_str += std::to_string(floatnum).substr(0, 7);
                } else {
                    vec_str += std::to_string(floatnum).substr(0, 8);
                }
                vec_str += ',';
            }
            if (!vec_str.empty()) {
                vec_str.pop_back();
            }
            return vec_str;
        }
    case lgraph_api::NUL:
        break;
    }
    LOG_ERROR() << "Data type " << field_data_helper::FieldTypeName(def_.type) << " not handled";
    return "";
}

}  // namespace _detail
}  // namespace lgraph
