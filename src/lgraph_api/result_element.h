﻿/**
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

#include <map>
#include "lgraph/lgraph_result.h"

namespace lgraph_api {
namespace lgraph_result {
typedef int64_t VertexId, RelpID;

struct Node {
    VertexId id;
    std::string label;
    std::map<std::string, lgraph_api::FieldData> properties;
    nlohmann::json ToJson();
};

struct Relationship {
    RelpID id;
    VertexId src;
    VertexId dst;
    uint16_t label_id;
    std::string label;
    int64_t tid;
    bool forward;
    std::map<std::string, lgraph_api::FieldData> properties;
    nlohmann::json ToJson();
};

// WARNING: [PathElement] just include node and relationship
//          So don't add new constructor before [Path] add
//          a new member.
struct PathElement {
    lgraph_api::ResultElementType type_;
    union {
        Node *node;
        Relationship *repl;
    } v;
    explicit PathElement(const Node &node) {
        type_ = lgraph_api::ResultElementType::NODE;
        v.node = new Node(node);
    }
    explicit PathElement(const Relationship &repl) {
        type_ = lgraph_api::ResultElementType::RELATIONSHIP;
        v.repl = new Relationship(repl);
    }
    PathElement(const PathElement &);
    PathElement(PathElement &&);
    inline PathElement &operator=(const PathElement &);
    inline PathElement &operator=(PathElement &&);
    nlohmann::json ToJson();
    std::string ToString();
    ~PathElement();
};

using Path = std::vector<PathElement>;

}  // namespace lgraph_result

struct ResultElement {
    // using Path = typename traversal::Path;
    ResultElementType type_;
    union {
        lgraph_result::Node *node;
        lgraph_result::Relationship *repl;
        FieldData *fieldData;
        std::vector<nlohmann::json> *list;
        std::map<std::string, nlohmann::json> *map;
        lgraph_result::Path *path;
    } v;

    ResultElement() : type_(ResultElementType::NUL) {}
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
    explicit ResultElement(const lgraph_result::Node &data) {
        type_ = ResultElementType::NODE;
        v.node = new lgraph_result::Node(data);
    }
    explicit ResultElement(const lgraph_result::Relationship &data) {
        type_ = ResultElementType::RELATIONSHIP;
        v.repl = new lgraph_result::Relationship(data);
    }

    explicit ResultElement(const lgraph_result::Path &data) {
        type_ = ResultElementType::PATH;
        v.path = new lgraph_result::Path(data);
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
