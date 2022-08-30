/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

//
// Created by wt on 18-10-31.
//
#pragma once

#include "cypher/parser/data_typedef.h"
#include "cypher/resultset/result_info.h"
#include "lgraph/lgraph_result.h"

namespace cypher {

// input context
class SubmitQueryContext {
 public:
    lgraph::StateMachine *sm_ = nullptr;
    lgraph::Galaxy *galaxy_ = nullptr;
    std::string token_;
    std::string user_;
    std::string graph_;
    PARAM_TAB param_tab_;
    bool optimistic_ = false;
    bool path_unique_ = true;
    lgraph::AclManager::FieldAccess field_access_;

    SubmitQueryContext() = default;

    SubmitQueryContext(lgraph::StateMachine *sm, lgraph::Galaxy *galaxy, const std::string &token,
                       const std::string &user, const std::string &graph,
                       const lgraph::AclManager::FieldAccess &field_access)
        : sm_(sm), galaxy_(galaxy), token_(token),
          user_(user), graph_(graph), field_access_(field_access) {}

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
    std::unique_ptr<lgraph::AccessControlledDB> ac_db_ = nullptr;
    std::unique_ptr<lgraph::Transaction> txn_ = nullptr;
    std::unique_ptr<ResultInfo> result_info_ = nullptr;
    std::unique_ptr<lgraph_api::Result> result_ = nullptr;

    RTContext() = default;

    RTContext(lgraph::StateMachine *sm, lgraph::Galaxy *galaxy, const std::string &token,
              const std::string &user, const std::string &graph,
              const lgraph::AclManager::FieldAccess& field_access)
        : SubmitQueryContext(sm, galaxy, token, user, graph, field_access) {}

    bool Check(std::string &msg) const {
        if (!SubmitQueryContext::Check(msg)) return false;
        if (ac_db_) {
            msg = "Access controlled db not empty";
            return false;
        }
        if (txn_) {
            msg = "Previous transaction not closed.";
            return false;
        }
        return true;
    }
};
}  // namespace cypher
