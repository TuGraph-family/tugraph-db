/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Copyright (c) "Neo4j"
 * Neo4j Sweden AB [https://neo4j.com]
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

/*
 * written by botu.wzy, inspired by Neo4j Go Driver
 */
#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <memory>
#include <any>

namespace bolt {
struct Node {
    int64_t id;                                       // id of this Node.
    std::string elementId;                            // elementId of this Node.
    std::vector<std::string> labels;                  // labels attached to this Node.
    std::unordered_map<std::string, std::any> props;  // Properties of this Node.
};

struct Relationship  {
    int64_t id;                  // id of this Relationship.
    std::string elementId;       // elementId of this Relationship.
    int64_t startId;             // id of the start Node of this Relationship.
    std::string startElementId;  // elementId of the start Node of this Relationship.
    int64_t endId;               // id of the End Node of this Relationship.
    std::string endElementId;    // elementId of the End Node of this Relationship.
    std::string type;            // type of this Relationship.
    std::unordered_map<std::string, std::any> props;  // Properties of this Relationship.
};

// Path represents a directed sequence of relationships between two nodes.
// This generally represents a traversal or walk through a graph and maintains a direction
// separate from that of any
// relationships traversed. It is allowed to be of size 0, meaning there are no relationships in it.
// In this case, it contains only a single node which is both the start and the End of the path.
struct Path {
    std::vector<Node> nodes;  // All the nodes in the path.
    std::vector<Relationship> relationships;
};

}  // namespace bolt
