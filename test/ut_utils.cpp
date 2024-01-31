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

#include "fma-common/fma_stream.h"
#include "../test/ut_utils.h"

void TuGraphTest::setup() {
    if (_ut_buffer_log) {
        lgraph_log::LoggerManager::GetInstance().EnableBufferMode();
    }
}

void TuGraphTest::teardown() {
    if (_ut_buffer_log) {
        const ::testing::TestInfo* const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        if (test_info->result()->Failed()) {
            lgraph_log::LoggerManager::GetInstance().FlushBufferLog();
        }
        lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    }
}

void TuGraphTest::SetUp() { setup(); }

void TuGraphTest::TearDown() { teardown(); }

void build_so(const std::string& so_name, const std::string& so_path) {
    const std::string INCLUDE_DIR = "../../include";
    const std::string DEPS_INCLUDE_DIR = "../../deps/install/include";
    const std::string LIBLGRAPH = "./liblgraph.so";
    int rt;
#ifndef __clang__
    std::string cmd_f =
        "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#elif __APPLE__
    std::string cmd_f =
        "clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -Xpreprocessor "
        "-fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#else
    std::string cmd_f =
        "clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#endif
    std::string cmd;
    cmd = FMA_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, so_name,
                  so_path, LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
}

bool HasElement(const nlohmann::json& val, const std::string& value, const std::string& field) {
    if (val.is_array()) {
        for (const auto & i : val) {
            if (i.contains(field)) {
                if (i.at(field).get<std::string>() == value) {
                    return true;
                }
            }
        }
    } else if (val.is_object()) {
        if (!val.contains(field)) return false;
        if (val.at(field).get<std::string>() == value) {
            return true;
        }
    }
    return false;
}
