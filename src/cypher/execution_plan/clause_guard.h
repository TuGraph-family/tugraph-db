/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include <unordered_set>
#include "geax-front-end/ast/AstNode.h"

namespace cypher {
class ClauseGuard {
 public:
    ClauseGuard(geax::frontend::AstNodeType type,
                std::unordered_set<geax::frontend::AstNodeType>& cur_types)
        : type_(type), cur_types_(cur_types) {
        cur_types.emplace(type_);
    }

    ~ClauseGuard() { cur_types_.erase(type_); }

    static bool InClause(geax::frontend::AstNodeType type,
                         const std::unordered_set<geax::frontend::AstNodeType>& cur_types) {
        return cur_types.find(type) != cur_types.end();
    }

 private:
    geax::frontend::AstNodeType type_;
    std::unordered_set<geax::frontend::AstNodeType>& cur_types_;
};
}  // namespace cypher
