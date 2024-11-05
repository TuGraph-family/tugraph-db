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

#include "cypher/resultset/record.h"

namespace cypher {

Value Entry::GetEntityField(RTContext *ctx, const std::string &fd) {
    switch (type) {
        case NODE: {
            if (!node->vertex_) {
                return {};
            }
            return node->vertex_->GetProperty(fd);
        }
        case RELATIONSHIP: {
            if (!relationship->edge_) {
                return {};
            }
            return relationship->edge_->GetProperty(fd);
        }
        case CONSTANT: {
            if (!constant.IsMap()) {
                THROW_CODE(CypherException, "Only support for map type");
            }
            const auto& map = constant.AsMap();
            auto it = map.find(fd);
            if (it == map.end()) {
                THROW_CODE(CypherException, "Not found for GetEntityField");
            }
            return it->second;
        }
        case NODE_SNAPSHOT:
        case RELP_SNAPSHOT:
        default:
            CYPHER_TODO();
    }
}

std::string Entry::UUID() {
    switch (type) {
        case UNKNOWN: {
            return "unknown";
        }
        case CONSTANT: {
            return "C" + constant.ToString();
        }
        case NODE: {
            return "N" + std::to_string(node->vertex_->GetId());
        }
        case RELATIONSHIP: {
            return "R" + std::to_string(relationship->edge_->GetId());
        }
        case PATH: {
            CYPHER_TODO();
        }
        default:
            CYPHER_TODO();
    }
}

std::string Record::UUID() {
    std::string uuid;
    for (auto& v : values) {
        uuid.append(v.UUID());
    }
    return uuid;
}

}  // namespace cypher
