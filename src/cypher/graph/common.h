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

//
// Created by wt on 6/12/18.
//
#pragma once
#include <string>
#include <unordered_set>
#include "common/value.h"
#include "graphdb/graph_entity.h"
namespace cypher {

typedef int64_t NodeID;
typedef int64_t RelpID;

struct Property {
    std::string field;
    std::string value_alias;
    std::string map_field_name;
    Value value;
    enum {
        NUL,        // empty
        //PARAMETER,  // {name:$name}
        VALUE,      // {name:'Tom Hanks'}
        VARIABLE,   // UNWIND [1,2] AS mid MATCH (n {id:mid}) || WITH {a: 1, b: 2} as pair
    } type;

    Property() : type(NUL) {}
};

/* See also traversal::Path */
struct Path {
    std::vector<graphdb::Edge> edges;
    std::vector<graphdb::Vertex> vertexes;

    Path() = default;

    void SetStart(const graphdb::Vertex &vertex) {
        vertexes.push_back(vertex);
    }

    [[nodiscard]] bool Empty() const { return vertexes.empty(); }

    [[nodiscard]] size_t Length() const { return vertexes.size(); }

    void Append(const graphdb::Edge &edge, const graphdb::Vertex &vertex) {
        edges.push_back(edge);
        vertexes.push_back(vertex);
    }

    void PopBack() {
        edges.pop_back();
        vertexes.pop_back();
    }

    void Clear() {
        edges.clear();
        vertexes.clear();
    }
};
}  // namespace cypher
