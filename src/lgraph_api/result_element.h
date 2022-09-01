/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <map>
#include "lgraph/lgraph_result.h"

namespace lgraph_api {
typedef int64_t VertexId, RelpID;

struct Node {
    VertexId id;
    std::string label;
    std::map<std::string, FieldData> properties;
};

struct Relationship {
    RelpID id;
    VertexId start;
    VertexId end;
    uint16_t label_id;
    std::string label;
    std::map<std::string, FieldData> properties;
};

struct ResultElement {
    ResultElementType type_;
    union {
        Node *node;
        Relationship *repl;
        FieldData *fieldData;
        std::vector<nlohmann::json> *list;
        std::map<std::string, nlohmann::json> *map;
    } v;

    ResultElement() { type_ = ResultElementType::NUL; }
    explicit ResultElement(const FieldData &data) {
        type_ = ResultElementType::ANY;
        v.fieldData = new FieldData(data);
    }
    explicit ResultElement(const FieldData &data, const ResultElementType &type) {
        type_ = type;
        v.fieldData = new FieldData(data);
    }

    explicit ResultElement(const std::map<std::string, nlohmann::json> &data) {
        type_ = ResultElementType::MAP;
        v.map = new std::map<std::string, nlohmann::json>(data);
    }
    explicit ResultElement(const std::vector<nlohmann::json> &data) {
        type_ = ResultElementType::LIST;
        v.list = new std::vector<nlohmann::json>(data);
    }
    explicit ResultElement(const Node &data) {
        type_ = ResultElementType::NODE;
        v.node = new Node(data);
    }
    explicit ResultElement(const Relationship &data) {
        type_ = ResultElementType::RELATIONSHIP;
        v.repl = new Relationship(data);
    }

    ResultElement(const ResultElement &);
    ResultElement(const ResultElement &&);

    inline ResultElement &operator=(const ResultElement &);

    inline ResultElement &operator=(const ResultElement &&);

    ~ResultElement();

    nlohmann::json ToJson();
    std::string ToString();
};

}  // namespace lgraph_api
