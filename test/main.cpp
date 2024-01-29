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

#include <typeinfo>

#include "gtest/gtest.h"
#include "gflags/gflags.h"

#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"
#include "./ut_utils.h"
#include "./ut_types.h"
#include "./graph_factory.h"
#include "tools/lgraph_log.h"

int _ut_argc;
char** _ut_argv;
bool _ut_buffer_log = true;
bool _ut_run_benchmarks = false;
GraphFactory::GRAPH_DATASET_TYPE _ut_graph_dataset_type = GraphFactory::GRAPH_DATASET_TYPE::YAGO;
lgraph::ut::QUERY_TYPE _ut_query_type = lgraph::ut::QUERY_TYPE::GQL;

#ifndef _WIN32
namespace brpc {
DECLARE_bool(usercode_in_pthread);
}
#endif

int main(int argc, char** argv) {
    #ifndef _WIN32
    brpc::FLAGS_usercode_in_pthread = true;
    #endif
    ::testing::InitGoogleTest(&argc, argv);

    fma_common::Configuration query_config;
    query_config.Add(_ut_graph_dataset_type, "dataset", true)
        .Comment(fma_common::StringFormatter::Format(
            "Select the graph dataset, currently only supported under the "
            "--gtest_filter=TestQuery.TestDemo condition. Possible values: {}",
            GraphFactory::get_graph_dataset_types()));
    query_config.Add(_ut_query_type, "query_type", true)
        .Comment(fma_common::StringFormatter::Format(
            "Select the query type, currently only supported under the "
            "--gtest_filter=TestQuery.TestDemo condition. Possible values: {}",
            lgraph::ut::ToString(lgraph::ut::QUERY_TYPE::CYPHER) + ", " +
                lgraph::ut::ToString(lgraph::ut::QUERY_TYPE::GQL)));
    query_config.ExitAfterHelp(false);
    query_config.ParseAndRemove(&argc, &argv);
    query_config.Finalize();

    fma_common::Configuration config;
    unsigned int verbose = 1;
    config.Add(verbose, "verbose", true).Comment("Verbose level");
    config.Add(_ut_buffer_log, "buffer_log", true)
        .Comment("Buffer log in memory and print only on failure.");
    config.Add(_ut_run_benchmarks, "run_benchmarks", true)
        .Comment("Run benchmarks.");
    config.ParseAndRemove(&argc, &argv);
    config.Finalize();

    auto severity_level = lgraph_log::severity_level::ERROR;
    if (verbose == 0) {
        severity_level = lgraph_log::severity_level::ERROR;
    } else if (verbose == 1) {
        severity_level = lgraph_log::severity_level::INFO;
    } else {
        severity_level = lgraph_log::severity_level::DEBUG;
    }
    lgraph_log::LoggerManager::GetInstance().Init("", severity_level);
    _ut_argc = argc;
    _ut_argv = argv;
    auto ret = RUN_ALL_TESTS();
#ifdef __SANITIZE_ADDRESS__
    // For address sanitizer: wait gc thread to release memory object
    fma_common::SleepS(20);
#endif
    return ret;
}
