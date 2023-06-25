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
// Created by wt on 18-8-14.
//
#include "antlr4-runtime.h"

#include "core/task_tracker.h"

#include "parser/generated/LcypherLexer.h"
#include "parser/generated/LcypherParser.h"
#include "parser/cypher_base_visitor.h"
#include "parser/cypher_error_listener.h"

#include "execution_plan.h"
#include "scheduler.h"

namespace cypher {

inline fma_common::Logger &Logger() {
    static fma_common::Logger &logger = fma_common::Logger::Get("server.cypher.execution_plan");
    return logger;
}

void Scheduler::Eval(RTContext *ctx, const std::string &script, ElapsedTime &elapsed) {
    using namespace parser;
    using namespace antlr4;
    auto t0 = fma_common::GetTime();
    // <script, execution plan>
    thread_local LRUCacheThreadUnsafe<std::string, std::shared_ptr<ExecutionPlan>> tls_plan_cache;
    std::shared_ptr<ExecutionPlan> plan;
    if (!tls_plan_cache.Get(script, plan)) {
        ANTLRInputStream input(script);
        LcypherLexer lexer(&input);
        CommonTokenStream tokens(&lexer);
        LcypherParser parser(&tokens);
        /* We can set ErrorHandler here.
         * setErrorHandler(std::make_shared<BailErrorStrategy>());
         * add customized ErrorListener  */
        parser.addErrorListener(&CypherErrorListener::INSTANCE);
        CypherBaseVisitor visitor(ctx, parser.oC_Cypher());
        FMA_DBG_STREAM(Logger()) << "-----CLAUSE TO STRING-----";
        for (const auto &sql_query : visitor.GetQuery()) {
            FMA_DBG_STREAM(Logger()) << sql_query.ToString();
        }

        plan = std::make_shared<ExecutionPlan>();
        // 在生成执行计划时获取Schema信息
        if (ctx->graph_.empty()) {
            ctx->ac_db_.reset(nullptr);
            plan->Build(visitor.GetQuery(), visitor.CommandType());
        } else {
            ctx->ac_db_ = std::make_unique<lgraph::AccessControlledDB>(
                ctx->galaxy_->OpenGraph(ctx->user_, ctx->graph_));
            lgraph_api::GraphDB db(ctx->ac_db_.get(), true);
            ctx->txn_ = std::make_unique<lgraph_api::Transaction>(db.CreateReadTxn());
            auto schema_info = ctx->txn_->GetTxn()->GetSchemaInfo();
            plan->SetSchemaInfo(&schema_info);
            plan->Build(visitor.GetQuery(), visitor.CommandType());
        }
        plan->Validate(ctx);
        ctx->txn_.reset(nullptr);
        ctx->ac_db_.reset(nullptr);

        if (visitor.CommandType() == parser::CmdType::EXPLAIN) {
            ctx->result_info_ = std::make_unique<ResultInfo>();
            ctx->result_ = std::make_unique<lgraph::Result>();

            ctx->result_->ResetHeader({{"@plan", lgraph_api::LGraphType::STRING}});
            auto r = ctx->result_->MutableRecord();
            r->Insert("@plan", lgraph::FieldData(plan->DumpPlan(0, false)));
            return;
        }
        FMA_DBG_STREAM(Logger()) << "Plan cache disabled.";
        // FMA_DBG_STREAM(Logger())
        //     << "Miss execution plan cache, build plan for this query.";
    } else {
        // FMA_DBG_STREAM(Logger())
        //     << "Hit execution plan cache.";
    }
    elapsed.t_compile = fma_common::GetTime() - t0;
    plan->DumpGraph();
    plan->DumpPlan(0, false);
    if (!plan->ReadOnly() && ctx->optimistic_) {
        while (1) {
            try {
                plan->Execute(ctx);
                break;
            } catch (lgraph::TxnCommitException &e) {
                FMA_DBG_STREAM(Logger()) << e.what();
            }
        }
    } else {
        plan->Execute(ctx);
    }
    elapsed.t_total = fma_common::GetTime() - t0;
    elapsed.t_exec = elapsed.t_total - elapsed.t_compile;
    if (plan->CommandType() == CmdType::PROFILE) {
        ctx->result_info_ = std::make_unique<ResultInfo>();
        ctx->result_ = std::make_unique<lgraph::Result>();
        ctx->result_->ResetHeader({{"@profile", lgraph_api::LGraphType::STRING}});

        auto r = ctx->result_->MutableRecord();
        r->Insert("@profile", lgraph::FieldData(plan->DumpGraph()));
        return;
    } else {
        /* promote priority of the recent plan
         * OR add the plan to the plan cache.  */
        // tls_plan_cache.Put(script, plan);
        // FMA_DBG_STREAM(Logger())
        //     << "Current Plan Cache (tid" << std::this_thread::get_id() << "):";
        // for (auto &p : tls_plan_cache.List())
        //     FMA_DBG_STREAM(Logger()) << p.first << "\n";
        // return;
    }
}

bool Scheduler::DetermineReadOnly(cypher::RTContext *ctx, const std::string &script,
                                  std::string &name, std::string &type) {
    using namespace parser;
    using namespace antlr4;
    ANTLRInputStream input(script);
    LcypherLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    LcypherParser parser(&tokens);
    /* We can set ErrorHandler here.
     * setErrorHandler(std::make_shared<BailErrorStrategy>());
     * add customized ErrorListener  */
    parser.addErrorListener(&CypherErrorListener::INSTANCE);
    tree::ParseTree *tree = parser.oC_Cypher();
    CypherBaseVisitor visitor = CypherBaseVisitor(ctx, tree);
    for (const auto &sq : visitor.GetQuery()) {
        if (!sq.ReadOnly(name, type)) return false;
    }
    return true;
}

//    ResultSet Scheduler::Eval(
//            const std::shared_ptr<cypher::ExecutionPlan> &plan,
//            lgraph::PluginManager *plugin_manager,
//            lgraph::StateMachine *state_machine,
//            cypher::PARAM_TAB *param_tab,
//            double &elapsed)
//    {
//        // TODO: store script in the plan so we can track the task name
//        lgraph::AutoTaskTracker task_tracker("[CYPHER_WITH_PARAM]");
//        auto t0 = fma_common::GetTime();
//        plan->Reset();
//        plan->SetContext(param_tab, plugin_manager, state_machine);
//        plan->Execute();
//        elapsed = fma_common::GetTime() - t0;
//        plan->DumpGraph();
//        plan->DumpPlan();
//        FMA_DBG_STREAM(Logger()) << "Current Plan Cache (" << _plan_cache.List().size() << "):";
//        for (auto &p : _plan_cache.List()) FMA_DBG_STREAM(Logger()) << p.first << "\n";
//        auto result = plan->GetResultSet();
//        return result;
//    }
}  // namespace cypher
