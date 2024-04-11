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
#include "core/data_type.h"
#include "core/value.h"

namespace lgraph {
namespace _detail {

#ifdef _USELESS_CODE
inline bool CheckFieldType(FieldData::DataType t1, FieldType t2) {
    switch (t2) {
    case FieldType::BOOL:
    case FieldType::INT8:
    case FieldType::INT16:
    case FieldType::INT32:
    case FieldType::INT64:
    case FieldType::DATE:
    case FieldType::DATETIME:
        return t1 == FieldData::INT;
    case FieldType::FLOAT:
    case FieldType::DOUBLE:
        return t1 == FieldData::REAL;
    case FieldType::STRING:
    case FieldType::BIN:
        return t1 == FieldData::STR;
    default:
        return false;
    }
}

inline bool ConstRefOfFieldData(const FieldData& d, FieldType dt, Value& v) {
    switch (dt) {
    case FieldType::BOOL:
    case FieldType::INT8:
    case FieldType::INT16:
    case FieldType::INT32:
    case FieldType::INT64:

        FMA_DBG_ASSERT(d.type == FieldData::INT || d.type == FieldData::REAL);
        return d.type == FieldData::INT ? CopyValue(dt, d.integer(), v)
                                        : CopyValue(dt, d.real(), v);
    case FieldType::DATE:
    case FieldType::DATETIME:
        FMA_DBG_ASSERT(d.type == FieldData::INT);
        return CopyValue(dt, d.integer(), v);
    case FieldType::FLOAT:
    case FieldType::DOUBLE:
        FMA_DBG_ASSERT(d.type == FieldData::REAL || d.type == FieldData::INT);
        return d.type == FieldData::REAL ? CopyValue(dt, d.real(), v)
                                         : CopyValue(dt, d.integer(), v);
    case FieldType::STRING:
    case FieldType::BIN:
        FMA_DBG_ASSERT(d.type == FieldData::STR);
        v.Copy(d.string());
        return true;
    default:
        THROW_CODE(InternalError,
                   "Failed to convert FieldData of type " + std::to_string((int)d.type) +
                            " into " + FieldTypeName(dt));
        return false;
    }
}

inline FieldData ValueToFieldData(const Value& v, FieldType dt) {
    FieldData dst;
    switch (dt) {
    case FieldType::BOOL:
        return FieldData(v.AsType<bool>());
    case FieldType::INT8:
        return FieldData::Int(v.AsType<int8_t>());
    case FieldType::INT16:
        return FieldData::Int(v.AsType<int16_t>());
    case FieldType::INT32:
        return FieldData::Int(v.AsType<int32_t>());
    case FieldType::INT64:
        return FieldData::Int(v.AsType<int64_t>());
    case FieldType::DATE:
        return FieldData::Int(v.AsType<int32_t>());
    case FieldType::DATETIME:
        return FieldData::Int(v.AsType<int64_t>());
    case FieldType::FLOAT:
        return FieldData::Real(v.AsType<float>());
    case FieldType::DOUBLE:
        return FieldData::Real(v.AsType<double>());
    case FieldType::STRING:
    case FieldType::BIN:
        return FieldData(v.AsString());
    default:
        FMA_ASSERT(false);
    }
    return dst;
}
#endif

}  // namespace _detail
}  // namespace lgraph
