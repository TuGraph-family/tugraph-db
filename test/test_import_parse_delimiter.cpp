/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/logger.h"
#include "fma-common/utils.h"
#include "fma-common/unit_test_utils.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "import/parse_delimiter.h"

class TestImportParseDelimiter : public TuGraphTest {};

TEST_F(TestImportParseDelimiter, ImportParseDelimiter) {
    using namespace lgraph;
    UT_EXPECT_EQ(ParseDelimiter(","), ",");
    UT_EXPECT_EQ(ParseDelimiter("\""), "\"");
    UT_EXPECT_EQ(ParseDelimiter("abcd1234"), "abcd1234");
    // \\  \a \f \n \r \t \v
    UT_EXPECT_EQ(ParseDelimiter("\\\\"), "\\");
    UT_EXPECT_EQ(ParseDelimiter("\\ab"), "\ab");
    UT_EXPECT_EQ(ParseDelimiter("ff\\f"), "ff\f");
    UT_EXPECT_EQ(ParseDelimiter("k\\r\\np"), "k\r\np");
    UT_EXPECT_EQ(ParseDelimiter("\\t"), "\t");
    UT_EXPECT_EQ(ParseDelimiter("pp\\vv"), "pp\vv");
    UT_EXPECT_EQ(ParseDelimiter(" "), " ");
    // \nnn
    UT_EXPECT_EQ(ParseDelimiter("\\001"), std::string() + '\001');
    UT_EXPECT_EQ(ParseDelimiter("\\254"), "\254");
    UT_EXPECT_THROW_MSG(ParseDelimiter("\\500"), "Illegal escape sequence: \\500");
    UT_EXPECT_THROW_MSG(ParseDelimiter("\\777"), "Illegal escape sequence: \\777");
    UT_EXPECT_THROW_MSG(ParseDelimiter("\\998"), "Illegal escape sequence: \\9");
    // \Xnn
    UT_EXPECT_EQ(ParseDelimiter("\\xABCD"),
                 "\xAB"
                 "CD");
    UT_EXPECT_EQ(ParseDelimiter("55\\xA9CD"),
                 "55\xA9"
                 "CD");
    UT_EXPECT_THROW_MSG(ParseDelimiter("0\\xGG"), "Illegal escape sequence: \\xG");
    // illegal
    UT_EXPECT_THROW_MSG(ParseDelimiter("0\\ii"), "Illegal escape sequence: \\i");
}
