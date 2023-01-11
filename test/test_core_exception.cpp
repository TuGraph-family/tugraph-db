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

#include <string>
#include <iostream>
#include <fstream>

#include "fma-common/fma_stream.h"
#include "fma-common/logging.h"
#include "fma-common/text_parser.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "import/import_exception.h"

using namespace fma_common;
using namespace lgraph;
using namespace lgraph::_detail;

class TestCoreException : public TuGraphTest {};

int TestException() {
    std::string beg("test app exception");
    std::string end("test end!");
    UT_LOG() << DumpLine(beg.data(), beg.data() + beg.size());
    UT_LOG() << BinaryLine(beg);
    return 0;
}

void TestFailSkip() {
    std::string line("test input line");
    size_t column = 2;
    FailToSkipColumnException fail_exp(&line[0], &line[0] + line.size(), column);
    UT_LOG() << "fail exception" << fail_exp.what();
    FailToParseColumnException fail_parse(&line[0], &line[0] + 2, &line[0] + line.size(), column,
                                          FieldType::STRING);
    UT_LOG() << "parse exception" << fail_parse.what();
    MissingDelimiterException miss_delimiter(&line[0], &line[0] + line.size(), column);
    UT_LOG() << "miss delimiter exception" << miss_delimiter.what();
}

TEST_F(TestCoreException, CoreException) {
    TestFailSkip();
    TestException();
}

/*
TEST_F(TestCoreException, ShouldFail) {
    FAIL() << "Expected failure";
}*/
