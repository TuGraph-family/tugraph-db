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

#include <map>
#include <string>
#include <vector>

#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "fma-common/logging.h"
#include "fma-common/type_traits.h"
#include "fma-common/utils.h"
#include "./unit_test_utils.h"

using namespace fma_common;

int RunTest(const char *argv0, const char *args) {
    return ExecCmd(std::string(argv0) + " " + args, false);
}

/*!
 * \fn  int TestAll(int argc, char** argv)
 *
 * \brief   Run all tests one by one
 *
 * \param           argc    The argc.
 * \param [in,out]  argv    The argv.
 *
 * \return  An int.
 */
int TestAll(int argc, char **argv) {
    for (auto &test : ::fma_common::GetUnitTests()) {
        for (auto &params : test.second.params) {
            FMA_LOG() << "------------------------------------------";
            FMA_LOG() << "Testing " << test.first << " " << params;
            std::string cmd = argv[0];
            cmd.append(" -t ").append(test.first);
            FMA_UT_CHECK_EQ(RunTest(cmd.c_str(), params.c_str()), 0);
            FMA_LOG() << "TEST PASSED";
        }
    }
    FMA_LOG() << "---------------------";
    FMA_LOG() << "ALL PASSED";
    return 0;
}

int main(int argc, char **argv) {
    auto tests = ::fma_common::GetUnitTests();
    tests["all"].func = TestAll;
    std::vector<std::string> test_strs;
    for (auto &kv : tests) {
        test_strs.push_back(kv.first);
    }

    std::string test;
    Configuration config;
    config.Add(test, "t").Comment("Select a test to run").SetPossibleValues(test_strs);
    config.ParseAndRemove(&argc, &argv);
    config.Finalize();
    lgraph_log::LoggerManager::GetInstance().Init("");
    return tests[test].func(argc, argv);
}
