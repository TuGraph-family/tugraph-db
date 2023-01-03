/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
}
