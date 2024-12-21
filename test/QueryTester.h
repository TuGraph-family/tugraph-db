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


#ifndef TEST_QUERYTESTER_H_  // TEST_QUERYTESTER_H_?
#define TEST_QUERYTESTER_H_

#include <memory>
#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#include "./graph_factory.h"
#include "./antlr4-runtime.h"
#include "geax-front-end/ast/AstNode.h"
#include "geax-front-end/ast/AstDumper.h"
#include "geax-front-end/isogql/GQLResolveCtx.h"
#include "geax-front-end/isogql/GQLAstVisitor.h"
#include "geax-front-end/isogql/parser/AntlrGqlParser.h"

#include "cypher/parser/generated/LcypherLexer.h"
#include "cypher/parser/generated/LcypherParser.h"
#include "cypher/parser/cypher_base_visitor_v2.h"
#include "cypher/parser/cypher_error_listener.h"
#include "cypher/rewriter/GenAnonymousAliasRewriter.h"
#include "cypher/rewriter/MultiPathPatternRewriter.h"
#include "fma-common/file_system.h"
#include "db/galaxy.h"
#include "cypher/rewriter/PushDownFilterAstRewriter.h"
#include "cypher/execution_plan/runtime_context.h"
#include "cypher/execution_plan/execution_plan_v2.h"
#include "lgraph/lgraph_utils.h"
#include "./ut_utils.h"
#include "./ut_config.h"
#include "./ut_types.h"

#ifndef LOGGING_UTILS_H
#define LOGGING_UTILS_H

#include <boost/log/trivial.hpp>
#include <boost/log/utility/setup/common_attributes.hpp>
#include <boost/log/utility/setup/console.hpp>

void InitLogging();

#endif  // LOGGING_UTILS_H


using namespace geax::frontend;
using geax::frontend::GEAXErrorCode;

class QueryTester {
 public:
    QueryTester();
    void RunTestDemo();

 private:
    void set_graph_type(GraphFactory::GRAPH_DATASET_TYPE graph_type);
    void set_query_type(lgraph::ut::QUERY_TYPE query_type);
    void init_db();
    bool test_gql_case(const std::string& gql, std::string& result);
    bool test_cypher_case(const std::string& cypher, std::string& result);
    void test_file(const std::string& file_prefix, bool check_result = true);
    void load_procedure(const std::string& name, const std::string& procedure_source_path,
                        bool read_only = true);
    bool diff_file(const std::string& lef, const std::string& rig);

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
    std::string db_dir_ = "./testdb";
    std::string graph_name_ = "default";
    GraphFactory::GRAPH_DATASET_TYPE graph_type_ = GraphFactory::GRAPH_DATASET_TYPE::YAGO;
    lgraph::ut::QUERY_TYPE query_type_ = lgraph::ut::QUERY_TYPE::GQL;

 protected:
    std::string test_suite_dir_ = lgraph::ut::TEST_RESOURCE_DIRECTORY + "/cases";
};

#endif  // TEST_QUERYTESTER_H_
