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
#include "gtest/gtest.h"

#include "db/token_manager.h"
#include "./test_tools.h"
#include "./ut_utils.h"
#include "db/acl.h"

class TestTokenManager : public TuGraphTest {};

TEST_F(TestTokenManager, TokenManager) {
    using lgraph::TokenManager;

    size_t clock_skew = 100000;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    fma_common::Configuration config;
    config.Add(clock_skew, "skew", true).Comment("Clock skew in microseconds.");
    config.ParseAndFinalize(argc, argv);
    UT_LOG() << "===Generating and validating...";
    TokenManager m("secret");
    TokenManager m2("another_secret");

    std::string tok1 = m.IssueFirstToken();
    UT_EXPECT_EQ(m.JudgeRefreshTime(tok1), true);
    std::string tok2 = m.UpdateToken(tok1);
    UT_EXPECT_EQ(m.JudgeRefreshTime(tok2), true);
    m.SetTokenTimeUnlimited();
    UT_EXPECT_EQ(m.GetTokenTime(tok2).first, std::numeric_limits<int>::max());
    UT_EXPECT_EQ(m.GetTokenTime(tok2).second, std::numeric_limits<int>::max());
    m.ModifyRefreshTime(tok2, 1);
    fma_common::SleepS(1);
    UT_EXPECT_EQ(m.JudgeRefreshTime(tok2), false);
    m.ModifyRefreshTime(tok2, 600);
    m.ModifyExpireTime(tok2, 3600);
    UT_EXPECT_EQ(m.GetTokenTime(tok2).first, 600);
    UT_EXPECT_EQ(m.GetTokenTime(tok2).second, 3600);
    auto new_token = m.UpdateToken(tok2);
    UT_EXPECT_EQ(m.JudgeRefreshTime(new_token), true);
}
