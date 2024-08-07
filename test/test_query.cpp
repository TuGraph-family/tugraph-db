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

#include <memory>
#include <optional>
#include <string>
#include <tuple>

#include "./graph_factory.h"
/* Make sure include graph_factory.h BEFORE antlr4-runtime.h. Otherwise causing the following error:
 * ‘EOF’ was not declared in this scope.
 * For the former (include/butil) uses macro EOF, which is undefined in antlr4. */
#include "./antlr4-runtime.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/AstDumper.h"
#include "geax-front-end/isogql/GQLResolveCtx.h"
#include "geax-front-end/isogql/GQLAstVisitor.h"
#include "geax-front-end/isogql/parser/AntlrGqlParser.h"

#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor.h"
#include "cypher/parser/cypher_error_listener.h"
#include "cypher/rewriter/GenAnonymousAliasRewriter.h"
#include "fma-common/file_system.h"
#include "db/galaxy.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/execution_plan/execution_plan_v2.h"
#include "lgraph/lgraph_utils.h"
#include "./ut_utils.h"
#include "./ut_config.h"
#include "./ut_types.h"

using namespace geax::frontend;
using geax::frontend::GEAXErrorCode;

extern GraphFactory::GRAPH_DATASET_TYPE _ut_graph_dataset_type;
extern lgraph::ut::QUERY_TYPE _ut_query_type;

class TestQuery : public TuGraphTest {
 private:
    std::shared_ptr<cypher::RTContext> ctx_;
    std::shared_ptr<lgraph::Galaxy> galaxy_;
    lgraph::Galaxy::Config gconf_;
    inline static const std::string TEST_SUFFIX = ".test";
    inline static const std::string REAL_SUFFIX = ".real";
    inline static const std::string RESULT_SUFFIX = ".result";
    inline static const std::string COMMENT_PREFIX = "#";
    inline static const std::string END_LINE_SUFFIX = ";";
    inline static const std::string LOAD_PROCEDURE_CMD_PREFIX = "-- loadProcedure";
    inline static const std::string ERROR_CMD_PREFIX = "-- error";
    inline static const std::string LOAD_PROCEDURE_READ_ONLY = "read_only=true";
    std::string db_dir_ = "./data.tinypace";
    std::string graph_name_ = "test";
    GraphFactory::GRAPH_DATASET_TYPE graph_type_ = GraphFactory::GRAPH_DATASET_TYPE::YAGO;
    lgraph::ut::QUERY_TYPE query_type_ = lgraph::ut::QUERY_TYPE::GQL;

 protected:
    std::string test_suite_dir_ = lgraph::ut::TEST_RESOURCE_DIRECTORY + "/cases";

    void set_graph_type(GraphFactory::GRAPH_DATASET_TYPE graph_type) {
        graph_type_ = graph_type;
    }

    void set_query_type(lgraph::ut::QUERY_TYPE query_type) {
        query_type_ = query_type;
    }

    bool diff_file(const std::string& lef, const std::string& rig) {
        std::string cmd = fma_common::StringFormatter::Format("diff {} {}", lef, rig);
        lgraph::SubProcess diff(cmd, false);
        diff.Wait();
        if (diff.GetExitCode() != 0) {
            UT_LOG() << "-----" << cmd << "-----";
            UT_LOG() << diff.Stdout();
        }
        return diff.GetExitCode() == 0;
    }

    void init_db() {
        ctx_.reset();
        galaxy_.reset();
        GraphFactory::create_graph(graph_type_, db_dir_);
        gconf_.dir = db_dir_;
        galaxy_ = std::make_shared<lgraph::Galaxy>(gconf_, true, nullptr);
        ctx_ = std::make_shared<cypher::RTContext>(
            nullptr, galaxy_.get(),
            lgraph::_detail::DEFAULT_ADMIN_NAME, graph_name_);
    }

    bool test_gql_case(const std::string& gql, std::string& result) {
        if (ctx_ == nullptr) {
            UT_LOG() << "ctx_ is nullptr";
            return false;
        }
        geax::frontend::AntlrGqlParser parser(gql);
        parser::GqlParser::GqlRequestContext* rule = parser.gqlRequest();
        if (!parser.error().empty()) {
            UT_LOG() << "parser.gqlRequest() error: " << parser.error();
            result = parser.error();
            return false;
        }
        geax::common::ObjectArenaAllocator objAlloc_;
        GQLResolveCtx gql_ctx{objAlloc_};
        GQLAstVisitor visitor{gql_ctx};
        rule->accept(&visitor);
        auto ret = visitor.error();
        if (ret != GEAXErrorCode::GEAX_SUCCEED) {
            UT_LOG() << "rule->accept(&visitor) ret: " << ToString(ret);
            result = ToString(ret);
            return false;
        }
        AstNode* node = visitor.result();
        // rewrite ast
        cypher::GenAnonymousAliasRewriter gen_anonymous_alias_rewriter;
        node->accept(gen_anonymous_alias_rewriter);
        // dump
        AstDumper dumper;
        ret = dumper.handle(node);
        if (ret != GEAXErrorCode::GEAX_SUCCEED) {
            UT_LOG() << "dumper.handle(node) gql: " << gql;
            UT_LOG() << "dumper.handle(node) ret: " << ToString(ret);
            UT_LOG() << "dumper.handle(node) error_msg: " << dumper.error_msg();
            result = dumper.error_msg();
            return false;
        } else {
            UT_DBG() << "--- dumper.handle(node) dump ---";
            UT_DBG() << dumper.dump();
        }
        cypher::ExecutionPlanV2 execution_plan_v2;
        ret = execution_plan_v2.Build(node, ctx_.get());
        if (ret != GEAXErrorCode::GEAX_SUCCEED) {
            UT_LOG() << "build execution_plan_v2 failed: " << execution_plan_v2.ErrorMsg();
            result = execution_plan_v2.ErrorMsg();
            return false;
        } else {
            try {
                execution_plan_v2.Execute(ctx_.get());
            } catch (std::exception &e) {
                UT_LOG() << e.what();
                result = e.what();
                return false;
            }
            UT_LOG() << "-----result-----";
            result = ctx_->result_->Dump(false);
            UT_LOG() << result;
        }
        return true;
    }

    bool test_cypher_case(const std::string& cypher, std::string& result) {
        try {
            antlr4::ANTLRInputStream input(cypher);
            parser::LcypherLexer lexer(&input);
            antlr4::CommonTokenStream tokens(&lexer);
            parser::LcypherParser parser(&tokens);
            parser.addErrorListener(&parser::CypherErrorListener::INSTANCE);
            parser::CypherBaseVisitor visitor(ctx_.get(), parser.oC_Cypher());
            LOG_INFO() << "-----CLAUSE TO STRING-----";
            LOG_INFO() << visitor.GetQuery().size();
            for (const auto &sql_query : visitor.GetQuery()) {
                LOG_INFO() << sql_query.ToString();
            }
            cypher::ExecutionPlan execution_plan;
            execution_plan.PreValidate(ctx_.get(), visitor.GetNodeProperty(),
                                       visitor.GetRelProperty());
            execution_plan.Build(visitor.GetQuery(), visitor.CommandType(), ctx_.get());
            execution_plan.Validate(ctx_.get());
            LOG_INFO() << execution_plan.DumpGraph();
            LOG_INFO() << execution_plan.DumpPlan(0, false);
            execution_plan.Execute(ctx_.get());
            // result = ctx_->result_->Dump(false);
            UT_LOG() << "-----result-----";
            // result = ctx_->result_->Dump(false);
            UT_LOG() << result;
            return true;
        } catch (std::exception& e) {
            UT_LOG() << e.what();
            result = e.what();
            return false;
        }
    }

    void test_files(const std::string& dir) {
        fma_common::LocalFileSystem fs;
        for (auto& file : fs.ListFiles(dir)) {
            if (fma_common::EndsWith(file, TEST_SUFFIX)) {
                test_file(file.substr(0, file.size() - TEST_SUFFIX.size()));
            }
        }
    }

    void test_file(const std::string& file_prefix, bool check_result = true) {
        std::string test_file = file_prefix + TEST_SUFFIX;
        std::string result_file = file_prefix + RESULT_SUFFIX;
        std::string real_file = file_prefix + REAL_SUFFIX;
        std::string line, query, result;
        bool is_error = false;
        fma_common::LocalFileSystem fs;
        if (!fs.FileExists(test_file)) {
            UT_ERR() << "test_file not exists: " << test_file;
            UT_EXPECT_TRUE(false);
            return;
        }
        std::ifstream test_file_in(test_file);
        std::ofstream real_file_out(real_file);
        init_db();
        UT_DBG() << "test_file: " << test_file;
        auto test_query_handle_result = [&]() {
            UT_LOG() << "-----" << lgraph::ut::ToString(query_type_) << "-----";
            UT_LOG() << query;
            bool success;
            if (query_type_ == lgraph::ut::QUERY_TYPE::CYPHER) {
                success = test_cypher_case(query, result);
            } else if (query_type_ == lgraph::ut::QUERY_TYPE::GQL) {
                success = test_gql_case(query, result);
            } else {
                LOG_FATAL() << "unhandled query_type_: " << lgraph::ut::ToString(query_type_);
                UT_EXPECT_TRUE(false);
                return;
            }
            if (!success && !is_error) {
                UT_EXPECT_TRUE(false);
            }
            real_file_out << query << std::endl;
            real_file_out << result << std::endl;
            query.clear();
            is_error = false;
        };
        while (std::getline(test_file_in, line)) {
            std::string line_t = fma_common::Strip(line, ' ');
            bool start_with_comment_prefix = fma_common::StartsWith(line_t, COMMENT_PREFIX);
            if (start_with_comment_prefix || line_t.empty()) {
                real_file_out << line << std::endl;
                continue;
            }
            if (fma_common::StartsWith(line_t, ERROR_CMD_PREFIX)) {
                real_file_out << line << std::endl;
                is_error = true;
                continue;
            } else if (fma_common::StartsWith(line_t, LOAD_PROCEDURE_CMD_PREFIX)) {
                // Load stored procedure
                // Input format: -- loadProcedure name procedure_source_path [read_only=true]
                // The default value for read_only is true.
                real_file_out << line << std::endl;
                auto args = fma_common::Split(line_t, " ");
                if (args.size() / 2 == 2 && !args[2].empty() && !args[3].empty()) {
                    load_procedure(args[2], args[3],
                                   args.size() != 5 || args[4] == LOAD_PROCEDURE_READ_ONLY);
                    continue;
                }
                UT_EXPECT_TRUE(false);
            }
            if (!query.empty()) {
                query += "\n";
            }
            query += line;
            if (fma_common::EndsWith(line_t, END_LINE_SUFFIX)) {
                test_query_handle_result();
            }
        }
        if (!query.empty()) {
            test_query_handle_result();
        }
        test_file_in.close();
        real_file_out.close();
        if (!check_result) {
            return;
        }
        if (diff_file(real_file, result_file)) {
            fma_common::LocalFileSystem fs;
            fs.Remove(real_file);
        } else {
            UT_EXPECT_TRUE(false);
        }
    }

    void load_procedure(const std::string& name, const std::string& procedure_source_path,
                        bool read_only = true) {
        std::ifstream f;
        f.open(procedure_source_path, std::ios::in);
        std::string buf;
        std::string text = "";
        while (getline(f, buf)) {
            text += buf;
            text += "\n";
        }
        f.close();
        std::string encoded = lgraph_api::encode_base64(text);
        std::string result;
        std::string procedure_version = "v1";
        UT_EXPECT_TRUE(test_cypher_case(
            FMA_FMT("CALL db.plugin.loadPlugin('CPP','{}','{}','CPP','{}', {}, '{}')", name,
                    encoded, name, read_only ? "true" : "false", procedure_version),
            result));
        return;
    }
};

TEST_F(TestQuery, TestGqlParserDemo) {
    // MATCH (n:Person WHERE n.id = 3) RETURN n.name
    std::string isogql = "MATCH (n:Person WHERE n.id = 3) RETURN n.name";
    geax::frontend::AntlrGqlParser parser(isogql);
    parser::GqlParser::GqlRequestContext* rule = parser.gqlRequest();
    geax::common::ObjectArenaAllocator objAlloc_;
    GQLResolveCtx ctx{objAlloc_};
    GQLAstVisitor visitor{ctx};
    rule->accept(&visitor);
    auto ret = visitor.error();
    if (ret != GEAXErrorCode::GEAX_SUCCEED) {
        UT_DBG() << "rule->accept(&visitor) ret: " << ToString(ret);
    }
    UT_EXPECT_TRUE(ret == GEAXErrorCode::GEAX_SUCCEED);
    AstNode* node = visitor.result();
    AstDumper dumper;
    ret = dumper.handle(node);
    if (ret != GEAXErrorCode::GEAX_SUCCEED) {
        UT_DBG() << "dumper.handle(node) gql: " << isogql;
        UT_DBG() << "dumper.handle(node) ret: " << ToString(ret);
        UT_DBG() << "dumper.handle(node) error_msg: " << dumper.error_msg();
    } else {
        UT_DBG() << "--- dumper.handle(node) dump ---";
        UT_DBG() << dumper.dump();
    }
    UT_EXPECT_TRUE(ret == GEAXErrorCode::GEAX_SUCCEED);
}

TEST_F(TestQuery, TestDemo) {
    set_graph_type(_ut_graph_dataset_type);
    set_query_type(_ut_query_type);
    std::string dir = test_suite_dir_ + "/demo";
    test_file(dir, false);
}

TEST_F(TestQuery, TestGqlSuite) {
    set_graph_type(GraphFactory::GRAPH_DATASET_TYPE::YAGO);
    set_query_type(lgraph::ut::QUERY_TYPE::GQL);
    std::string dir = test_suite_dir_ + "/suite/gql";
    test_files(dir);
}

TEST_F(TestQuery, TestCypherSuite) {
    set_graph_type(GraphFactory::GRAPH_DATASET_TYPE::YAGO);
    set_query_type(lgraph::ut::QUERY_TYPE::CYPHER);
    std::string dir = test_suite_dir_ + "/suite/cypher";
    test_files(dir);
}

TEST_F(TestQuery, TestGqlFinbench) {
    set_graph_type(GraphFactory::GRAPH_DATASET_TYPE::MINI_FINBENCH);
    set_query_type(lgraph::ut::QUERY_TYPE::GQL);
    std::string dir = test_suite_dir_ + "/finbench/gql";
    test_files(dir);
}

TEST_F(TestQuery, TestGqlSNB) {
    set_graph_type(GraphFactory::GRAPH_DATASET_TYPE::MINI_SNB);
    set_query_type(lgraph::ut::QUERY_TYPE::GQL);
    std::string dir = test_suite_dir_ + "/snb/gql";
    test_files(dir);
}

TEST_F(TestQuery, TestCypherFinbench) {
    set_graph_type(GraphFactory::GRAPH_DATASET_TYPE::MINI_FINBENCH);
    set_query_type(lgraph::ut::QUERY_TYPE::CYPHER);
    std::string dir = test_suite_dir_ + "/finbench/cypher";
    lgraph::AccessControlledDB::SetEnablePlugin(true);
    test_files(dir);
}
