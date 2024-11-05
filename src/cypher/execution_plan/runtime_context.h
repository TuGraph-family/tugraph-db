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
// Created by wt on 18-10-31.
//
#pragma once

#include "cypher/parser/data_typedef.h"
#include "cypher/resultset/result_info.h"
#include "server/galaxy.h"
#include "transaction/transaction.h"
#include "geax-front-end/common/ObjectAllocator.h"

namespace cypher {

// input context
class SubmitQueryContext {
 public:
    server::Galaxy *galaxy_ = nullptr;
    std::string user_;
    std::string graph_;
    bool path_unique_ = true;

    SubmitQueryContext() = default;

    SubmitQueryContext(server::Galaxy *galaxy,
                       const std::string &user, const std::string &graph)
        : galaxy_(galaxy), user_(user), graph_(graph) {}
};

// runtime context of execution plan
class RTContext : public SubmitQueryContext {
 public:
    // generated context while plan execution
    graphdb::GraphDB* graph_db_;
    txn::Transaction* txn_;
    std::unique_ptr<ResultInfo> result_info_;
    std::unordered_map<std::string, geax::frontend::Expr*> bolt_parameters_;
    geax::common::ObjectArenaAllocator obj_alloc_;

    RTContext() = default;

    RTContext(server::Galaxy *galaxy,
              const std::string &user, const std::string &graph)
        : SubmitQueryContext(galaxy, user, graph) {}

};
}  // namespace cypher
