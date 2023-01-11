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
#include "tools/json.hpp"
#include "lgraph/lgraph_types.h"

namespace lgraph_rfc {

using json = nlohmann::json;

static json FieldDataToJson(const lgraph_api::FieldData& data) {
    switch (data.type) {
    case lgraph_api::FieldType::NUL:
        {
            return json();
        }
    case lgraph_api::FieldType::BOOL:
        {
            return json(data.AsBool());
        }
    case lgraph_api::FieldType::INT8:
        {
            return json(data.AsInt8());
        }
    case lgraph_api::FieldType::INT16:
        {
            return json(data.AsInt16());
        }
    case lgraph_api::FieldType::INT32:
        {
            return json(data.AsInt32());
        }
    case lgraph_api::FieldType::INT64:
        {
            return json(data.AsInt64());
        }
    case lgraph_api::FieldType::FLOAT:
        {
            return json(data.AsFloat());
        }
    case lgraph_api::FieldType::DOUBLE:
        {
            return json(data.AsDouble());
        }
    case lgraph_api::FieldType::DATETIME:
        {
            return json(data.AsDateTime().ToString());
        }
    case lgraph_api::FieldType::DATE:
        {
            return json(data.AsDate().ToString());
        }
    case lgraph_api::FieldType::STRING:
        {
            try {
                return json::parse(data.AsString());
            } catch (...) {
                return json(data.AsString());
            }
        }
    case lgraph_api::FieldType::BLOB:
        {
            return json(data.AsBase64Blob());
        }
    default:
        abort();
    }
}

static lgraph_api::FieldData JsonToFieldData(const json& j_object) {
    if (j_object.is_boolean()) {
        return lgraph_api::FieldData(j_object.get<bool>());
    } else if (j_object.is_number_integer()) {
        return lgraph_api::FieldData(j_object.get<int>());
    } else if (j_object.is_number_float()) {
        return lgraph_api::FieldData(j_object.get<float>());
    } else if (j_object.is_string()) {
        return lgraph_api::FieldData(j_object.get<std::string>());
    } else {
        return lgraph_api::FieldData();
    }
}
}  // namespace lgraph_rfc
