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
#include "common/value.h"

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

TEST(LocalTime, timeFromString) {
    EXPECT_EQ(common::LocalTime("21:40:32.142").ToString(), "21:40:32.142000000");
    EXPECT_EQ(common::LocalTime("214032.142").ToString(), "21:40:32.142000000");
    EXPECT_EQ(common::LocalTime("21:40:32").ToString(), "21:40:32.000000000");
    EXPECT_EQ(common::LocalTime("214032").ToString(), "21:40:32.000000000");
    EXPECT_EQ(common::LocalTime("21:40").ToString(), "21:40:00.000000000");
    EXPECT_EQ(common::LocalTime("2140").ToString(), "21:40:00.000000000");
    EXPECT_EQ(common::LocalTime("21").ToString(), "21:00:00.000000000");
}

TEST(LocalTime, timeFromMap) {
    EXPECT_EQ(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                 {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(789)},
                                 {"millisecond", Value::Integer(123)}, {"microsecond", Value::Integer(456)}})).ToString(),
              "12:31:14.123456789");
    EXPECT_EQ(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                            {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})).ToString(),
              "12:31:14.645876123");
    EXPECT_EQ(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                            {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)}})).ToString(),
              "12:31:14.645876000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                            {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})).ToString(),
              "12:31:14.645000000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                            {"second", Value::Integer(14)}})).ToString(), "12:31:14.000000000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)}})).ToString(),
              "12:31:00.000000000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}})).ToString(),
              "12:00:00.000000000");
    EXPECT_EQ(common::LocalTime(Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                             {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))).ToString(),
              "12:31:14.645876123");
    EXPECT_EQ(common::LocalTime(Value::Map({{"time", Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                       {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))}})).ToString(),
              "12:31:14.645876123");
    EXPECT_EQ(common::LocalTime(Value::Map({
                                    {"time", Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                        {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))},
                                    {"second", Value::Integer(42)}
                                })).ToString(),
              "12:31:42.645876123");
    EXPECT_TRUE(common::LocalTime(Value::Map({{"hour", Value::Integer(10)}, {"minute", Value::Integer(35)}})) < common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                                                                              {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})));
    EXPECT_TRUE(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                              {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})) == common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                                                                              {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})));
}
