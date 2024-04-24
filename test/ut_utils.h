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

#pragma once
#include <exception>
#include <regex>
#include <string>
#include "tools/lgraph_log.h"
#include "gtest/gtest.h"
#include "tools/json.hpp"

extern int _ut_argc;    // left-over arguments after parsing gtest flags, set in main()
extern char** _ut_argv;
extern bool _ut_buffer_log;  // buffer log in memory and only print on failure
extern bool _ut_run_benchmarks;  // whether to run benchmarks

#define UT_EXPECT_THROW(statement, exception) EXPECT_THROW(statement, exception)

#define UT_EXPECT_THROW_CODE(statement, error_code)                     \
    {                                                                   \
        try {                                                           \
            statement;                                                  \
            FAIL() << "Expecting exception, but nothing is thrown.";    \
        } catch (lgraph_api::LgraphException &e) {                      \
            if (e.code() != lgraph_api::ErrorCode::error_code) {        \
                FAIL() << "Unexpected exception message: " << e.what(); \
            } else {                                                    \
                SUCCEED() << "Expected exception: " << e.what();        \
            }                                                           \
        } catch (std::exception &e) {                                   \
            FAIL() << "Unexpected exception message: " << e.what();     \
        }                                                               \
    }

#define UT_EXPECT_THROW_MSG(statement, msg)                         \
    {                                                               \
        try {                                                       \
            statement;                                              \
            FAIL() <<                                               \
                "Expecting exception, but nothing is thrown.";      \
        } catch (std::exception &e) {                               \
            std::string src = e.what();                             \
            if (src.find(msg) != src.npos) {                        \
                SUCCEED() << "Expected exception: " << e.what();    \
            } else {                                                \
                FAIL() << "Unexpected exception message: "          \
                    << e.what() << "\n"                             \
                    << "expected message: " << msg;                 \
            }                                                       \
        }                                                           \
    }

#define UT_EXPECT_THROW_REGEX(statement, msg)                       \
    {                                                               \
        try {                                                       \
            statement;                                              \
            FAIL() <<                                               \
                "Expecting exception, but nothing is thrown.";      \
        } catch (std::exception &e) {                               \
            if (!std::regex_search(e.what(), std::regex(msg))) {    \
                FAIL() << "Unexpected exception message: "          \
                    << e.what() << "\n"                             \
                    << "expected pattern: " << msg;                 \
            }  else {                                               \
                SUCCEED() << "Expected exception: " << e.what();    \
            }                                                       \
        }                                                           \
    }

#define UT_LOG() LOG_INFO()
#define UT_DBG() LOG_DEBUG()
#define UT_ERR() LOG_ERROR()
#define UT_WARN() LOG_WARN()
#define UT_EXPECT_ANY_THROW(statement) do {EXPECT_ANY_THROW(statement);} while (0)
#define UT_EXPECT_NO_THROW(statement) do {EXPECT_NO_THROW(statement);} while (0)
#define UT_EXPECT_LE(a, b) do {EXPECT_LE(a, b);} while (0)
#define UT_EXPECT_LT(a, b) do {EXPECT_LT(a, b);} while (0)
#define UT_EXPECT_GE(a, b) do {EXPECT_GE(a, b);} while (0)
#define UT_EXPECT_GT(a, b) do {EXPECT_GT(a, b);} while (0)
#define UT_EXPECT_NE(a, b) do {EXPECT_NE(a, b);} while (0)
#define UT_EXPECT_EQ(a, b) do {EXPECT_EQ(a, b);} while (0)
#define UT_EXPECT_FALSE(condition) do {EXPECT_FALSE(condition);} while (0)
#define UT_EXPECT_TRUE(condition) do {EXPECT_TRUE(condition);} while (0)
#define UT_ASSERT(cond) do {EXPECT_TRUE(cond);} while (0)

class TuGraphTest : public testing::Test {
 protected:
    void SetUp() override;
    void TearDown() override;

 public:
    static void setup();
    static void teardown();
};

template<typename T>
class TuGraphTestWithParam : public testing::TestWithParam<T> {
 protected:
    void SetUp() {
        TuGraphTest::setup();
    }

    void TearDown() {
        TuGraphTest::teardown();
    }
};

void build_so(const std::string& so_name, const std::string& so_path);

bool HasElement(const nlohmann::json& val, const std::string& value, const std::string& field);
