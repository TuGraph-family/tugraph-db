/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
    if (label.empty()) {
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
    if (type_ == ResultElementType::NODE) {
        v.node = new lgraph_result::Node(*value.v.node);
    } else {
        v.repl = new Relationship(*value.v.repl);
    }
}

PathElement::PathElement(PathElement &&value) {
    type_ = value.type_;
    if (type_ == ResultElementType::NODE) {
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
    if (type_ == ResultElementType::NODE) {
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
    if (type_ == ResultElementType::NODE) {
        v.node = value.v.node;
        value.v.node = nullptr;
    } else {
        v.repl = value.v.repl;
        value.v.repl = nullptr;
    }
    return *this;
}

nlohmann::json PathElement::ToJson() {
    if (type_ == ResultElementType::NODE) {
        return v.node->ToJson();
    } else {
        return v.repl->ToJson();
    }
}

PathElement::~PathElement() {
    if (type_ == ResultElementType::NODE) {
        delete v.node;
    } else {
        delete v.repl;
    }
}

}  // namespace lgraph_result

ResultElement::ResultElement(const ResultElement &value) {
    type_ = value.type_;
    switch (type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::ANY:
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case ResultElementType::LIST:
        v.list = new std::vector<json>(*value.v.list);
        break;
    case ResultElementType::MAP:
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case ResultElementType::NODE:
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case ResultElementType::PATH:
        v.path = new lgraph_result::Path(*value.v.path);
        break;
    default:
        type_ = ResultElementType::NUL;
        break;
    }
}
// TODO(jiazheng): remove const and delete value' pointer
ResultElement::ResultElement(const ResultElement &&value) {
    type_ = value.type_;
    switch (value.type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::ANY:
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case ResultElementType::LIST:
        v.list = new std::vector<json>(*value.v.list);
        break;
    case ResultElementType::MAP:
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case ResultElementType::NODE:
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case ResultElementType::PATH:
        v.path = value.v.path;
        break;
    default:
        type_ = ResultElementType::NUL;
        break;
    }
}

ResultElement &ResultElement::operator=(const ResultElement &value) {
    if (this == &value) return *this;
    type_ = value.type_;
    switch (value.type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::ANY:
        delete v.fieldData;
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case ResultElementType::LIST:
        delete v.list;
        v.list = new std::vector<json>(*value.v.list);
        break;
    case ResultElementType::MAP:
        delete v.map;
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case ResultElementType::NODE:
        delete v.node;
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        delete v.repl;
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case ResultElementType::PATH:
        delete v.path;
        v.path = new lgraph_result::Path(*value.v.path);
        break;
    default:
        type_ = ResultElementType::NUL;
        break;
    }
}

ResultElement &ResultElement::operator=(const ResultElement &&value) {
    if (this == &value) return *this;
    type_ = value.type_;
    switch (value.type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::ANY:
        delete v.fieldData;
        v.fieldData = new lgraph_api::FieldData(*value.v.fieldData);
        break;
    case ResultElementType::LIST:
        delete v.list;
        v.list = new std::vector<json>(*value.v.list);
        break;
    case ResultElementType::MAP:
        delete v.map;
        v.map = new std::map<std::string, json>(*value.v.map);
        break;
    case ResultElementType::NODE:
        delete v.node;
        v.node = new lgraph_result::Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        delete v.repl;
        v.repl = new lgraph_result::Relationship(*value.v.repl);
        break;
    case ResultElementType::PATH:
        delete v.path;
        v.path = new lgraph_result::Path(*value.v.path);
        break;
    default:
        type_ = ResultElementType::NUL;
        break;
    }
    return *this;
}

ResultElement::~ResultElement() {
    switch (type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::ANY:
        delete v.fieldData;
        break;
    case ResultElementType::LIST:
        delete v.list;
        break;
    case ResultElementType::MAP:
        delete v.map;
        break;
    case ResultElementType::NODE:
        delete v.node;
        break;
    case ResultElementType::RELATIONSHIP:
        delete v.repl;
        break;
    case ResultElementType::PATH:
        delete v.path;
        break;
    default:
        break;
    }
}
json ResultElement::ToJson() {
    json result;
    std::map<std::string, json> properties;

    switch (type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::FIELD:
    case ResultElementType::ANY:
        result = lgraph_rfc::FieldDataToJson(*v.fieldData);
        break;
    case ResultElementType::LIST:
        for (auto &l : *v.list) {
            result.push_back(l);
        }
        break;
    case ResultElementType::MAP:
        for (auto &m : *v.map) {
            result[m.first] = m.second;
        }
        break;
    case ResultElementType::NODE:
        result = v.node->ToJson();
        break;
    case ResultElementType::RELATIONSHIP:
        result = v.repl->ToJson();
        break;
    case ResultElementType::PATH:
        {
            for (auto p : *v.path) {
                result.emplace_back(p.ToJson());
            }
        }
    default:
        break;
    }
    return result;
}

std::string ResultElement::ToString() { return ToJson().dump(); }
}  // namespace lgraph_api
