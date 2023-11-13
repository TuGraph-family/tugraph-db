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
// Created by chnlkw on 12/14/17.
//

#include "fma-common/configuration.h"
#include "fma-common/fma_stream.h"
#include "fma-common/logger.h"
#include "fma-common/logging.h"
#include "./unit_test_utils.h"

using namespace fma_common;
using std::string;

int TestArg() {
    std::vector<char *> args = {(char *)"test.exe", (char *)"all", (char *)"12",
                                (char *)"--size",   (char *)"13",  (char *)"--round",
                                (char *)"14"};
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

    using ArgParser = Configuration;
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
    parser.Add<bool>("verbose,v").Comment("Verbose output").SetDefault(false);
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
    return 0;
}

struct ConfChild : public Configuration {
    string b;
    bool f = false;

    ConfChild() {
        Add(b, "b,bob").Comment("parameter b");
        Add(f, "f,flag").Comment("parameter f");
    }
};

struct ConfFather : public Configuration {
    int a;
    ConfChild child;

    ConfFather() {
        Add(a, "a,ada").Comment("parameter a");
        AddChild(&child, "c");
    }
};

FMA_SET_TEST_PARAMS(Configuration, "");

FMA_UNIT_TEST(Configuration) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    Configuration c;
    std::string path;
    c.Add(path, "path", true)
        .Comment("Path of the json config file. If not specified, we will use generated json.");
    c.ParseAndRemove(&argc, &argv);
    c.Finalize();

    Configuration d;
    d.ExitAfterHelp();
    std::string path2;
    int tmp = 0;
    d.Add(tmp, "v,some_really_long_option_really_long", true)
        .Comment("This is a really long option.");
    d.Add(path, "test2", true).Comment("test2");
    try {
        d.Add(path2, "opt1,d", true);
        d.Add(path2, "opt2,d", true);
    } catch (ConfigurationError& err) {
        std::cout << "catched : " << err.what() << std::endl;
    }
    d.ParseAndRemove(&argc, &argv);
    d.Finalize();
    LOG() << d.HelpString();
#if !defined(__GNUC__) || __GNUC__ >= 5  // json.hpp does not support g++ 4
    FMA_LOG() << "Testing ParseJsonFile";
    {
        const std::string &json_path = "./tmp_json.json";
        {
            OutputFmaStream out(json_path);
            std::string json = "{\"a\":1,\"b\":\"this is b\"}";
            out.Write(json.data(), json.size());
        }
        Configuration conf;
        int a = 0;
        std::string b;
        conf.Add(a, "a", true);
        conf.Add(b, "b", true);
        conf.ParseJsonFile(json_path);
        conf.Finalize();
        FMA_UT_CHECK_EQ(a, 1);
        FMA_UT_CHECK_EQ(b, "this is b");
    }

    if (!path.empty()) {
        InputFmaStream in(path);
        FMA_UT_ASSERT(in.Good());
        std::string json, buf;
        buf.reserve(1024);
        while (true) {
            size_t r = in.Read(&buf[0], 1024);
            if (!r) break;
            json.append(&buf[0], r);
        }

        std::string dir;
        size_t db_size = 1;
        bool durable = true;
        bool track_incoming = true;
        int workerCount = 12;
        uint16_t port = 9;

        Configuration config;
        config.Add(dir, "directory", false).Comment("Directory where the db files are stored.");
        config.Add(db_size, "size_GB", true)
            .Comment(
                "Maximum size of the database, in GB."
                " If you set it too small, you may encounter \"DB_FULL\" errors with modify "
                "operations.");
        config.Add(durable, "durable", true)
            .Comment(
                "Whether to make the db durable."
                " When durable is set to true, write transactions are flushed to disk immediately "
                "after commit."
                " This will cause a lot of IOs and cause the write operation to slow down, "
                "especially when you"
                " have a slow disk. On the other hand, if durable is set to false, newly updated "
                "data may be lost"
                " since they are not flushed to disk yet.");
        config.Add(track_incoming, "track_incoming", true)
            .Comment(
                "Whether to track incoming edges in the destination vertex."
                " When this is set to true, each vertex will track its incoming edges, so you can "
                "iterate"
                " its incoming edges with InEdgeIterator. Moreover, when we delete a vertex, all "
                "its incoming"
                " edges can also be deleted automatically. However, this comes with cost: when we "
                "insert/delete"
                " an edge, will will need to update the information contained in both the source "
                "and destination"
                " edges, which will slow down the operation. The tracking information will also "
                "take extra"
                " space in the DB.");
        config.Add(workerCount, "worker_threads", true)
            .Comment("Number of worker threads used in the database.");
        config.Add(port, "rpc_port", true).Comment("Port on which we listen for RPC requests.");
        config.ParseJson(json.c_str());
        config.Parse(argc, argv);
        config.Finalize();
        LOG() << dir;
        LOG() << port;
        LOG() << db_size;
    } else {
        TestArg();
        Configuration conf;
        ConfFather fa;
        fa.Add<int>("b,bro");
        std::string str_opt;
        fa.Add(str_opt, "str");
        char str[] = {R"(
{
    "ada":2,
    "b":3,
    "s":"skipped",
    "str":"some_string",
    "c":{
        "b":4,
        "f":true
    }
}
        )"};
        //    std::cout << str << std::endl;
        //    fa.ParseAndRemove(&argc, &argv);
        fa.ParseJson(str);
        fa.Finalize();
        FMA_UT_CHECK_EQ(fa.a, 2);
        FMA_UT_CHECK_EQ(fa.GetValue<int>("b"), 3);
        FMA_UT_CHECK_EQ(fa.GetValue<int>("bro"), 3);
        FMA_UT_CHECK_EQ(str_opt, "some_string");
        FMA_UT_CHECK_EQ(fa.child.b, "4");  // not a bug. parse integer 4 as a string
        FMA_UT_CHECK_EQ(fa.child.f, true);
    }
#endif
    {
        auto parse_and_check = [&](const std::string &opts, bool expected_bool, int expected_int) {
            std::vector<std::string> ss = Split(opts);
            int argc = (int)ss.size();
            std::vector<char *> argvs;
            for (auto &s : ss) argvs.emplace_back((char *)s.data());
            char **argv = (char **)argvs.data();
            std::atomic<bool> bool_opt(false);
            std::atomic<int> int_opt(100);
            Configuration config;
            config.Add(bool_opt, "b,bool", true).Comment("Bool option");
            config.Add(int_opt, "i,int", true).Comment("IntOption");
            config.ParseAndFinalize(argc, (char **)argv);
            FMA_UT_CHECK_EQ(bool_opt, expected_bool);
            FMA_UT_CHECK_EQ(int_opt, expected_int);
        };
        parse_and_check("-b true -i 200", true, 200);
        parse_and_check("--bool true --int 200", true, 200);
        parse_and_check("--int 200", false, 200);
        parse_and_check("", false, 100);
        // no such option
        FMA_EXPECT_EXCEPTION(parse_and_check("-x 1", true, 100));
        // value not given
        FMA_EXPECT_EXCEPTION(parse_and_check("-i", true, 100));
        // fail to parse value
        FMA_EXPECT_EXCEPTION(parse_and_check("-i a", true, 100));
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
