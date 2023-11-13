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
#include "lgraph/lgraph_result.h"
#include "lgraph_api/result_element.h"
#include "server/json_convert.h"
#include "core/transaction.h"

using json = nlohmann::json;

namespace lgraph_api {

Record::Record(const std::vector<std::pair<std::string, LGraphType>> &args) {
    for (auto arg : args) {
        auto key = arg.first;
        auto value = arg.second;
        header[key] = value;
    }
    length_ = 0;
}

Record::Record(const Record &rhs) {
    header = rhs.header;
    for (auto r : rhs.record) {
        record[r.first] = std::shared_ptr<ResultElement>(new ResultElement(*r.second));
    }
}

Record::Record(Record &&rhs) {
    header = rhs.header;
    for (auto r : rhs.record) {
        record[r.first] = r.second;
        r.second = nullptr;
    }
}

Record &Record::operator=(const Record &rhs) {
    if (this == &rhs) return *this;
    header = rhs.header;
    for (auto r : rhs.record) {
        record[r.first] = std::shared_ptr<ResultElement>(new ResultElement(*r.second));
    }
    return *this;
}

Record &Record::operator=(Record &&rhs) {
    if (this == &rhs) return *this;
    header = rhs.header;
    for (auto r : rhs.record) {
        record[r.first] = r.second;
        r.second = nullptr;
    }
    return *this;
}

void Record::Insert(const std::string &key, const FieldData &value) {
    if (!HasKey(key)) {
        throw std::runtime_error(FMA_FMT("[Result ERROR] the variable {} is not exist", key));
    }
    if (header[key] == LGraphType::INTEGER &&
        (value.type == FieldType::INT8 || value.type == FieldType::INT16 ||
         value.type == FieldType::INT32 || value.type == FieldType::INT64)) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, LGraphType::INTEGER));
    } else if (header[key] == LGraphType::FLOAT && value.type == FieldType::FLOAT) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, LGraphType::FLOAT));
    } else if (header[key] == LGraphType::DOUBLE && value.type == FieldType::DOUBLE) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, LGraphType::DOUBLE));
    } else if (header[key] == LGraphType::BOOLEAN && value.type == FieldType::BOOL) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, LGraphType::BOOLEAN));
    } else if (header[key] == LGraphType::STRING && value.type == FieldType::STRING) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, LGraphType::STRING));
    } else if (header[key] == LGraphType::PATH && value.type == FieldType::STRING) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, LGraphType::PATH));
    } else if (header[key] == LGraphType::ANY) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, LGraphType::ANY));
    } else {
        throw std::runtime_error("[Result ERROR] type is valid");
    }

    length_++;
}

void Record::Insert(const std::string &key, const std::map<std::string, FieldData> &value) {
    if (!HasKey(key) || header[key] != LGraphType::MAP) {
        throw std::runtime_error(FMA_FMT("[Result ERROR] the variable {} is not exist", key));
    }
    std::map<std::string, json> j_value;
    for (auto &v : value) {
        j_value[v.first] = lgraph_rfc::FieldDataToJson(v.second);
    }
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(j_value));
    length_++;
}

void Record::Insert(const std::string &key, const std::vector<FieldData> &value) {
    if (!HasKey(key) || header[key] != LGraphType::LIST) {
        throw std::runtime_error(FMA_FMT("[Result ERROR] the variable {} is not exist", key));
    }
    std::vector<json> j_value;
    for (auto &v : value) {
        j_value.emplace_back(lgraph_rfc::FieldDataToJson(v));
    }
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(j_value));
    length_++;
}

void Record::Insert(const std::string &key, const lgraph_api::VertexIterator &vertex_it) {
    if (!HasKey(key) || header[key] != LGraphType::NODE) {
        throw std::runtime_error(
            FMA_FMT("[STANDARD RESULT ERROR] the variable {} is not exist", key));
    }
    if (!vertex_it.IsValid()) {
        throw std::runtime_error("[Result ERROR] the vertex iterator is not valid");
    }
    lgraph_result::Node node;
    node.id = vertex_it.GetId();
    node.label = vertex_it.GetLabel();
    node.properties = vertex_it.GetAllFields();
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(node));
    length_++;
}

void Record::Insert(const std::string &key, const lgraph_api::InEdgeIterator &in_edge_it) {
    if (!HasKey(key) || header[key] != LGraphType::RELATIONSHIP) {
        throw std::runtime_error(
            FMA_FMT("[STANDARD RESULT ERROR] the variable {} is not exist", key));
    }
    if (!in_edge_it.IsValid()) {
        throw std::runtime_error("[STANDARD RESULT ERROR] the in_edge iterator is not valid");
    }
    lgraph_result::Relationship repl;
    auto uid = in_edge_it.GetUid();
    repl.id = uid.eid;
    repl.src = uid.src;
    repl.dst = uid.dst;
    repl.label_id = uid.lid;
    repl.tid = uid.tid;
    repl.forward = false;
    repl.label = in_edge_it.GetLabel();
    repl.properties = in_edge_it.GetAllFields();
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

void Record::Insert(const std::string &key, const lgraph_api::OutEdgeIterator &out_edge_it) {
    if (!HasKey(key) || header[key] != LGraphType::RELATIONSHIP) {
        throw std::runtime_error(
            FMA_FMT("[STANDARD RESULT ERROR] the variable {} is not exist", key));
    }
    if (!out_edge_it.IsValid()) {
        throw std::runtime_error("[STANDARD RESULT ERROR] the out_edge iterator is not valid");
    }
    lgraph_result::Relationship repl;
    auto uid = out_edge_it.GetUid();
    repl.id = uid.eid;
    repl.src = uid.src;
    repl.dst = uid.dst;
    repl.label_id = uid.lid;
    repl.label = out_edge_it.GetLabel();
    repl.tid = uid.tid;
    repl.forward = true;
    repl.properties = out_edge_it.GetAllFields();
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

void Record::Insert(const std::string &key, const int64_t vid, lgraph_api::Transaction *txn) {
    auto core_txn = txn->GetTxn().get();
    auto vit = core_txn->GetVertexIterator(vid);
    lgraph_result::Node node;
    node.id = vid;
    node.label = core_txn->GetVertexLabel(vit);
    for (auto &property : core_txn->GetVertexFields(vit)) {
        node.properties.insert(property);
    }
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(node));
    length_++;
}

void Record::InsertVertexByID(const std::string &key, int64_t node_id) {
    lgraph_result::Node node;
    node.id = node_id;
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(node));
    length_++;
}

void Record::Insert(const std::string &key, EdgeUid &uid, lgraph_api::Transaction *txn) {
    auto core_txn = txn->GetTxn().get();
    auto eit = core_txn->GetOutEdgeIterator(uid, false);
    lgraph_result::Relationship repl;
    repl.id = uid.eid;
    repl.src = uid.src;
    repl.dst = uid.dst;
    repl.label_id = uid.lid;
    repl.label = core_txn->GetEdgeLabel(eit);
    repl.tid = uid.tid;
    // repl.forward is unknown
    auto rel_fields = core_txn->GetEdgeFields(eit);
    for (auto &property : rel_fields) {
        repl.properties.insert(property);
    }
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

void Record::InsertEdgeByID(const std::string &key, const EdgeUid &uid) {
    lgraph_result::Relationship repl;
    repl.id = uid.eid;
    repl.src = uid.src;
    repl.dst = uid.dst;
    repl.label_id = uid.lid;
    repl.tid = uid.tid;
    // repl.label is unknown
    // repl.forward is unknown
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

Result::Result() : row_count_(-1) {}

void Record::Insert(const std::string &key, const traversal::Path &path,
                    lgraph_api::Transaction *txn) {
    auto core_txn = txn->GetTxn().get();
    if (!HasKey(key) || header[key] != LGraphType::PATH) {
        throw std::runtime_error(
            FMA_FMT("[STANDARD RESULT ERROR] the variable {} is not exist", key));
    }
    lgraph_result::Path result_path;
    if (path.Length() == 0) {
        record[key] = std::shared_ptr<ResultElement>(new ResultElement(result_path));
    }
    for (size_t i = 0; i < path.Length(); i++) {
        lgraph_result::Node node;
        auto vid = path.GetNthVertex(i).GetId();
        auto vit = core_txn->GetVertexIterator(vid);
        node.id = vid;
        node.label = core_txn->GetVertexLabel(vit);
        for (const auto &property : core_txn->GetVertexFields(vit)) {
            node.properties[property.first] = property.second;
        }
        result_path.emplace_back(lgraph_result::PathElement(std::move(node)));
        auto edge = path.GetNthEdge(i);
        lgraph_result::Relationship repl;
        auto euid = lgraph::EdgeUid(edge.GetSrcVertex().GetId(), edge.GetDstVertex().GetId(),
                                    edge.GetLabelId(), edge.GetTemporalId(), edge.GetEdgeId());
        auto eit = core_txn->GetOutEdgeIterator(euid, false);
        repl.id = euid.eid;
        repl.src = euid.src;
        repl.dst = euid.dst;
        repl.label_id = euid.lid;
        repl.tid = euid.tid;
        repl.label = core_txn->GetEdgeLabel(eit);
        repl.forward = ((int64_t)vid == euid.src);
        for (const auto &property : core_txn->GetEdgeFields(eit)) {
            repl.properties[property.first] = property.second;
        }
        result_path.emplace_back(lgraph_result::PathElement(std::move(repl)));
    }
    lgraph_result::Node node;
    auto vid = path.GetEndVertex().GetId();
    auto vit = core_txn->GetVertexIterator(vid);
    node.id = vid;
    node.label = core_txn->GetVertexLabel(vit);
    for (const auto &property : core_txn->GetVertexFields(vit)) {
        node.properties[property.first] = property.second;
    }
    result_path.emplace_back(lgraph_result::PathElement(std::move(node)));
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(result_path));
    length_++;
}

Result::Result(const std::initializer_list<std::pair<std::string, LGraphType>> &args) {
    for (auto h : args) header.push_back(h);
    row_count_ = -1;
}

Record *Result::MutableRecord() {
    result.emplace_back(Record(header));
    row_count_++;
    return &result[row_count_];
}

void Result::ResetHeader(const std::vector<std::pair<std::string, LGraphType>> &new_header) {
    result.clear();
    header = new_header;
    row_count_ = -1;
}

void Result::ResetHeader(
    const std::initializer_list<std::pair<std::string, LGraphType>> &new_header) {
    result.clear();
    header.clear();
    row_count_ = -1;
    for (auto h : new_header) header.push_back(h);
}

const std::vector<std::pair<std::string, LGraphType>> &Result::Header() { return header; }

Result::Result(const std::vector<std::pair<std::string, LGraphType>> &args) {
    header = args;
    row_count_ = -1;
}

int64_t Result::Size() const { return row_count_ + 1; }

const std::unordered_map<std::string, std::shared_ptr<ResultElement>> &Result::RecordView(
    int64_t row_num) {
    if (row_num > row_count_) {
        throw std::runtime_error(
            FMA_FMT("[RecordView ERROR] table size is {}, but row_num is {}", row_count_, row_num));
    }
    return result[row_num].record;
}

LGraphType Result::GetType(std::string title) {
    for (auto &h : header) {
        if (h.first == title) {
            return h.second;
        }
    }
    throw std::runtime_error(FMA_FMT("[Output Error] the {} is not exist", title));
}

std::string Result::Dump(bool is_standard) {
    json arr = json::array();
    for (auto& record : result) {
        json j;
        for (auto& h : header) {
            if (record.record.find(h.first) == record.record.end() &&
                record.record[h.first] == nullptr) {
                continue;
            }
            j[h.first] = record.record[h.first]->ToJson();
        }
        if (j.is_null()) {
            throw std::runtime_error(
                "result has a null row!. mMaybe your new record  not a reference");
        }
        arr.emplace_back(j);
    }
    if (is_standard) {
        json output;
        output["header"] = header;
        output["is_standard"] = true;
        output["data"] = arr;
        return output.dump();
    } else {
        return arr.dump();
    }
}

void Result::Load(const std::string &output) {
    try {
        auto j = json::parse(output);
        if (j["is_standard"].get<bool>() != true) {
            throw std::runtime_error("result is not standard");
        }
        // clear origin data
        result.clear();
        header.clear();

        auto j_header = j["header"];
        auto data = j["data"];
        for (auto h : j_header) {
            std::string title_name = h[0].get<std::string>();
            LGraphType title_type = h[1].get<LGraphType>();
            header.push_back({title_name, title_type});
        }
        for (auto &row : data) {
            auto record = this->MutableRecord();
            for (auto &col : row.items()) {
                auto title = col.key();
                switch (GetType(title)) {
                case LGraphType::INTEGER:
                    record->Insert(title, FieldData(std::forward<int>(col.value().get<int>())));
                    break;
                case LGraphType::FLOAT:
                    record->Insert(title, FieldData(std::forward<float>(col.value().get<float>())));
                    break;
                case LGraphType::DOUBLE:
                    record->Insert(title,
                                  FieldData(std::forward<double>(col.value().get<double>())));
                    break;
                case LGraphType::BOOLEAN:
                    record->Insert(title, FieldData(std::forward<bool>(col.value().get<bool>())));
                    break;
                case LGraphType::STRING:
                case LGraphType::ANY:
                    record->Insert(
                        title,
                        FieldData(std::forward<std::string>(col.value().get<std::string>())));
                    break;
                case LGraphType::LIST:
                    {
                        std::vector<FieldData> list;
                        for (auto &obj : col.value()) list.push_back(FieldData(obj.dump()));
                        record->Insert(title, list);
                    }
                    break;
                case LGraphType::MAP:
                    {
                        std::map<std::string, FieldData> map;
                        for (auto &obj : col.value().items())
                            map[obj.key()] = FieldData(lgraph_rfc::JsonToFieldData(obj.value()));
                        record->Insert(title, map);
                    }
                    break;
                case LGraphType::NODE:
                    {
                        lgraph_result::Node node;
                        node.id = col.value()["identity"].get<int64_t>();
                        node.label = col.value()["label"].get<std::string>();
                        std::map<std::string, FieldData> properties;
                        for (auto &obj : col.value()["properties"].items())
                            properties[obj.key()] = lgraph_rfc::JsonToFieldData(obj.value());
                        node.properties = properties;
                        record->record[title] =
                            std::shared_ptr<ResultElement>(new ResultElement(node));
                        record->length_++;
                    }
                    break;
                case LGraphType::RELATIONSHIP:
                    {
                        lgraph_result::Relationship repl;
                        repl.id = col.value()["identity"].get<int64_t>();
                        repl.src = col.value()["src"].get<int64_t>();
                        repl.dst = col.value()["dst"].get<int64_t>();
                        repl.label_id = col.value()["label_id"].get<uint16_t>();
                        repl.tid = col.value()["temporal_id"].get<int64_t>();
                        std::map<std::string, FieldData> properties;
                        for (auto &obj : col.value()["properties"].items())
                            properties[obj.key()] = lgraph_rfc::JsonToFieldData(obj.value());
                        repl.properties = properties;
                        record->record[title] =
                            std::shared_ptr<ResultElement>(new ResultElement(repl));
                        record->length_++;
                    }
                    break;
                case LGraphType::PATH:
                    {
                        bool is_node = true;
                        lgraph_result::Path path;
                        for (auto &json_path : col.value()) {
                            if (is_node) {
                                lgraph_result::Node node;
                                node.id = json_path["identity"].get<int64_t>();
                                node.label = json_path["label"].get<std::string>();
                                std::map<std::string, FieldData> properties;
                                for (auto &obj : json_path["properties"].items())
                                    properties[obj.key()] =
                                        lgraph_rfc::JsonToFieldData(obj.value());
                                node.properties = properties;
                                path.emplace_back(lgraph_result::PathElement(node));
                            } else {
                                lgraph_result::Relationship repl;
                                repl.src = json_path["src"].get<int64_t>();
                                repl.dst = json_path["dst"].get<int64_t>();
                                repl.label_id = json_path["label_id"].get<uint16_t>();
                                repl.tid = json_path["temporal_id"].get<int64_t>();
                                repl.id = json_path["identity"].get<int64_t>();
                                std::map<std::string, FieldData> properties;
                                for (auto &obj : json_path["properties"].items())
                                    properties[obj.key()] =
                                        lgraph_rfc::JsonToFieldData(obj.value());
                                repl.properties = properties;
                                path.emplace_back(lgraph_result::PathElement(repl));
                            }
                            is_node = !is_node;
                            record->length_++;
                            record->record[title] =
                                std::shared_ptr<ResultElement>(new ResultElement(path));
                        }
                    }
                    break;
                default:
                    break;
                }
            }
        }
    } catch (std::exception &e) {
        auto record = this->MutableRecord();
        record->Insert(header[0].first, FieldData(output));
        record->length_++;
        FMA_LOG() << FMA_FMT("[Plugin Error] Error: {}", e.what());
    }
}

}  // namespace lgraph_api
