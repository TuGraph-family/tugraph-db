/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/string_formatter.h"
#include "lgraph_api/result_element.h"
#include "server/json_convert.h"
#include "cypher/filter/iterator.h"

using json = nlohmann::json;

namespace lgraph_api {

ResultElementType ResultElementTypeUpcast(ResultElementType type) {
    if ((static_cast<int>(type) & 0xF0) == static_cast<int>(ResultElementType::FIELD)) {
        return ResultElementType::FIELD;
    }
    if ((static_cast<int>(type) & 0xF0) == static_cast<int>(ResultElementType::GRAPH_ELEMENT)) {
        return ResultElementType::GRAPH_ELEMENT;
    }
    if ((static_cast<int>(type) & 0xF0) == static_cast<int>(ResultElementType::COLLECTION)) {
        return ResultElementType::COLLECTION;
    }
    if ((static_cast<int>(type) & 0xF0) == static_cast<int>(ResultElementType::ANY)) {
        return ResultElementType::ANY;
    } else {
        return type;
    }
}

Record::Record(const std::vector<std::pair<std::string, ResultElementType>> &args) {
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
    // 为了可读写做妥协
    if (header[key] == ResultElementType::INTEGER &&
        (value.type == FieldType::INT8 || value.type == FieldType::INT16 ||
         value.type == FieldType::INT32 || value.type == FieldType::INT64)) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, ResultElementType::INTEGER));
    } else if (header[key] == ResultElementType::FLOAT && value.type == FieldType::FLOAT) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, ResultElementType::FLOAT));
    } else if (header[key] == ResultElementType::DOUBLE && value.type == FieldType::DOUBLE) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, ResultElementType::DOUBLE));
    } else if (header[key] == ResultElementType::BOOLEAN && value.type == FieldType::BOOL) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, ResultElementType::BOOLEAN));
    } else if (header[key] == ResultElementType::STRING && value.type == FieldType::STRING) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, ResultElementType::STRING));
    } else if (header[key] == ResultElementType::PATH && value.type == FieldType::STRING) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, ResultElementType::PATH));
    } else if (header[key] == ResultElementType::ANY && value.type == FieldType::STRING) {
        record[key] =
            std::shared_ptr<ResultElement>(new ResultElement(value, ResultElementType::ANY));
    } else if (header[key] == ResultElementType::FIELD) {
        record[key] = std::shared_ptr<ResultElement>(new ResultElement(value));
    } else {
        throw std::runtime_error("[Result ERROR] type is valid");
    }

    length_++;
}

void Record::Insert(const std::string &key, const std::map<std::string, FieldData> &value) {
    if (!HasKey(key) || header[key] != ResultElementType::MAP) {
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
    if (!HasKey(key) || header[key] != ResultElementType::LIST) {
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
    if (!HasKey(key) || header[key] != ResultElementType::NODE) {
        throw std::runtime_error(FMA_FMT("[Plugin ERROR] the variable {} is not exist", key));
    }
    if (!vertex_it.IsValid()) {
        throw std::runtime_error("[Result ERROR] the vertex iterator is not valid");
    }
    Node node;
    node.id = vertex_it.GetId();
    node.label = vertex_it.GetLabel();
    node.properties = vertex_it.GetAllFields();
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(node));
    length_++;
}

void Record::Insert(const std::string &key, const lgraph_api::InEdgeIterator &in_edge_it) {
    if (!HasKey(key) || header[key] != ResultElementType::RELATIONSHIP) {
        throw std::runtime_error(FMA_FMT("[Plugin ERROR] the variable {} is not exist", key));
    }
    if (!in_edge_it.IsValid()) {
        throw std::runtime_error("[Plugin ERROR] the in_edge iterator is not valid");
    }
    Relationship repl;
    auto uid = in_edge_it.GetUid();
    repl.id = uid.eid;
    repl.start = uid.src;
    repl.end = uid.dst;
    repl.label_id = uid.lid;
    repl.label = in_edge_it.GetLabel();
    repl.properties = in_edge_it.GetAllFields();
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

void Record::Insert(const std::string &key, const lgraph_api::OutEdgeIterator &out_edge_it) {
    if (!HasKey(key) || header[key] != ResultElementType::RELATIONSHIP) {
        throw std::runtime_error(FMA_FMT("[Plugin ERROR] the variable {} is not exist", key));
    }
    if (!out_edge_it.IsValid()) {
        throw std::runtime_error("[Plugin ERROR] the out_edge iterator is not valid");
    }
    Relationship repl;
    auto uid = out_edge_it.GetUid();
    repl.id = uid.eid;
    repl.start = uid.src;
    repl.end = uid.dst;
    repl.label_id = uid.lid;
    repl.label = out_edge_it.GetLabel();
    repl.properties = out_edge_it.GetAllFields();
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

void Record::InsertVertex(const std::string &key,
        const int64_t vid,
        lgraph::Transaction* txn) {
    auto vit = std::make_unique<lgraph::VIter>(txn, lgraph::VIter::VERTEX_ITER, vid);
    Node node;
    node.id = vid;
    node.label = vit->GetLabel();
    auto node_fields = vit->GetFields();
    for (auto &property : node_fields) {
        node.properties.insert(property);
    }
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(node));
    length_++;
}

void Record::InsertVertexByID(const std::string &key, int64_t node_id) {
    Node node;
    node.id = node_id;
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(node));
    length_++;
}

void Record::InsertEdge(const std::string &key,
        EdgeUid &uid,
        lgraph::Transaction* txn) {
    auto vit = std::make_unique<lgraph::EIter>(txn, uid);
    Relationship repl;
    repl.id = uid.eid;
    repl.start = uid.src;
    repl.end = uid.dst;
    repl.label_id = uid.lid;
    repl.label = vit->GetLabel();
    auto rel_fields = vit->GetFields();
    for (auto &property : rel_fields) {
        repl.properties.insert(property);
    }
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

void Record::InsertEdgeByID(const std::string &key, const EdgeUid &uid) {
    Relationship repl;
    repl.id = uid.eid;
    repl.start = uid.src;
    repl.end = uid.dst;
    repl.label_id = uid.lid;
    record[key] = std::shared_ptr<ResultElement>(new ResultElement(repl));
    length_++;
}

Result::Result() : row_count_(-1) {}

Result::Result(const std::initializer_list<std::pair<std::string, ResultElementType>> &args) {
    for (auto h : args) header.push_back(h);
    row_count_ = -1;
}

Record &Result::NewRecord() {
    result.emplace_back(Record(header));
    row_count_++;
    return result[row_count_];
}

void Result::ResetHeader(const std::vector<std::pair<std::string, ResultElementType>> &new_header) {
    result.clear();
    header = new_header;
    row_count_ = -1;
}

void Result::ResetHeader(
    const std::initializer_list<std::pair<std::string, ResultElementType>> &new_header) {
    result.clear();
    header.clear();
    row_count_ = -1;
    for (auto h : new_header) header.push_back(h);
}

const std::vector<std::pair<std::string, ResultElementType>> &Result::Header() { return header; }

Result::Result(const std::vector<std::pair<std::string, ResultElementType>> &args) {
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

ResultElementType Result::GetType(std::string title) {
    for (auto &h : header) {
        if (h.first == title) {
            return h.second;
        }
    }
    throw std::runtime_error(FMA_FMT("[Output Error] the {} is not exist", title));
}

std::string Result::Dump(bool is_standard) {
    // output["header"] = header;
    // output["row_num"] = row_count_;
    json arr = json::array();
    for (auto record : result) {
        json j;
        for (auto h : header) {
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
        if (row_count_ == -1) {
            json j;
            return j.dump();
        } else if (row_count_ == 0) {
            json j;
            for (auto h : header) {
                j[h.first] = result[0].record[h.first]->ToJson();
            }
            return j.dump();
        } else {
            return arr.dump();
        }
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
            ResultElementType title_type = h[1].get<ResultElementType>();
            header.push_back({title_name, title_type});
        }
        for (auto &row : data) {
            auto &record = this->NewRecord();
            for (auto &col : row.items()) {
                auto title = col.key();
                switch (GetType(title)) {
                case ResultElementType::INTEGER:
                    record.Insert(title, FieldData(std::forward<int>(col.value().get<int>())));
                    break;
                case ResultElementType::FLOAT:
                    record.Insert(title, FieldData(std::forward<float>(col.value().get<float>())));
                    break;
                case ResultElementType::DOUBLE:
                    record.Insert(title,
                                  FieldData(std::forward<double>(col.value().get<double>())));
                    break;
                case ResultElementType::BOOLEAN:
                    record.Insert(title, FieldData(std::forward<bool>(col.value().get<bool>())));
                    break;
                case ResultElementType::STRING:
                case ResultElementType::PATH:
                case ResultElementType::ANY:
                    record.Insert(
                        title,
                        FieldData(std::forward<std::string>(col.value().get<std::string>())));
                    break;
                case ResultElementType::FIELD:
                    record.Insert(title, FieldData(std::forward<FieldData>(
                                             lgraph_rfc::JsonToFieldData(col.value()))));
                case ResultElementType::LIST:
                    {
                        std::vector<FieldData> list;
                        for (auto &obj : col.value()) list.push_back(FieldData(obj.dump()));
                        record.Insert(title, list);
                    }
                    break;
                case ResultElementType::MAP:
                    {
                        std::map<std::string, FieldData> map;
                        for (auto &obj : col.value().items())
                            map[obj.key()] = FieldData(lgraph_rfc::JsonToFieldData(obj.value()));
                        record.Insert(title, map);
                    }
                    break;
                case ResultElementType::NODE:
                    {
                        Node node;
                        node.id = col.value()["identity"].get<int64_t>();
                        node.label = col.value()["label"].get<std::string>();
                        std::map<std::string, FieldData> properties;
                        for (auto &obj : col.value()["properties"].items())
                            properties[obj.key()] = lgraph_rfc::JsonToFieldData(obj.value());
                        node.properties = properties;
                        record.record[title] =
                            std::shared_ptr<ResultElement>(new ResultElement(node));
                        record.length_++;
                    }
                    break;
                case ResultElementType::RELATIONSHIP:
                    {
                        Relationship repl;
                        repl.id = col.value()["identity"].get<int64_t>();
                        repl.start = col.value()["start"].get<int64_t>();
                        repl.end = col.value()["end"].get<int64_t>();
                        repl.label_id = col.value()["label_id"].get<uint16_t>();
                        std::map<std::string, FieldData> properties;
                        for (auto &obj : col.value()["properties"].items())
                            properties[obj.key()] = lgraph_rfc::JsonToFieldData(obj.value());
                        repl.properties = properties;
                        record.record[title] =
                            std::shared_ptr<ResultElement>(new ResultElement(repl));
                        record.length_++;
                    }
                    break;
                default:
                    break;
                }
            }
        }
    } catch (std::exception &e) {
        auto &record = this->NewRecord();
        record.Insert(header[0].first, FieldData(output));
        record.length_++;
        FMA_LOG() << FMA_FMT("[Plugin Error] Error: {}", e.what());
    }
}

}  // namespace lgraph_api
