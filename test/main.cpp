/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <typeinfo>
#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "gtest/gtest.h"

int _ut_argc;
char** _ut_argv;
bool _ut_buffer_log = true;
bool _ut_run_benchmarks = false;

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
    _ut_argc = argc;
    _ut_argv = argv;
    return RUN_ALL_TESTS();
}
