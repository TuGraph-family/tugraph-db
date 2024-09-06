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
#include "lgraph/lgraph_result.h"
#include "bolt/connection.h"
#include "geax-front-end/common/ObjectAllocator.h"

namespace cypher {

// input context
class SubmitQueryContext {
 public:
    lgraph::StateMachine *sm_ = nullptr;
    lgraph::Galaxy *galaxy_ = nullptr;
    std::string user_;
    std::string graph_;
    std::shared_ptr<std::unordered_map<std::string, geax::frontend::Expr*>> bolt_parameters_v2_;
    std::shared_ptr<std::unordered_map<std::string, parser::Expression>> bolt_parameters_;
    geax::common::ObjectArenaAllocator obj_alloc_;
    PARAM_TAB param_tab_;
    bool optimistic_ = false;
    bool path_unique_ = true;
    bool is_cypher_v2_ = true;

    SubmitQueryContext() = default;

    SubmitQueryContext(lgraph::StateMachine *sm, lgraph::Galaxy *galaxy,
                       const std::string &user, const std::string &graph,
                       bool is_cypher_v2 = true)
        : sm_(sm), galaxy_(galaxy), user_(user), graph_(graph),
          is_cypher_v2_(is_cypher_v2) {}

    bool Check(std::string &msg) const {
        if (!galaxy_) {
            // graph_ can be empty, eg. call dbms.graph.listGraphs()
            msg = "Invalid plan input context";
            return false;
        }
        return true;
    }
};

// runtime context of execution plan
class RTContext : public SubmitQueryContext {
 public:
    // generated context while plan execution
    std::unique_ptr<lgraph::AccessControlledDB> ac_db_;
    std::unique_ptr<lgraph_api::Transaction> txn_;
    std::unique_ptr<ResultInfo> result_info_;
    std::unique_ptr<lgraph_api::Result> result_;
    bolt::BoltConnection* bolt_conn_ = nullptr;

    RTContext() = default;

    RTContext(lgraph::StateMachine *sm, lgraph::Galaxy *galaxy,
              const std::string &user, const std::string &graph,
              bool is_cypher_v2 = true)
        : SubmitQueryContext(sm, galaxy, user, graph, is_cypher_v2) {}

    bool Check(std::string &msg) const {
        if (!SubmitQueryContext::Check(msg)) return false;
        // We removed the check for the existence of ac_db_ during execution plan execution
        // since ac_db_ is already created during the execution plan optimization phase
        if (txn_) {
            msg = "Previous transaction not closed.";
            return false;
        }
        return true;
    }

    void SetBoltConnection(bolt::BoltConnection* c) {
        bolt_conn_ = c;
    }
};
}  // namespace cypher
