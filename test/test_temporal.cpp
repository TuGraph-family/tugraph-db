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

TEST(Date, dateFromMap) {
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1816)},
                                  {"week", Value((int64_t)1)}}))
                  .ToString(),
              "1816-01-01");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1816)},
                                  {"week", Value((int64_t)52)}}))
                  .ToString(),
              "1816-12-23");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1817)},
                                  {"week", Value((int64_t)1)}}))
                  .ToString(),
              "1816-12-30");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1817)},
                                  {"week", Value((int64_t)10)}}))
                  .ToString(),
              "1817-03-03");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1817)},
                                  {"week", Value((int64_t)30)}}))
                  .ToString(),
              "1817-07-21");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1817)},
                                  {"week", Value((int64_t)52)}}))
                  .ToString(),
              "1817-12-22");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1818)},
                                  {"week", Value((int64_t)1)}}))
                  .ToString(),
              "1817-12-29");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1818)},
                                  {"week", Value((int64_t)52)}}))
                  .ToString(),
              "1818-12-21");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1818)},
                                  {"week", Value((int64_t)53)}}))
                  .ToString(),
              "1818-12-28");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1819)},
                                  {"week", Value((int64_t)1)}}))
                  .ToString(),
              "1819-01-04");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1819)},
                                  {"week", Value((int64_t)52)}}))
                  .ToString(),
              "1819-12-27");

    EXPECT_EQ(common::Date(Value({{"dayOfWeek", Value((int64_t)2)},
                                  {"year", Value((int64_t)1817)},
                                  {"week", Value((int64_t)1)}}))
                  .ToString(),
              "1816-12-31");

    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)},
                                  {"month", Value((int64_t)10)},
                                  {"day", Value((int64_t)11)}}))
                  .ToString(),
              "1984-10-11");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)},
                                  {"month", Value((int64_t)10)}}))
                  .ToString(),
              "1984-10-01");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)},
                                  {"week", Value((int64_t)10)},
                                  {"dayOfWeek", Value((int64_t)3)}}))
                  .ToString(),
              "1984-03-07");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)},
                                  {"week", Value((int64_t)10)}}))
                  .ToString(),
              "1984-03-05");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)}})).ToString(),
              "1984-01-01");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)},
                                  {"ordinalDay", Value((int64_t)202)}}))
                  .ToString(),
              "1984-07-20");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)},
                                  {"quarter", Value((int64_t)3)},
                                  {"dayOfQuarter", Value((int64_t)45)}}))
                  .ToString(),
              "1984-08-14");
    EXPECT_EQ(common::Date(Value({{"year", Value((int64_t)1984)},
                                  {"quarter", Value((int64_t)3)}}))
                  .ToString(),
              "1984-07-01");

    EXPECT_EQ(common::Date(Value({{"date", Value(common::Date("1816-12-30"))},
                                  {"week", Value((int64_t)2)},
                                  {"dayOfWeek", Value((int64_t)3)}}))
                  .ToString(),
              "1817-01-08");

    EXPECT_EQ(common::Date(Value({{"date", Value(common::Date("1816-12-31"))},
                                  {"week", Value((int64_t)2)}}))
                  .ToString(),
              "1817-01-07");

    EXPECT_EQ(common::Date(Value({{"date", Value(common::Date("1816-12-31"))},
                                  {"year", Value((int64_t)1817)},
                                  {"week", Value((int64_t)2)}}))
                  .ToString(),
              "1817-01-07");
}

TEST(LocalDateTime, localDateTimeFromString) {
    EXPECT_EQ(common::LocalDateTime("2015-07-21T21:40:32.142").ToString(),
              "2015-07-21T21:40:32.142000000");
    EXPECT_EQ(common::LocalDateTime("2015-W30-2T214032.142").ToString(),
              "2015-07-21T21:40:32.142000000");
    EXPECT_EQ(common::LocalDateTime("2015-202T21:40:32").ToString(),
              "2015-07-21T21:40:32.000000000");
    EXPECT_EQ(common::LocalDateTime("2015T214032").ToString(),
              "2015-01-01T21:40:32.000000000");
    EXPECT_EQ(common::LocalDateTime("20150721T21:40").ToString(),
              "2015-07-21T21:40:00.000000000");
    EXPECT_EQ(common::LocalDateTime("2015-W30T2140").ToString(),
              "2015-07-20T21:40:00.000000000");
    EXPECT_EQ(common::LocalDateTime("2015202T21").ToString(),
              "2015-07-21T21:00:00.000000000");
}

TEST(LocalDateTime, localDateTimeFromMap) {
    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1816)},
                                                {"week", Value::Integer(1)}}))
                  .ToString(),
              "1816-01-01T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1816)},
                                                {"week", Value::Integer(52)}}))
                  .ToString(),
              "1816-12-23T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1817)},
                                                {"week", Value::Integer(1)}}))
                  .ToString(),
              "1816-12-30T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1817)},
                                                {"week", Value::Integer(10)}}))
                  .ToString(),
              "1817-03-03T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1817)},
                                                {"week", Value::Integer(30)}}))
                  .ToString(),
              "1817-07-21T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1817)},
                                                {"week", Value::Integer(52)}}))
                  .ToString(),
              "1817-12-22T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1818)},
                                                {"week", Value::Integer(1)}}))
                  .ToString(),
              "1817-12-29T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1818)},
                                                {"week", Value::Integer(52)}}))
                  .ToString(),
              "1818-12-21T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1818)},
                                                {"week", Value::Integer(53)}}))
                  .ToString(),
              "1818-12-28T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1819)},
                                                {"week", Value::Integer(1)}}))
                  .ToString(),
              "1819-01-04T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1819)},
                                                {"week", Value::Integer(52)}}))
                  .ToString(),
              "1819-12-27T00:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"dayOfWeek", Value::Integer(2)},
                                          {"year", Value::Integer(1817)},
                                          {"week", Value::Integer(1)}}))
            .ToString(),
        "1816-12-31T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value({{"date", Value(common::Date("1816-12-30"))},
                         {"week", Value((int64_t)2)},
                         {"dayOfWeek", Value((int64_t)3)}}))
                  .ToString(),
              "1817-01-08T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value({{"date", Value(common::Date("1816-12-31"))},
                         {"week", Value((int64_t)2)}}))
                  .ToString(),
              "1817-01-07T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value({{"date", Value(common::Date("1816-12-31"))},
                         {"year", Value((int64_t)1817)},
                         {"week", Value((int64_t)2)}}))
                  .ToString(),
              "1817-01-07T00:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)}}))
            .ToString(),
        "1984-01-01T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"quarter", Value::Integer(3)},
                              {"dayOfQuarter", Value::Integer(45)}}))
                  .ToString(),
              "1984-08-14T00:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"quarter", Value::Integer(3)},
                                          {"dayOfQuarter", Value::Integer(45)},
                                          {"hour", Value::Integer(12)}}))
            .ToString(),
        "1984-08-14T12:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"quarter", Value::Integer(3)},
                                          {"dayOfQuarter", Value::Integer(45)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)}}))
            .ToString(),
        "1984-08-14T12:31:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"quarter", Value::Integer(3)},
                                          {"dayOfQuarter", Value::Integer(45)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)},
                                          {"second", Value::Integer(14)}}))
            .ToString(),
        "1984-08-14T12:31:14.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"quarter", Value::Integer(3)},
                              {"dayOfQuarter", Value::Integer(45)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"millisecond", Value::Integer(645)}}))
                  .ToString(),
              "1984-08-14T12:31:14.645000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"quarter", Value::Integer(3)},
                              {"dayOfQuarter", Value::Integer(45)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"microsecond", Value::Integer(645876)}}))
                  .ToString(),
              "1984-08-14T12:31:14.645876000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"quarter", Value::Integer(3)},
                              {"dayOfQuarter", Value::Integer(45)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"nanosecond", Value::Integer(645876123)}}))
                  .ToString(),
              "1984-08-14T12:31:14.645876123");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"ordinalDay", Value::Integer(202)}}))
            .ToString(),
        "1984-07-20T00:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"ordinalDay", Value::Integer(202)},
                                          {"hour", Value::Integer(12)}}))
            .ToString(),
        "1984-07-20T12:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"ordinalDay", Value::Integer(202)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)}}))
            .ToString(),
        "1984-07-20T12:31:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"ordinalDay", Value::Integer(202)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)},
                                          {"second", Value::Integer(14)}}))
            .ToString(),
        "1984-07-20T12:31:14.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"ordinalDay", Value::Integer(202)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"millisecond", Value::Integer(645)}}))
                  .ToString(),
              "1984-07-20T12:31:14.645000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"ordinalDay", Value::Integer(202)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"microsecond", Value::Integer(645876)}}))
                  .ToString(),
              "1984-07-20T12:31:14.645876000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"ordinalDay", Value::Integer(202)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"nanosecond", Value::Integer(645876123)}}))
                  .ToString(),
              "1984-07-20T12:31:14.645876123");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"week", Value::Integer(10)},
                                          {"dayOfWeek", Value::Integer(3)}}))
            .ToString(),
        "1984-03-07T00:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"week", Value::Integer(10)},
                                          {"dayOfWeek", Value::Integer(3)},
                                          {"hour", Value::Integer(12)}}))
            .ToString(),
        "1984-03-07T12:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"week", Value::Integer(10)},
                                          {"dayOfWeek", Value::Integer(3)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)}}))
            .ToString(),
        "1984-03-07T12:31:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"week", Value::Integer(10)},
                                          {"dayOfWeek", Value::Integer(3)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)},
                                          {"second", Value::Integer(14)}}))
            .ToString(),
        "1984-03-07T12:31:14.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"week", Value::Integer(10)},
                              {"dayOfWeek", Value::Integer(3)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"millisecond", Value::Integer(645)}}))
                  .ToString(),
              "1984-03-07T12:31:14.645000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"week", Value::Integer(10)},
                              {"dayOfWeek", Value::Integer(3)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"microsecond", Value::Integer(645876)}}))
                  .ToString(),
              "1984-03-07T12:31:14.645876000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"week", Value::Integer(10)},
                              {"dayOfWeek", Value::Integer(3)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"nanosecond", Value::Integer(645876123)}}))
                  .ToString(),
              "1984-03-07T12:31:14.645876123");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                                {"month", Value::Integer(10)},
                                                {"day", Value::Integer(11)}}))
                  .ToString(),
              "1984-10-11T00:00:00.000000000");

    EXPECT_EQ(common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                                {"month", Value::Integer(10)},
                                                {"day", Value::Integer(11)},
                                                {"hour", Value::Integer(12)}}))
                  .ToString(),
              "1984-10-11T12:00:00.000000000");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"month", Value::Integer(10)},
                                          {"day", Value::Integer(11)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)}}))
            .ToString(),
        "1984-10-11T12:31:00.000000000");
    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"month", Value::Integer(10)},
                                          {"day", Value::Integer(11)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)},
                                          {"second", Value::Integer(14)}}))
            .ToString(),
        "1984-10-11T12:31:14.000000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"month", Value::Integer(10)},
                              {"day", Value::Integer(11)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"millisecond", Value::Integer(645)}}))
                  .ToString(),
              "1984-10-11T12:31:14.645000000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"month", Value::Integer(10)},
                              {"day", Value::Integer(11)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"microsecond", Value::Integer(645876)}}))
                  .ToString(),
              "1984-10-11T12:31:14.645876000");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"month", Value::Integer(10)},
                              {"day", Value::Integer(11)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"nanosecond", Value::Integer(645876123)}}))
                  .ToString(),
              "1984-10-11T12:31:14.645876123");

    EXPECT_EQ(
        common::LocalDateTime(Value::Map({{"year", Value::Integer(1984)},
                                          {"month", Value::Integer(10)},
                                          {"day", Value::Integer(11)},
                                          {"hour", Value::Integer(12)},
                                          {"minute", Value::Integer(31)},
                                          {"second", Value::Integer(14)},
                                          {"nanosecond", Value::Integer(3)}}))
            .ToString(),
        "1984-10-11T12:31:14.000000003");

    EXPECT_EQ(common::LocalDateTime(
                  Value::Map({{"year", Value::Integer(1984)},
                              {"month", Value::Integer(10)},
                              {"day", Value::Integer(11)},
                              {"hour", Value::Integer(12)},
                              {"minute", Value::Integer(31)},
                              {"second", Value::Integer(14)},
                              {"nanosecond", Value::Integer(789)},
                              {"millisecond", Value::Integer(123)},
                              {"microsecond", Value::Integer(456)}}))
                  .ToString(),
              "1984-10-11T12:31:14.123456789");
}

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
