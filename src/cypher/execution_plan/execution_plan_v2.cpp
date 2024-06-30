/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include "db/galaxy.h"
#include "cypher/execution_plan/execution_plan_maker.h"
#include "cypher/execution_plan/pattern_graph_maker.h"
#include "cypher/execution_plan/optimization/locate_node_by_indexed_prop.h"
#include "cypher/execution_plan/execution_plan_v2.h"
#include "cypher/execution_plan/clause_read_only_decider.h"

namespace cypher {

ExecutionPlanV2::~ExecutionPlanV2() { OpBase::FreeStream(root_); }

geax::frontend::GEAXErrorCode ExecutionPlanV2::Build(geax::frontend::AstNode* astNode,
                                                     RTContext* ctx) {
    geax::frontend::GEAXErrorCode ret = geax::frontend::GEAXErrorCode::GEAX_SUCCEED;
    // build pattern graph
    PatternGraphMaker pattern_graph_maker(pattern_graphs_);
    ret = pattern_graph_maker.Build(astNode, ctx);
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        error_msg_ = pattern_graph_maker.ErrorMsg();
        return ret;
    }
    LOG_DEBUG() << DumpGraph();
    // build execution plan
    ExecutionPlanMaker execution_plan_maker(pattern_graphs_, obj_alloc_);
    ret = execution_plan_maker.Build(astNode, root_);
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        error_msg_ = execution_plan_maker.ErrorMsg();
        return ret;
    }
    result_info_ = execution_plan_maker.GetResultInfo();
    LOG_DEBUG() << DumpPlan(0, false);
    // optimize
    LocateNodeByIndexedProp locate_node_by_indexed_prop;
    locate_node_by_indexed_prop.Execute(Root());
    LOG_DEBUG() << DumpPlan(0, false);

    ClauseReadOnlyDecider decider;
    ret = decider.Build(astNode);
    read_only_ = decider.IsReadOnly();
    return ret;
}

int ExecutionPlanV2::Execute(RTContext* ctx) {
    // check input
    std::string msg;
#ifndef NDEBUG
    std::thread::id entry_id = std::this_thread::get_id();  // check if tid changes in this function
#endif
    if (!ctx->Check(msg)) throw lgraph::CypherException(msg);
    // instantiate db, transaction
    size_t memory_limit = ctx->galaxy_->GetUserMemoryLimit(ctx->user_, ctx->user_);
    AllocatorManager.BindTid(ctx->user_);
    AllocatorManager.SetMemoryLimit(memory_limit);
    if (ctx->graph_.empty()) {
        ctx->ac_db_.reset(nullptr);
    } else {
        ctx->ac_db_ = std::make_unique<lgraph::AccessControlledDB>(
            ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_));
        lgraph_api::GraphDB db(ctx->ac_db_.get(), ReadOnly());
        if (ReadOnly()) {
            ctx->txn_ = std::make_unique<lgraph_api::Transaction>(db.CreateReadTxn());
        } else {
            ctx->txn_ =
                std::make_unique<lgraph_api::Transaction>(db.CreateWriteTxn(ctx->optimistic_));
        }
    }
    ctx->result_info_ = std::make_unique<ResultInfo>(result_info_);
    std::vector<std::pair<std::string, lgraph_api::LGraphType>> header;
    for (auto& h : ctx->result_info_->header.colums) {
        std::pair<std::string, lgraph_api::LGraphType> column;
        column.first = h.alias.empty() ? h.name : h.alias;
        column.second = h.type;
        header.emplace_back(column);
    }
    ctx->result_ = std::make_unique<lgraph_api::Result>(lgraph_api::Result(header));
    try {
        OpBase::OpResult res;
        do {
            res = root_->Consume(ctx);
#ifndef NDEBUG
            LOG_DEBUG() << "root op result: " << res << " (" << OpBase::OP_OK << " for OK)";
#endif
        } while (res == OpBase::OP_OK);
        Reset();
    } catch (std::exception& e) {
        // clear ctx
        ctx->txn_.reset(nullptr);
        ctx->ac_db_.reset(nullptr);
        throw;
    }
    // finialize, clean db, transaction
    if (ctx->txn_) {
        if (!ReadOnly() && ctx->optimistic_) {
            try {
                ctx->txn_->Commit();
            } catch (std::exception& e) {
                ctx->txn_ = nullptr;
                std::string err = "Optimistic transaction commit failed: ";
                err.append(e.what());
                throw lgraph::TxnCommitException(err);
            }
        } else {
            ctx->txn_->Commit();
        }
    }
    // clear ctx
    ctx->txn_.reset(nullptr);
    ctx->ac_db_.reset(nullptr);
#ifndef NDEBUG
    std::thread::id out_id = std::this_thread::get_id();  // check if tid changes in this function
    if (entry_id != out_id) LOG_DEBUG() << "switch thread from: " << entry_id << " to " << out_id;
#endif
    return 0;
}

std::string ExecutionPlanV2::DumpPlan(int indent, bool statistics) const {
    std::string s = statistics ? "Profile statistics:\n" : "Execution Plan:\n";
    OpBase::DumpStream(root_, indent, statistics, s);
    return s;
}

std::string ExecutionPlanV2::DumpGraph() const {
    std::string s;
    for (auto& g : pattern_graphs_) s.append(g.DumpGraph());
    return s;
}

std::string ExecutionPlanV2::ErrorMsg() { return error_msg_; }

OpBase* ExecutionPlanV2::Root() { return root_; }

bool ExecutionPlanV2::ReadOnly() const { return read_only_; }

void ExecutionPlanV2::Reset() {}

}  // namespace cypher
