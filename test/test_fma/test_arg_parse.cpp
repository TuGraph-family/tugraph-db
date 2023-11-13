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

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

FMA_SET_TEST_PARAMS(ArgParser, "");

FMA_UNIT_TEST(ArgParser) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    std::vector<char *> args = {(char *)"test.exe", (char *)"all",       (char *)"12",
                                (char *)"--size",   (char *)"13",        (char *)"--round",
                                (char *)"14",       (char *)"--verbose", (char *)"false"};
    int my_argc = (int)args.size();
    char **my_argv = args.data();

#if 0
    // This should fail with FMA_UT_ASSERT,
    // since Arg1 cannot have default: it is not the last positional
    ArgParser parser;
    parser.AddPositional<std::string>()
        .Comment("Arg at position 1")
        .SetDefault("hello");
    parser.AddPositional<std::string>()
        .Comment("Arg at position 2")
        .SetDefault("world");
    parser.Parse((int)args.size(), args.data());
#endif

    ArgParser parser;
    // Positional options:
    //   1. Appears in the front of the arguments,
    //      wherever they are declared
    //   2. Index starts from 1 and continues
    //   3. Only the last option can have default value
    //   4. Can have different types
    parser.Add<std::string>()
        .Comment("Test to perform")
        .SetPossibleValues({"positional", "binary", "keyed", "all"})
        // SetPreserve(true) enables parser to keep this argument
        // after ParseAndRemove(). By default, options are removed
        // if they are matched.
        .SetPreserve(true);
    // Keyed options:
    //   1. Can have different types
    //   2. Have a long key and a short key
    //   3. Long key starts with --, and short key starts with -
    parser.Add<int>("round,n")
        .Comment("Number of tests to perform")
        .SetMin(1)
        .SetMax(1024)
        .SetDefault(1);
    // Binary options:
    //   1. False by default
    //   2. Turned on if the option appears in command line
    parser.Add<bool>("verbose,v").Comment("Verbose output").SetDefault(true);
    // Positional options start from 1 and continues
    // before any keyed or binary options, no matter
    // where they are declared.
    parser.Add<int>().Comment("A number in arg2").SetDefault(1);
    // ParseAndRemove will remove any options that are matched,
    // unless it is declared with SetPreserve(true).
    // It does not give warning if some options are not matched.
    parser.ParseAndRemove(&my_argc, &my_argv);
    parser.Finalize();

    std::string test = parser.GetValue(1);
    FMA_UT_CHECK_EQ(test, "all");
    int number = parser.GetValue<int>(2);
    FMA_UT_CHECK_EQ(number, 12);
    int n_round = parser.GetValue<int>("round");
    FMA_UT_CHECK_EQ(n_round, 14);
    bool verbose = parser.GetValue<bool>("verbose");
    FMA_UT_CHECK_EQ(verbose, false);

    ArgParser parser2;
    parser2.Add<std::string>().Comment("Test to perform");
    parser2.Add<int>("size,s").Comment("Size of xxx");
    // Parse:
    //   1. Gives warnings if some options are not matched
    //   2. Will not remove any matched options
    parser2.Parse(my_argc, my_argv);
    parser2.Finalize();
    std::string test2 = parser2.GetValue(1);
    FMA_UT_CHECK_EQ(test2, "all");
    int size = parser2.GetValue<int>("size");
    FMA_UT_CHECK_EQ(size, 13);

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
