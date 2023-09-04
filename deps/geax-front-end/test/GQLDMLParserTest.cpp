/**
 * Copyright 2023 AntGroup CO., Ltd.
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
 *
 * Author:
 *         Yaochi <boyao.zby@alibaba-inc.com>
 */

#include <gtest/gtest.h>

#include "GQLParserTest.h"
#include "geax-front-end/utils/Logging.h"

namespace geax {
namespace frontend {

class GQLDMLParserTest : public ::testing::Test {
};

TEST_F(GQLDMLParserTest, base) {
    GQLParserTest parser;
    std::string testPath = "../../../../test/case/dml/";
    ASSERT_EQ(parser.test(testPath), GEAXErrorCode::GEAX_SUCCEED);
}

}  // end of namespace frontend
}  // end of namespace geax
