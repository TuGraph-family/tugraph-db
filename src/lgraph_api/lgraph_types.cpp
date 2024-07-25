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
#include "lgraph/lgraph_types.h"
#include "bolt/temporal.h"
#include "fma-common/utils.h"

namespace lgraph_api {

std::any FieldData::ToBolt() const {
    switch (type) {
    case FieldType::NUL:
        return {};
    case FieldType::BOOL:
        return data.boolean;
    case FieldType::INT8:
        return data.int8;
    case FieldType::INT16:
        return data.int16;
    case FieldType::INT32:
        return data.int32;
    case FieldType::INT64:
        return data.int64;
    case FieldType::FLOAT:
        {
            // bolt protocol does not have float type
            return fma_common::DoubleDecimalPlaces(data.sp, 5);
        }
    case FieldType::DOUBLE:
        return data.dp;
    case FieldType::STRING:
        return *data.buf;
    case FieldType::DATE:
        return bolt::Date{data.int32};
    case FieldType::DATETIME: {
            int64_t sec = data.int64 / 1000000;
            int64_t micro = data.int64 % 1000000;
            return bolt::LocalDateTime{sec, micro*1000};
        }
    default:
        throw std::runtime_error("ToBolt meet unsupported data type.");
    }
}
}  // namespace lgraph_api
