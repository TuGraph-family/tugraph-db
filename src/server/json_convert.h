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
#include "core/data_type.h"

namespace lgraph_rfc {

using json = nlohmann::json;

[[maybe_unused]]
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
    case lgraph_api::FieldType::VECTOR:
        {
            return json(data.AsVector());
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
            std::string str = data.AsString();
            json j = nlohmann::json::parse(str, nullptr, false);
            if (j.is_array() || j.is_object()) {
                return j;
            } else {
                return json(str);
            }
        }
    case lgraph_api::FieldType::BLOB:
        {
            return json(data.AsBase64Blob());
        }
    case lgraph_api::FieldType::POINT:
        {
            ::lgraph_api::SRID s = data.GetSRID();
            switch (s) {
                case ::lgraph_api::SRID::WGS84:
                    return json(data.AsWgsPoint().ToString());
                case ::lgraph_api::SRID::CARTESIAN:
                    return json(data.AsCartesianPoint().ToString());
                default:
                    throw lgraph::InputError("unsupported spatial srid");
            }
        }
    case lgraph_api::FieldType::LINESTRING:
        {
            ::lgraph_api::SRID s = data.GetSRID();
            switch (s) {
                case ::lgraph_api::SRID::WGS84:
                    return json(data.AsWgsLineString().ToString());
                case ::lgraph_api::SRID::CARTESIAN:
                    return json(data.AsCartesianLineString().ToString());
                default:
                    throw lgraph::InputError("unsupported spatial srid");
            }
        }
    case lgraph_api::FieldType::POLYGON:
        {
            ::lgraph_api::SRID s = data.GetSRID();
            switch (s) {
                case ::lgraph_api::SRID::WGS84:
                    return json(data.AsWgsPolygon().ToString());
                case ::lgraph_api::SRID::CARTESIAN:
                    return json(data.AsCartesianPolygon().ToString());
                default:
                    throw lgraph::InputError("unsupported spatial srid");
            }
        }
    case lgraph_api::FieldType::SPATIAL:
        {
            ::lgraph_api::SRID s = data.GetSRID();
            switch (s) {
                case ::lgraph_api::SRID::WGS84:
                    return json(data.AsWgsSpatial().ToString());
                case ::lgraph_api::SRID::CARTESIAN:
                    return json(data.AsCartesianSpatial().ToString());
                default:
                    throw lgraph::InputError("unsupported spatial srid");
            }
        }
    default:
        throw lgraph::InputError(fma_common::StringFormatter::Format(
            "FieldDataToJson: unsupported field type: {}", data.type));
    }
}

[[maybe_unused]]
static lgraph_api::FieldData JsonToFieldData(const json& j_object) {
    if (j_object.is_boolean()) {
        return lgraph_api::FieldData(j_object.get<bool>());
    } else if (j_object.is_number_integer()) {
        return lgraph_api::FieldData(j_object.get<int64_t>());
    } else if (j_object.is_number_float()) {
        return lgraph_api::FieldData(j_object.get<float>());
    } else if (j_object.is_string()) {
        return lgraph_api::FieldData(j_object.get<std::string>());
    } else if (j_object.is_null()) {
        return lgraph_api::FieldData();
    } else if (!j_object.is_discarded()) {
        return lgraph_api::FieldData(j_object.dump());
    } else {
        throw lgraph::InputError("JsonToFieldData: unsupported json");
    }
}
}  // namespace lgraph_rfc
