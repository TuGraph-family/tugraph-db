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

#include <typeinfo>
#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "gtest/gtest.h"
#include "gflags/gflags.h"

int _ut_argc;
char** _ut_argv;
bool _ut_buffer_log = true;
bool _ut_run_benchmarks = false;

#ifndef _WIN32
namespace brpc {
DECLARE_bool(usercode_in_pthread);
bool brpc::FLAGS_usercode_in_pthread = true;
}
#endif

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    fma_common::Configuration config;
    unsigned int verbose = 1;
    config.Add(verbose, "verbose", true).Comment("Verbose level");
    config.Add(_ut_buffer_log, "buffer_log", true)
        .Comment("Buffer log in memory and print only on failure.");
    config.Add(_ut_run_benchmarks, "run_benchmarks", true)
        .Comment("Run benchmarks.");
    config.ParseAndRemove(&argc, &argv);
    config.Finalize();
    auto level = fma_common::LogLevel::LL_ERROR;
    if (verbose == 0)
        level = fma_common::LogLevel::LL_ERROR;
    else if (verbose == 1)
        level = fma_common::LogLevel::LL_INFO;
    else
        level = fma_common::LogLevel::LL_DEBUG;
    fma_common::Logger::Get().SetLevel(level);
    fma_common::Logger::Get().SetFormatter(std::make_shared<fma_common::TimedLogFormatter>());
    _ut_argc = argc;
    _ut_argv = argv;
    return RUN_ALL_TESTS();
}
