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

#include <cstdio>
#include <cctype>

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/text_parser.h"
#include "./unit_test_utils.h"

using namespace fma_common;

FMA_SET_TEST_PARAMS(TextParserUtils, "", "--nIter 1", "--nIter 2", "--nIter 10");

FMA_UNIT_TEST(TextParserUtils) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    int n_iter = 10000000;
    bool performance = false;
    Configuration config;
    config.Add(n_iter, "nIter", true).Comment("Number of iterations to run");
    config.Add(performance, "perf", true).Comment("Whether to run performance test");
    config.ExitAfterHelp();
    config.ParseAndFinalize(argc, argv);

    if (performance) {
        double t1 = GetTime();
        size_t sum1 = 0;
        for (int i = 0; i < n_iter; i++) {
            for (unsigned char c = 0; c < 255; c++) {
                sum1 += TextParserUtils::IsDigits(c);
            }
        }
        double t2 = GetTime();
        size_t sum2 = 0;
        for (int i = 0; i < n_iter; i++) {
            for (unsigned char c = 0; c < 255; c++) {
                sum2 += (c >= '0' && c <= '9');
            }
        }
        double t3 = GetTime();
        LOG() << "method1: " << t2 - t1;
        LOG() << "method2: " << t3 - t2;
        FMA_UT_CHECK_EQ(sum1, sum2);
        return 0;
    }

    {
        for (int i = 0; i < 256; i++) {
            FMA_UT_CHECK_EQ(TextParserUtils::IsControl(i), (bool)std::iscntrl(i));
            FMA_UT_CHECK_EQ(TextParserUtils::IsDigits(i), (bool)std::isdigit(i));
            FMA_UT_CHECK_EQ(TextParserUtils::IsDigital(i), ((bool)std::isdigit(i) || i == '-' ||
                                                     i == '.' || i == '-' || i == 'e' || i == 'E'));
            FMA_UT_CHECK_EQ(TextParserUtils::IsGraphical(i), (bool)std::isgraph(i));
            FMA_UT_CHECK_EQ(TextParserUtils::IsNewLine(i), (i == '\r' || i == '\n'));
            FMA_UT_CHECK_EQ(TextParserUtils::IsAlphabetNumberic(i), (bool)std::isalnum(i));
            FMA_UT_CHECK_EQ(TextParserUtils::IsBlank(i), (bool)std::isblank(i));
            FMA_UT_CHECK_EQ(TextParserUtils::IsValidNameCharacter(i),
                            (bool)std::isalnum(i) || i == '_');
        }
    }

    {
        bool b = false;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("True", b), 4);
        FMA_UT_ASSERT(b);
        b = false;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("t", b), 0);
        b = false;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("1", b), 1);
        FMA_UT_ASSERT(b);
        b = true;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("fAlse", b), 5);
        FMA_UT_ASSERT(!b);
        b = true;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("F", b), 0);
        b = true;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("0", b), 1);
        FMA_UT_ASSERT(!b);
        b = false;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("100", b), 3);
        FMA_UT_ASSERT(b);
        b = false;
        FMA_UT_CHECK_EQ(TextParserUtils::ParseT("-100", b), 4);
        FMA_UT_ASSERT(b);
    }

    std::string num = "+30.1223E+3";
    double truev = +30.1223E+3;
    double dou = 0;
    // FMA_UT_CHECK_EQ(TextParserUtils::ParseDigit(num.data(), &num[num.size()], dou), num.size());
    FMA_UT_CHECK_EQ(TextParserUtils::ParseDouble(num.data(), &num[num.size()], dou), num.size());
    double delta = (dou - truev) / truev;
    FMA_UT_ASSERT(delta >= -1e-6 && delta <= 1e-6);
    LOG() << dou;

    double t1 = GetTime();
    double x = 0;
    size_t bytes_parsed = 0;
    int d = 0;
    // const char* beg = (const char*)&num[0];
    // const char* end = (const char*)&num[num.size()];
    for (int i = 0; i < n_iter; i++) {
        // bytes_parsed += TextParserUtils::ParseDigit(beg, end, d);
        d = (int)TextParserUtils::IsAlphabetNumberic((char)i);
        x += d;
    }
    double et = GetTime() - t1;
    // LOG() << "x=" << x;
    printf("%30f\n", x);
    LOG() << "Ran " << n_iter << " iterations at " << (double)n_iter / et << " tps";
    LOG() << "Parsed " << bytes_parsed << "bytes at " << (double)bytes_parsed / 1024 / 1024 / et
          << "MB/s";

    LOG() << "Testing FindNextLine";
    // test FindNextLine
    {
        const std::string str = "1\r\n2\n\r3\r4\n5\n\n7";
        const char *b = str.data();
        const char *e = str.data() + str.size();
        const char *p = TextParserUtils::FindNextLine(b, e);
        FMA_UT_CHECK_EQ(*p, '2');
        p = TextParserUtils::FindNextLine(p, e);
        FMA_UT_CHECK_EQ(*p, '3');
        p = TextParserUtils::FindNextLine(p, e);
        FMA_UT_CHECK_EQ(*p, '4');
        p = TextParserUtils::FindNextLine(p, e);
        FMA_UT_CHECK_EQ(*p, '5');
        p = TextParserUtils::FindNextLine(p, e);
        FMA_UT_CHECK_EQ(*p, '\n');
        p = TextParserUtils::FindNextLine(p, e);
        FMA_UT_CHECK_EQ(*p, '7');
        p = TextParserUtils::FindNextLine(p, e);
        FMA_UT_CHECK_EQ(p, e);
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
