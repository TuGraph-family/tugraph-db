/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph_api/result_element.h"
#include "server/json_convert.h"
#include "fma-common/string_formatter.h"

using json = nlohmann::json;

namespace lgraph_api {

ResultElement::ResultElement(const ResultElement &value) {
    type_ = value.type_;
    switch (value.type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::PATH:
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
        v.node = new Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        v.repl = new Relationship(*value.v.repl);
        break;
    default:
        type_ = ResultElementType::NUL;
        break;
    }
}
// 改一下 ，指针转移
ResultElement::ResultElement(const ResultElement &&value) {
    type_ = value.type_;
    switch (value.type_) {
    case ResultElementType::INTEGER:
    case ResultElementType::FLOAT:
    case ResultElementType::DOUBLE:
    case ResultElementType::BOOLEAN:
    case ResultElementType::STRING:
    case ResultElementType::PATH:
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
        v.node = new Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        v.repl = new Relationship(*value.v.repl);
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
    case ResultElementType::PATH:
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
        v.node = new Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        delete v.repl;
        v.repl = new Relationship(*value.v.repl);
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
    case ResultElementType::PATH:
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
        v.node = new Node(*value.v.node);
        break;
    case ResultElementType::RELATIONSHIP:
        delete v.repl;
        v.repl = new Relationship(*value.v.repl);
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
    case ResultElementType::PATH:
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
    case ResultElementType::PATH:
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
        result["identity"] = v.node->id;
        if (!v.node->label.empty()) {
            result["label"] = v.node->label;
        }
        for (auto p : v.node->properties) {
            properties[p.first] = lgraph_rfc::FieldDataToJson(p.second);
        }
        if (!properties.empty()) {
            result["properties"] = properties;
        }
        break;
    case ResultElementType::RELATIONSHIP:
        result["identity"] = v.repl->id;
        result["start"] = v.repl->start;
        result["end"] = v.repl->end;
        if (!v.repl->label.empty()) {
            result["label"] = v.repl->label;
        }
        result["label_id"] = v.repl->label_id;
        for (auto p : v.repl->properties) {
            properties[p.first] = lgraph_rfc::FieldDataToJson(p.second);
        }
        if (!properties.empty()) {
            result["properties"] = properties;
        }
        break;
    default:
        break;
    }
    return result;
}

std::string ResultElement::ToString() {
    return ToJson().dump();
}

}  // namespace lgraph_api
