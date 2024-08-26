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
// Created by gelincheng on 2/17/22.
//

#include <db/galaxy.h>
#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

/* Make sure include graph_factory.h BEFORE antlr4-runtime.h. Otherwise causing the following error:
 * ‘EOF’ was not declared in this scope.
 * For the former (include/butil) uses macro EOF, which is undefined in antlr4.  */
#include "./graph_factory.h"

#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/AstDumper.h"
#include "antlr4-runtime/antlr4-runtime.h"

#include "cypher/execution_plan/execution_plan_v2.h"

#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor_v2.h"
#include "cypher/rewriter/GenAnonymousAliasRewriter.h"
#include "cypher/rewriter/MultiPathPatternRewriter.h"
#include "cypher/rewriter/PushDownFilterAstRewriter.h"

using namespace parser;
using namespace antlr4;

class TestCypherPlan : public TuGraphTest {};

static const cypher::PARAM_TAB g_param_tab = {
    {"$name", cypher::FieldData(lgraph::FieldData("Lindsay Lohan"))},
    {"$personId", cypher::FieldData(lgraph::FieldData(1))},
    {"$personIds", cypher::FieldData(std::vector<lgraph::FieldData>{
                       lgraph::FieldData("Liam Neeson"), lgraph::FieldData("Dennis Quaid"),
                       lgraph::FieldData("Roy Redgrave")})},
};

void eval_query_check(cypher::RTContext *ctx, const std::string &query,
                      const std::string &expect_plan, const int expect_res) {
    UT_LOG() << query;
    ANTLRInputStream input(query);
    LcypherLexer lexer(&input);
    CommonTokenStream tokens(&lexer);
    LcypherParser parser(&tokens);
    geax::common::ObjectArenaAllocator objAlloc_;
    CypherBaseVisitorV2 visitor(objAlloc_, parser.oC_Cypher(), ctx);
    geax::frontend::AstNode *node = visitor.result();
    // rewrite ast
    cypher::GenAnonymousAliasRewriter gen_anonymous_alias_rewriter;
    node->accept(gen_anonymous_alias_rewriter);
    cypher::MultiPathPatternRewriter multi_path_pattern_rewriter(objAlloc_);
    node->accept(multi_path_pattern_rewriter);
    cypher::PushDownFilterAstRewriter push_down_filter_ast_writer(objAlloc_, ctx);
    node->accept(push_down_filter_ast_writer);

    double t0, t1, t2;
    t0 = fma_common::GetTime();

    geax::frontend::AstDumper dumper;
    auto ret = dumper.handle(node);
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        UT_LOG() << "dumper.handle(node) gql: " << query;
        UT_LOG() << "dumper.handle(node) ret: " << ToString(ret);
        UT_LOG() << "dumper.handle(node) error_msg: " << dumper.error_msg();
        return;
    } else {
        UT_DBG() << "--- dumper.handle(node) dump ---";
        UT_DBG() << dumper.dump();
    }
    cypher::ExecutionPlanV2 execution_plan;
    execution_plan.PreValidate(ctx, visitor.GetNodeProperty(), visitor.GetRelProperty());
    ret = execution_plan.Build(node, ctx);
    if (ret != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {
        UT_LOG() << "build execution_plan_v2 failed: " << execution_plan.ErrorMsg();
        return;
    }
    execution_plan.DumpGraph();
    std::string res_plan = execution_plan.DumpPlan(0, false);

    t1 = fma_common::GetTime();
    execution_plan.Execute(ctx);
    t2 = fma_common::GetTime();
    UT_LOG() << "Plan Total Time " << t2 - t0 << "s";
    UT_LOG() << "Plan Execute Time " << t2 - t1 << "s";
    UT_LOG() << t2 - t0 << " " << t2 - t1;
    UT_LOG() << "Result:\n" << ctx->result_->Dump();

    UT_EXPECT_EQ(ctx->result_->Size(), expect_res);
    UT_EXPECT_EQ(res_plan, expect_plan);
}

int test_cypher_plan(const nlohmann::json &conf) {
    std::string dataset;
    std::string expect_plan;
    std::string plan;
    for (auto &el : conf["testCases"]) {
        dataset = el["dataset"];
        if (dataset == "yago") {
            UT_LOG() << "test on dataset:" << dataset;
            GraphFactory::create_yago_with_constraints("./testdb");
            lgraph::Galaxy::Config gconf;
            gconf.dir = "./testdb";
            lgraph::Galaxy galaxy(gconf, true, nullptr);
            cypher::RTContext db(nullptr, &galaxy, lgraph::_detail::DEFAULT_ADMIN_NAME, "default");
            db.param_tab_ = g_param_tab;
            for (auto &test_cases : el["querys"].items()) {
                for (auto &c : test_cases.value()) {
                    eval_query_check(&db, c["query"], c["plan"], c["res"]);
                }
            }
        } else {
            CYPHER_TODO();
        }
    }

    return 0;
}

TEST_F(TestCypherPlan, CypherPlan) {
    std::string validate_file = "../../test/cypher_plan_validate.json";
    int verbose = 1;
    int argc = _ut_argc;
    char **argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(verbose, "v", true).Comment("Verbose: 0-WARNING, 1-INFO, 2-DEBUG");
    config.ExitAfterHelp();
    config.ParseAndFinalize(argc, argv);

    auto severity_level = lgraph_log::severity_level::ERROR;
    if (verbose == 0)
        severity_level = lgraph_log::severity_level::WARNING;
    else if (verbose == 1)
        severity_level = lgraph_log::severity_level::INFO;
    else
        severity_level = lgraph_log::severity_level::DEBUG;
    lgraph_log::LoggerManager::GetInstance().Init("", severity_level);

    std::ifstream ifs(validate_file);
    nlohmann::json conf;
    ifs >> conf;
    test_cypher_plan(conf);
}
