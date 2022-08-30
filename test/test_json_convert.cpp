/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/logger.h"
#include "gtest/gtest.h"
#include "restful/server/json_convert.h"
#include "./ut_utils.h"

class TestJsonConvert : public TuGraphTest {};

TEST_F(TestJsonConvert, JsonConvert) {
    using namespace lgraph;
    auto js = ValueToJson(std::map<std::string, int>({{"a", 1}}));
    UT_LOG() << _TS(js.serialize());
}
