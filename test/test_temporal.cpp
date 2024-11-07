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

#include <gtest/gtest.h>
#include "common/temporal/temporal.h"

TEST(Date, dateFromString) {
    EXPECT_EQ(common::Date("2015-07-21").ToString(), "2015-07-21");
    EXPECT_EQ(common::Date("20150721").ToString(), "2015-07-21");
    EXPECT_EQ(common::Date("2015-07").ToString(), "2015-07-01");
    EXPECT_EQ(common::Date("201507").ToString(), "2015-07-01");
    EXPECT_EQ(common::Date("2015-W30-2").ToString(), "2015-07-21");
    EXPECT_EQ(common::Date("2015W302").ToString(), "2015-07-21");
    EXPECT_EQ(common::Date("2015-W30").ToString(), "2015-07-20");
    EXPECT_EQ(common::Date("2015W30").ToString(), "2015-07-20");
    EXPECT_EQ(common::Date("2015-202").ToString(), "2015-07-21");
    EXPECT_EQ(common::Date("2015202").ToString(), "2015-07-21");
    EXPECT_EQ(common::Date("2015").ToString(), "2015-01-01");
}

TEST(Date, dateFromStringException) {}

TEST(Date, dateFromMap) {}
