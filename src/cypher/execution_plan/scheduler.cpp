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
#include "./antlr4-runtime.h"
#include "fma-common/logger.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/AstDumper.h"
#include "geax-front-end/isogql/GQLResolveCtx.h"
#include "geax-front-end/isogql/GQLAstVisitor.h"
#include "geax-front-end/isogql/parser/AntlrGqlParser.h"

#include "core/task_tracker.h"

#include "parser/generated/LcypherLexer.h"
#include "parser/generated/LcypherParser.h"
#include "parser/cypher_base_visitor.h"
#include "parser/cypher_error_listener.h"

#include "cypher/execution_plan/execution_plan.h"
#include "cypher/execution_plan/scheduler.h"
#include "cypher/execution_plan/execution_plan_v2.h"
#include "cypher/rewriter/GenAnonymousAliasRewriter.h"

namespace cypher {

inline fma_common::Logger &Logger() {
    static fma_common::Logger &logger = fma_common::Logger::Get("server.cypher.execution_plan");
    return logger;
}

void Scheduler::Eval(RTContext *ctx, const lgraph_api::GraphQueryType &type,
                     const std::string &script, ElapsedTime &elapsed) {
    if (type == lgraph_api::GraphQueryType::CYPHER) {
        EvalCypher(ctx, script, elapsed);
    } else {
        EvalGql(ctx, script, elapsed);
    }
}

bool Scheduler::DetermineReadOnly(cypher::RTContext *ctx,
                                  const lgraph_api::GraphQueryType &query_type,
                                  const std::string &script, std::string &name, std::string &type) {
    if (query_type == lgraph_api::GraphQueryType::CYPHER) {
        return DetermineCypherReadOnly(ctx, script, name, type);
    } else {
        return DetermineGqlReadOnly(ctx, script, name, type);
    }
}

void Scheduler::EvalCypher(RTContext *ctx, const std::string &script, ElapsedTime &elapsed) {
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
        plan->Build(visitor.GetQuery(), visitor.CommandType(), ctx);
        plan->Validate(ctx);
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

void Scheduler::EvalGql(RTContext *ctx, const std::string &script, ElapsedTime &elapsed) {
    using geax::frontend::GEAXErrorCode;
    auto t0 = fma_common::GetTime();
    std::string result;
    geax::frontend::AntlrGqlParser parser(script);
    parser::GqlParser::GqlRequestContext *rule = parser.gqlRequest();
    if (!parser.error().empty()) {
        FMA_DBG() << "parser.gqlRequest() error: " << parser.error();
        result = parser.error();
        throw lgraph::ParserException(result);
    }
    geax::common::ObjectArenaAllocator objAlloc_;
    geax::frontend::GQLResolveCtx gql_ctx{objAlloc_};
    geax::frontend::GQLAstVisitor visitor{gql_ctx};
    rule->accept(&visitor);
    auto ret = visitor.error();
    if (ret != GEAXErrorCode::GEAX_SUCCEED) {
        FMA_DBG() << "rule->accept(&visitor) ret: " << ToString(ret);
        result = ToString(ret);
        throw lgraph::GqlException(result);
    }
    geax::frontend::AstNode *node = visitor.result();
    // rewrite ast
    cypher::GenAnonymousAliasRewriter gen_anonymous_alias_rewriter;
    node->accept(gen_anonymous_alias_rewriter);
    // dump
    geax::frontend::AstDumper dumper;
    ret = dumper.handle(node);
    if (ret != GEAXErrorCode::GEAX_SUCCEED) {
        FMA_DBG() << "dumper.handle(node) gql: " << script;
        FMA_DBG() << "dumper.handle(node) ret: " << ToString(ret);
        FMA_DBG() << "dumper.handle(node) error_msg: " << dumper.error_msg();
        result = dumper.error_msg();
        throw lgraph::GqlException(result);
    } else {
        FMA_DBG() << "--- dumper.handle(node) dump ---";
        FMA_DBG() << dumper.dump();
    }
    cypher::ExecutionPlanV2 execution_plan_v2;
    ret = execution_plan_v2.Build(node);
    elapsed.t_compile = fma_common::GetTime() - t0;
    if (ret != GEAXErrorCode::GEAX_SUCCEED) {
        FMA_DBG() << "build execution_plan_v2 failed: " << execution_plan_v2.ErrorMsg();
        result = execution_plan_v2.ErrorMsg();
        throw lgraph::GqlException(result);
    } else {
        execution_plan_v2.Execute(ctx);
        FMA_DBG() << "-----result-----";
        result = ctx->result_->Dump(false);
        FMA_DBG() << result;
        elapsed.t_total = fma_common::GetTime() - t0;
        elapsed.t_exec = elapsed.t_total - elapsed.t_compile;
    }
}

bool Scheduler::DetermineCypherReadOnly(cypher::RTContext *ctx, const std::string &script,
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

bool Scheduler::DetermineGqlReadOnly(cypher::RTContext *ctx, const std::string &script,
                                     std::string &name, std::string &type) {
    return true;
}

}  // namespace cypher
