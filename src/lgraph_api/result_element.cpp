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

#include "lgraph_api/result_element.h"
#include "lgraph/lgraph_result.h"
#include "server/json_convert.h"
#include "fma-common/string_formatter.h"

using json = nlohmann::json;

namespace lgraph_api {
namespace lgraph_result {

nlohmann::json Node::ToJson() {
    if (id == -1) {
        return json("__null__");
    }
    json result;
    std::map<std::string, json> j_properties;
    result["identity"] = id;
    if (!label.empty()) {
        result["label"] = label;
    }
    for (auto p : properties) {
        j_properties[p.first] = lgraph_rfc::FieldDataToJson(p.second);
    }
    if (!j_properties.empty()) {
        result["properties"] = j_properties;
    }
    return result;
}

nlohmann::json Relationship::ToJson() {
    json result;
    std::map<std::string, json> j_properties;
    result["identity"] = id;
    result["src"] = src;
    result["dst"] = dst;
    result["forward"] = forward;
    result["temporal_id"] = tid;
    if (!label.empty()) {
        result["label"] = label;
    }
    result["label_id"] = label_id;
    for (auto p : properties) {
        j_properties[p.first] = lgraph_rfc::FieldDataToJson(p.second);
    }
    if (!j_properties.empty()) {
        result["properties"] = j_properties;
    }
    return result;
}

PathElement::PathElement(const PathElement &value) {
    type_ = value.type_;
    if (type_ == LGraphType::NODE) {
        v.node = new lgraph_result::Node(*value.v.node);
    } else {
        v.repl = new Relationship(*value.v.repl);
    }
}

PathElement::PathElement(PathElement &&value) {
    type_ = value.type_;
    if (type_ == LGraphType::NODE) {
        v.node = value.v.node;
        value.v.node = nullptr;
    } else {
        v.repl = value.v.repl;
        value.v.repl = nullptr;
    }
}

PathElement &PathElement::operator=(const PathElement &value) {
    if (this == &value) return *this;
    type_ = value.type_;
    if (type_ == LGraphType::NODE) {
        delete v.node;
        v.node = new Node(*value.v.node);
    } else {
        delete v.repl;
        v.repl = new Relationship(*value.v.repl);
    }
    return *this;
}

PathElement &PathElement::operator=(PathElement &&value) {
    if (this == &value) return *this;
    type_ = value.type_;
    if (type_ == LGraphType::NODE) {
        v.node = value.v.node;
        value.v.node = nullptr;
    } else {
        v.repl = value.v.repl;
        value.v.repl = nullptr;
    }
    return *this;
}

nlohmann::json PathElement::ToJson() {
    if (type_ == LGraphType::NODE) {
        return v.node->ToJson();
    } else {
        return v.repl->ToJson();
    }
}

PathElement::~PathElement() {
    if (type_ == LGraphType::NODE) {
        delete v.node;
    } else {
        delete v.repl;
    }
}

}  // namespace lgraph_result

ResultElement::ResultElement(const ResultElement &value) {
    type_ = value.type_;
    switch (type_) {
    case LGraphType::INTEGER:
    case LGraphType::FLOAT:
    case LGraphType::DOUBLE:
    case LGraphType::BOOLEAN:
    case LGraphType::STRING:
    case LGraphType::ANY:
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case LGraphType::LIST:
        v.list = new std::vector<json>(*value.v.list);
        break;
    case LGraphType::MAP:
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case LGraphType::NODE:
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case LGraphType::RELATIONSHIP:
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case LGraphType::PATH:
        v.path = new lgraph_result::Path(*value.v.path);
        break;
    default:
        type_ = LGraphType::NUL;
        break;
    }
}
// TODO(jiazheng): remove const and delete value' pointer
ResultElement::ResultElement(const ResultElement &&value) {
    type_ = value.type_;
    switch (value.type_) {
    case LGraphType::INTEGER:
    case LGraphType::FLOAT:
    case LGraphType::DOUBLE:
    case LGraphType::BOOLEAN:
    case LGraphType::STRING:
    case LGraphType::ANY:
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case LGraphType::LIST:
        v.list = new std::vector<json>(*value.v.list);
        break;
    case LGraphType::MAP:
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case LGraphType::NODE:
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case LGraphType::RELATIONSHIP:
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case LGraphType::PATH:
        v.path = value.v.path;
        break;
    default:
        type_ = LGraphType::NUL;
        break;
    }
}

ResultElement &ResultElement::operator=(const ResultElement &value) {
    if (this == &value) return *this;
    type_ = value.type_;
    switch (value.type_) {
    case LGraphType::INTEGER:
    case LGraphType::FLOAT:
    case LGraphType::DOUBLE:
    case LGraphType::BOOLEAN:
    case LGraphType::STRING:
    case LGraphType::ANY:
        delete v.fieldData;
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case LGraphType::LIST:
        delete v.list;
        v.list = new std::vector<json>(*value.v.list);
        break;
    case LGraphType::MAP:
        delete v.map;
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case LGraphType::NODE:
        delete v.node;
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case LGraphType::RELATIONSHIP:
        delete v.repl;
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case LGraphType::PATH:
        delete v.path;
        v.path = new lgraph_result::Path(*value.v.path);
        break;
    default:
        type_ = LGraphType::NUL;
        break;
    }
    return *this;
}

ResultElement &ResultElement::operator=(const ResultElement &&value) {
    if (this == &value) return *this;
    type_ = value.type_;
    switch (value.type_) {
    case LGraphType::INTEGER:
    case LGraphType::FLOAT:
    case LGraphType::DOUBLE:
    case LGraphType::BOOLEAN:
    case LGraphType::STRING:
    case LGraphType::ANY:
        delete v.fieldData;
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case LGraphType::LIST:
        delete v.list;
        v.list = new std::vector<json>(*value.v.list);
        break;
    case LGraphType::MAP:
        delete v.map;
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case LGraphType::NODE:
        delete v.node;
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case LGraphType::RELATIONSHIP:
        delete v.repl;
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case LGraphType::PATH:
        delete v.path;
        v.path = new lgraph_result::Path(*value.v.path);
        break;
    default:
        type_ = LGraphType::NUL;
        break;
    }
    return *this;
}

ResultElement::~ResultElement() {
    switch (type_) {
    case LGraphType::INTEGER:
    case LGraphType::FLOAT:
    case LGraphType::DOUBLE:
    case LGraphType::BOOLEAN:
    case LGraphType::STRING:
    case LGraphType::ANY:
        delete v.fieldData;
        break;
    case LGraphType::LIST:
        delete v.list;
        break;
    case LGraphType::MAP:
        delete v.map;
        break;
    case LGraphType::NODE:
        delete v.node;
        break;
    case LGraphType::RELATIONSHIP:
        delete v.repl;
        break;
    case LGraphType::PATH:
        delete v.path;
        break;
    default:
        break;
    }
}
json ResultElement::ToJson() {
    json result;
    std::map<std::string, json> properties;

    if (LGraphTypeIsField(type_) || LGraphTypeIsAny(type_)) {
        result = lgraph_rfc::FieldDataToJson(*v.fieldData);
    } else if (type_ == LGraphType::LIST) {
        for (auto &l : *v.list) {
            result.push_back(l);
        }
    } else if (type_ == LGraphType::MAP) {
        for (auto &m : *v.map) {
            result[m.first] = m.second;
        }
    } else if (type_ == LGraphType::NODE) {
        result = v.node->ToJson();
    } else if (type_ == LGraphType::RELATIONSHIP) {
        result = v.repl->ToJson();
    } else if (type_ == LGraphType::PATH) {
        for (auto p : *v.path) {
            result.emplace_back(p.ToJson());
        }
    }
    return result;
}

std::string ResultElement::ToString() { return ToJson().dump(); }
}  // namespace lgraph_api
