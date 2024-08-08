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
#include <memory>
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

bolt::Node Node::ToBolt() {
    bolt::Node ret;
    ret.id = id;
    ret.labels = {label};
    for (auto& pair : properties) {
        ret.props.emplace(pair.first, pair.second.ToBolt());
    }
    return ret;
}

nlohmann::json Relationship::ToJson() {
    if (id == -1) {
        return json("__null__");;
    }
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

bolt::RelNode Relationship::ToBoltUnbound(int64_t* v_eid) {
    bolt::RelNode rel;
    if (v_eid) {
        rel.id = (*v_eid)++;
    } else {
        rel.id = id;
    }
    rel.name = label;
    for (auto &pair : properties) {
        rel.props.emplace(pair.first, pair.second.ToBolt());
    }
    return rel;
}

bolt::Relationship Relationship::ToBolt(int64_t* v_eid) {
    bolt::Relationship rel;
    if (v_eid) {
        rel.id = (*v_eid)++;
    } else {
        rel.id = id;
    }
    rel.startId = src;
    rel.endId = dst;
    rel.type = label;
    for (auto &pair : properties) {
        rel.props.emplace(pair.first, pair.second.ToBolt());
    }
    return rel;
}

PathElement::PathElement(const PathElement &value) {
    type_ = value.type_;
    v = value.v;
}

PathElement::PathElement(PathElement &&value) {
    type_ = value.type_;
    v = value.v;
}

PathElement &PathElement::operator=(const PathElement &value) {
    if (this == &value) return *this;
    type_ = value.type_;
    v = value.v;
    return *this;
}

PathElement &PathElement::operator=(PathElement &&value) {
    if (this == &value) return *this;
    type_ = value.type_;
    v = value.v;
    return *this;
}

nlohmann::json PathElement::ToJson() {
    if (type_ == LGraphType::NODE) {
        return std::get<std::shared_ptr<Node>>(v)->ToJson();
    } else {
        return std::get<std::shared_ptr<Relationship>>(v)->ToJson();
    }
}

PathElement::~PathElement() {
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

std::any ResultElement::ToBolt(int64_t* v_eid) {
    if (LGraphTypeIsField(type_) || LGraphTypeIsAny(type_)) {
        return v.fieldData->ToBolt();
    } else if (type_ == LGraphType::LIST) {
        json result;
        for (auto &l : *v.list) {
            result.push_back(l);
        }
        return result.dump();  // Convert to string
        /*std::vector<std::any> ret;
        for (auto &l : *v.list) {
            if (l.is_null()) {
                ret.emplace_back();
            } else if (l.is_number_float()) {
                ret.emplace_back(l.get<float>());
            } else if (l.is_number_integer()) {
                ret.emplace_back(l.get<int64_t>());
            } else if (l.is_boolean()) {
                ret.emplace_back(l.get<bool>());
            } else if (l.is_string()) {
                ret.emplace_back(l.get<std::string>());
            } else {
                THROW_CODE(InputError,
                    "ToBolt: unsupported item in list: {}", l.dump());
            }
        }
        return ret;*/
    } else if (type_ == LGraphType::MAP) {
        json result;
        for (auto &m : *v.map) {
            result[m.first] = m.second;
        }
        return result.dump();  // Convert to string
        /*std::unordered_map<std::string, std::any> ret;
        for (auto &pair : *v.map) {
            if (pair.second.is_null()) {
                ret.emplace(pair.first, std::any{});
            } else if (pair.second.is_number_float()) {
                ret.emplace(pair.first, pair.second.get<float>());
            } else if (pair.second.is_number_integer()) {
                ret.emplace(pair.first, pair.second.get<int64_t>());
            } else if (pair.second.is_boolean()) {
                ret.emplace(pair.first, pair.second.get<bool>());
            } else if (pair.second.is_string()) {
                ret.emplace(pair.first, pair.second.get<std::string>());
            } else {
                THROW_CODE(InputError,
                    "ToBolt: unsupported value in map: {}", pair.second.dump());
            }
        }
        return ret;*/
    } else if (type_ == LGraphType::NODE) {
        if (v.node->id == -1) {
            return {};
        } else {
            return v.node->ToBolt();
        }
    } else if (type_ == LGraphType::RELATIONSHIP) {
        if (v.repl->id == -1) {
            return {};
        } else {
            return v.repl->ToBolt(v_eid);
        }
    } else if (type_ == LGraphType::PATH) {
        bolt::InternalPath path;
        for (size_t i = 0; i < v.path->size(); i++) {
            auto& p = (*v.path)[i];
            if (p.type_ == LGraphType::NODE) {
                path.nodes.push_back(std::get<std::shared_ptr<lgraph_result::Node>>(p.v)->ToBolt());
            } else {
                // The neo4j python client checks the uniqueness of the edge id.
                path.rels.push_back(std::get<std::shared_ptr<lgraph_result::Relationship>>(p.v)
                                    ->ToBoltUnbound(v_eid));
            }
            if (i >= 1) {
                if (i%2 == 1) {
                    if (std::get<std::shared_ptr<lgraph_result::Relationship>>(p.v)->src
                        == path.nodes.back().id) {
                        path.indices.push_back((int)path.rels.size());
                    } else {
                        path.indices.push_back(0-(int)path.rels.size());
                    }
                } else {
                    path.indices.push_back((int)path.nodes.size()-1);
                }
            }
        }
        return path;
    } else {
        THROW_CODE(InputError,
            "ToBolt: unsupported field type: {}", to_string(type_));
    }
}

std::string ResultElement::ToString() { return ToJson().dump(); }
}  // namespace lgraph_api
