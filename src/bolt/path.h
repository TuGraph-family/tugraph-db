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
#include <unordered_map>
#include <memory>
#include "bolt/graph.h"

namespace bolt {
// Intermediate representation of part of path
struct RelNode  {
    int64_t id = 0;
    std::string elementId;
    std::string name;
    std::unordered_map<std::string, std::any> props;
};

struct InternalPath {
    std::vector<Node> nodes;
    std::vector<RelNode> rels;
    std::vector<int64_t> indices;
};

// BuildPath builds a path from Bolt representation
inline Path BuildPath(std::vector<Node> nodes, std::vector<RelNode> relNodes,
                      std::vector<int> indexes)  {
    auto num = indexes.size()/ 2;
    if (num == 0) {
        Path path;
        if (!nodes.empty()) {
            // there could be a single, disconnected node
            path.nodes = std::move(nodes);
        }
        return path;
    }
    std::vector<Relationship> rels;
    int i = 0;
    auto n1 = &nodes[0];
    while (num > 0) {
        auto relni = indexes[i];
        i++;
        auto n2i = indexes[i];
        i++;
        num--;
        RelNode* reln;
        bool n1start = false;
        if (relni < 0) {
            reln = &relNodes[(relni*-1)-1];
        } else {
            reln = &relNodes[relni-1];
            n1start = true;
        }
        auto n2 = &nodes[n2i];
        Relationship rel{.id = reln->id, .elementId = reln->elementId,
            .startId = -1,.startElementId = "-1", .endId = -1, .endElementId = "-1",
                         .type = reln->name, .props = reln->props};
        if (n1start) {
            rel.startId = n1->id;
            rel.endId = n2->id;
            rel.startElementId = n1->elementId;
            rel.endElementId = n2->elementId;
        } else {
            rel.startId = n2->id;
            rel.endId = n1->id;
            rel.startElementId = n2->elementId;
            rel.endElementId = n1->elementId;
        }
        rels.push_back(rel);
        n1 = n2;
    }
    Path ret;
    ret.nodes = std::move(nodes);
    ret.relationships = std::move(rels);
    return ret;
}

}  // namespace bolt
