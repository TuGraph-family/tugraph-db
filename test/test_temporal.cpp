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
#include <filesystem>
#include "common/logger.h"
#include "common/temporal/temporal.h"
#include "common/value.h"
#include "cypher/execution_plan/result_iterator.h"
#include "graphdb/graph_db.h"
using namespace graphdb;
namespace fs = std::filesystem;

static std::string test_db = "temporal_db";

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

TEST(Date, truncateDate2Date) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 2017, month: 10, day: 11}) AS other\n"
        "RETURN\n"
        "   date.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   date.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "2000-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "2000-01-01");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt =
        txn->Execute(&rtx,
                     "WITH date({year: 1984, month: 10, day: 11}) AS other\n"
                     "RETURN\n"
                     "  date.truncate('century', other, {day: 2}) AS col0,\n"
                     "  date.truncate('century', other, {}) AS col1,\n"
                     "  date.truncate('decade', other, {day: 2}) AS col2,\n"
                     "  date.truncate('decade', other, {}) AS col3,\n"
                     "  date.truncate('year', other, {day: 2}) AS col4,\n"
                     "  date.truncate('year', other, {}) AS col5,\n"
                     "  date.truncate('month', other, {day: 2}) AS col6,\n"
                     "  date.truncate('month', other, {}) AS col7,\n"
                     "  date.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
                     "  date.truncate('week', other, {}) AS col9,\n"
                     "  date.truncate('day', other, {}) AS col10");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1900-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1900-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDate().ToString(),
                  "1980-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDate().ToString(),
                  "1980-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDate().ToString(),
                  "1984-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDate().ToString(),
                  "1984-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDate().ToString(),
                  "1984-10-02");
        EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDate().ToString(),
                  "1984-10-01");
        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDate().ToString(),
                  "1984-10-09");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDate().ToString(),
                  "1984-10-08");
        EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDate().ToString(),
                  "1984-10-11");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt =
        txn->Execute(&rtx,
                     "WITH date({year: 1984, month: 2, day: 1}) AS other\n"
                     "RETURN\n"
                     "   date.truncate('weekYear', other, {day: 5}) AS col0,\n"
                     "   date.truncate('weekYear', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1984-01-05");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1984-01-02");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt =
        txn->Execute(&rtx,
                     "WITH date({year: 1984, month: 11, day: 11}) AS other\n"
                     "RETURN\n"
                     "   date.truncate('quarter', other, {day: 2}) AS col0,\n"
                     "   date.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1984-10-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1984-10-01");
    }
    txn->Commit();
}

TEST(Date, truncateDatetime2Date) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 2017, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "   date.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   date.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "2000-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "2000-01-01");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "  date.truncate('century', other, {day: 2}) AS col0,\n"
        "  date.truncate('century', other, {}) AS col1,\n"
        "  date.truncate('decade', other, {day: 2}) AS col2,\n"
        "  date.truncate('decade', other, {}) AS col3,\n"
        "  date.truncate('year', other, {day: 2}) AS col4,\n"
        "  date.truncate('year', other, {}) AS col5,\n"
        "  date.truncate('month', other, {day: 2}) AS col6,\n"
        "  date.truncate('month', other, {}) AS col7,\n"
        "  date.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
        "  date.truncate('week', other, {}) AS col9,\n"
        "  date.truncate('day', other, {}) AS col10");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1900-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1900-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDate().ToString(),
                  "1980-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDate().ToString(),
                  "1980-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDate().ToString(),
                  "1984-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDate().ToString(),
                  "1984-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDate().ToString(),
                  "1984-10-02");
        EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDate().ToString(),
                  "1984-10-01");
        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDate().ToString(),
                  "1984-10-09");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDate().ToString(),
                  "1984-10-08");
        EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDate().ToString(),
                  "1984-10-11");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 2, day: 1, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "   date.truncate('weekYear', other, {day: 5}) AS col0,\n"
        "   date.truncate('weekYear', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1984-01-05");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1984-01-02");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 11, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "   date.truncate('quarter', other, {day: 2}) AS col0,\n"
        "   date.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1984-10-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1984-10-01");
    }
    txn->Commit();
}

TEST(Date, truncateLocaldatetime2Date) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 2017, month: 10, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   date.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   date.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "2000-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "2000-01-01");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 10, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "  date.truncate('century', other, {day: 2}) AS col0,\n"
        "  date.truncate('century', other, {}) AS col1,\n"
        "  date.truncate('decade', other, {day: 2}) AS col2,\n"
        "  date.truncate('decade', other, {}) AS col3,\n"
        "  date.truncate('year', other, {day: 2}) AS col4,\n"
        "  date.truncate('year', other, {}) AS col5,\n"
        "  date.truncate('month', other, {day: 2}) AS col6,\n"
        "  date.truncate('month', other, {}) AS col7,\n"
        "  date.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
        "  date.truncate('week', other, {}) AS col9,\n"
        "  date.truncate('day', other, {}) AS col10");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1900-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1900-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDate().ToString(),
                  "1980-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDate().ToString(),
                  "1980-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDate().ToString(),
                  "1984-01-02");
        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDate().ToString(),
                  "1984-01-01");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDate().ToString(),
                  "1984-10-02");
        EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDate().ToString(),
                  "1984-10-01");
        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDate().ToString(),
                  "1984-10-09");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDate().ToString(),
                  "1984-10-08");
        EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDate().ToString(),
                  "1984-10-11");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 2, day: 1, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   date.truncate('weekYear', other, {day: 5}) AS col0,\n"
        "   date.truncate('weekYear', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1984-01-05");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1984-01-02");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 11, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   date.truncate('quarter', other, {day: 2}) AS col0,\n"
        "   date.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDate().ToString(),
                  "1984-10-02");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDate().ToString(),
                  "1984-10-01");
    }
    txn->Commit();
}

TEST(DateTime, dateTimeFromString) {
    EXPECT_EQ(common::DateTime("2015-07-21T21:40:32.142+0100").ToString(),
              "2015-07-21T21:40:32.142000000+01:00:00");
    EXPECT_EQ(common::DateTime("2015-W30-2T214032.142Z").ToString(),
              "2015-07-21T21:40:32.142000000Z");
    EXPECT_EQ(common::DateTime("2015-202T21:40:32+01:00").ToString(),
              "2015-07-21T21:40:32.000000000+01:00:00");
    EXPECT_EQ(common::DateTime("2015T214032-0100").ToString(),
              "2015-01-01T21:40:32.000000000-01:00:00");
    EXPECT_EQ(common::DateTime("20150721T21:40-01:30").ToString(),
              "2015-07-21T21:40:00.000000000-01:30:00");
    EXPECT_EQ(common::DateTime("2015-W30T2140-00:00").ToString(),
              "2015-07-20T21:40:00.000000000Z");
    EXPECT_EQ(common::DateTime("2015-W30T2140-02").ToString(),
              "2015-07-20T21:40:00.000000000-02:00:00");
    EXPECT_EQ(common::DateTime("2015202T21+18:00").ToString(),
              "2015-07-21T21:00:00.000000000+18:00:00");
}

TEST(DateTime, truncateDate2Datetime) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 2017, month: 10, day: 11}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   datetime.truncate('millennium', other, {}) AS col1,\n"
        "   datetime.truncate('millennium', other, {timezone: "
        "'Europe/Stockholm'}) AS col2,\n"
        "   datetime.truncate('century', other, {timezone: "
        "'Europe/Stockholm'}) AS col3");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "2000-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 1984, month: 10, day: 11}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('century', other, {day: 2}) AS col0,\n"
        "  datetime.truncate('century', other, {}) AS col1,\n"
        "  datetime.truncate('decade', other, {day: 2}) AS col2,\n"
        "  datetime.truncate('decade', other, {}) AS col3,\n"
        "  datetime.truncate('decade', other, {timezone: "
        "'Europe/Stockholm'}) AS col4,\n"
        "  datetime.truncate('year', other, {day: 2}) AS col5,\n"
        "  datetime.truncate('year', other, {}) AS col6,\n"
        "  datetime.truncate('year', other, {timezone: 'Europe/Stockholm'}) AS "
        "col7,\n"
        "  datetime.truncate('month', other, {day: 2}) AS col8,\n"
        "  datetime.truncate('month', other, {}) AS col9,\n"
        "  datetime.truncate('month', other, {timezone: 'Europe/Stockholm'}) "
        "AS col10,\n"
        "  datetime.truncate('week', other, {dayOfWeek: 2}) AS col11,\n"
        "  datetime.truncate('week', other, {}) AS col12,\n"
        "  datetime.truncate('week', other, {timezone: 'Europe/Stockholm'}) AS "
        "col13,\n"
        "  datetime.truncate('day', other, {}) AS col14,\n"
        "  datetime.truncate('day', other, {nanosecond: 2}) AS col15,\n"
        "  datetime.truncate('day', other, {timezone: 'Europe/Stockholm'}) AS "
        "col16");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1900-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1900-01-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "1980-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDateTime().ToString(),
                  "1980-01-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDateTime().ToString(),
                  "1980-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDateTime().ToString(),
                  "1984-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDateTime().ToString(),
                  "1984-01-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDateTime().ToString(),
                  "1984-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDateTime().ToString(),
                  "1984-10-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[11].data).AsDateTime().ToString(),
                  "1984-10-09T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[12].data).AsDateTime().ToString(),
                  "1984-10-08T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[13].data).AsDateTime().ToString(),
                  "1984-10-08T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[14].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[15].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000002Z");
        EXPECT_EQ(std::any_cast<Value>(record[16].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 1984, month: 2, day: 1}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('weekYear', other, {day: 5}) AS col0,\n"
        "   datetime.truncate('weekYear', other, {}) AS col1,\n"
        "   datetime.truncate('weekYear', other, {timezone: "
        "'Europe/Stockholm'}) AS col2");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1984-01-05T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1984-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "1984-01-02T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 1984, month: 11, day: 11}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('quarter', other, {day: 2}) AS col0,\n"
        "  datetime.truncate('quarter', other, {}) AS col1,\n"
        "  datetime.truncate('quarter', other, {timezone: 'Europe/Stockholm'}) "
        "AS col2");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1984-10-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();
}

TEST(DateTime, truncateDatetime2Datetime) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 2017, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   datetime.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "2000-01-02T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 2017, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '-01:00'}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('millennium', other, {timezone: "
        "'Europe/Stockholm'}) AS col0,\n"
        "   datetime.truncate('century', other, {timezone: "
        "'Europe/Stockholm'}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('century', other, {day: 2}) AS col0,\n"
        "  datetime.truncate('century', other, {}) AS col1,\n"
        "  datetime.truncate('decade', other, {day: 2}) AS col2,\n"
        "  datetime.truncate('decade', other, {}) AS col3,\n"
        "  datetime.truncate('year', other, {day: 2}) AS col4,\n"
        "  datetime.truncate('year', other, {}) AS col5,\n"
        "  datetime.truncate('month', other, {day: 2}) AS col6,\n"
        "  datetime.truncate('month', other, {}) AS col7,\n"
        "  datetime.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
        "  datetime.truncate('week', other, {}) AS col9,\n"
        "  datetime.truncate('day', other, {}) AS col10,\n"
        "  datetime.truncate('day', other, {nanosecond: 2}) AS col11,\n"
        "  datetime.truncate('second', other, {}) AS col12,\n"
        "  datetime.truncate('second', other, {nanosecond: 2}) AS col13,\n"
        "  datetime.truncate('millisecond', other, {}) AS col14,\n"
        "  datetime.truncate('millisecond', other, {nanosecond: 2}) AS col15,\n"
        "  datetime.truncate('microsecond', other, {}) AS col16,\n"
        "  datetime.truncate('microsecond', other, {nanosecond: 2}) AS "
        "col17");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1900-01-02T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1900-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "1980-01-02T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDateTime().ToString(),
                  "1980-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDateTime().ToString(),
                  "1984-01-02T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDateTime().ToString(),
                  "1984-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDateTime().ToString(),
                  "1984-10-02T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDateTime().ToString(),
                  "1984-10-09T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDateTime().ToString(),
                  "1984-10-08T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[11].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000002+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[12].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[13].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.000000002+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[14].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[15].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645000002+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[16].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645876000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[17].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645876002+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '-01:00'}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('decade', other, {timezone: 'Europe/Stockholm'}) "
        "AS col0,\n"
        "   datetime.truncate('year', other, {timezone: 'Europe/Stockholm'}) "
        "AS col1,\n"
        "   datetime.truncate('month', other, {timezone: 'Europe/Stockholm'}) "
        "AS col2,\n"
        "   datetime.truncate('week', other, {timezone: 'Europe/Stockholm'}) "
        "AS col3,\n"
        "   datetime.truncate('day', other, {timezone: 'Europe/Stockholm'}) AS "
        "col4,\n"
        "   datetime.truncate('hour', other, {}) AS col5,\n"
        "   datetime.truncate('hour', other, {nanosecond: 2}) AS col6,\n"
        "   datetime.truncate('hour', other, {timezone: 'Europe/Stockholm'}) "
        "AS col7,\n"
        "   datetime.truncate('minute', other, {}) AS col8,\n"
        "   datetime.truncate('minute', other, {nanosecond: 2}) AS col9,\n"
        "   datetime.truncate('minute', other, {timezone: 'Europe/Stockholm'}) "
        "AS col10");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1980-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1984-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDateTime().ToString(),
                  "1984-10-08T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000000+01:00:00");

        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDateTime().ToString(),
                  "1984-10-11T12:00:00.000000000-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDateTime().ToString(),
                  "1984-10-11T12:00:00.000000002-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDateTime().ToString(),
                  "1984-10-11T12:00:00.000000000+01:00:00");

        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:00.000000000-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:00.000000002-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 1, day: 1, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('weekYear', other, {}) AS col0,\n"
        "   datetime.truncate('weekYear', other, {day: 5}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1983-01-03T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1983-01-05T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 1, day: 1, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '-01:00'}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('weekYear', other, {timezone: "
        "'Europe/Stockholm'}) AS col0");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1983-01-03T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 11, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('quarter', other, {day: 2}) AS col0,\n"
        "  datetime.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1984-10-02T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 11, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '-01:00'}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('quarter', other, {timezone: 'Europe/Stockholm'}) "
        "AS col0");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();
}

TEST(DateTime, truncateLocaldatetime2Datetime) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 2017, month: 10, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   datetime.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "2000-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000Z");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt =
        txn->Execute(&rtx,
                     "WITH localdatetime({year: 2017, month: 10, day: 11, "
                     "hour: 12, minute: 31, "
                     "second: 14, nanosecond: 645876123}) AS other\n"
                     "RETURN\n"
                     "   datetime.truncate('millennium', other, {timezone: "
                     "'Europe/Stockholm'}) AS col0,\n"
                     "   datetime.truncate('century', other, {timezone: "
                     "'Europe/Stockholm'}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "2000-01-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 10, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('century', other, {day: 2}) AS col0,\n"
        "  datetime.truncate('century', other, {}) AS col1,\n"
        "  datetime.truncate('decade', other, {day: 2}) AS col2,\n"
        "  datetime.truncate('decade', other, {}) AS col3,\n"
        "  datetime.truncate('year', other, {day: 2}) AS col4,\n"
        "  datetime.truncate('year', other, {}) AS col5,\n"
        "  datetime.truncate('month', other, {day: 2}) AS col6,\n"
        "  datetime.truncate('month', other, {}) AS col7,\n"
        "  datetime.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
        "  datetime.truncate('week', other, {}) AS col9,\n"
        "  datetime.truncate('day', other, {}) AS col10,\n"
        "  datetime.truncate('day', other, {nanosecond: 2}) AS col11,\n"
        "  datetime.truncate('second', other, {}) AS col12,\n"
        "  datetime.truncate('second', other, {nanosecond: 2}) AS col13,\n"
        "  datetime.truncate('millisecond', other, {}) AS col14,\n"
        "  datetime.truncate('millisecond', other, {nanosecond: 2}) AS col15,\n"
        "  datetime.truncate('microsecond', other, {}) AS col16,\n"
        "  datetime.truncate('microsecond', other, {nanosecond: 2}) AS "
        "col17");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1900-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1900-01-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "1980-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDateTime().ToString(),
                  "1980-01-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDateTime().ToString(),
                  "1984-01-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDateTime().ToString(),
                  "1984-01-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDateTime().ToString(),
                  "1984-10-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDateTime().ToString(),
                  "1984-10-09T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDateTime().ToString(),
                  "1984-10-08T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[11].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000002Z");
        EXPECT_EQ(std::any_cast<Value>(record[12].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[13].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.000000002Z");
        EXPECT_EQ(std::any_cast<Value>(record[14].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[15].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645000002Z");
        EXPECT_EQ(std::any_cast<Value>(record[16].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645876000Z");
        EXPECT_EQ(std::any_cast<Value>(record[17].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:14.645876002Z");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 10, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('decade', other, {timezone: 'Europe/Stockholm'}) "
        "AS col0,\n"
        "   datetime.truncate('year', other, {timezone: 'Europe/Stockholm'}) "
        "AS col1,\n"
        "   datetime.truncate('month', other, {timezone: 'Europe/Stockholm'}) "
        "AS col2,\n"
        "   datetime.truncate('week', other, {timezone: 'Europe/Stockholm'}) "
        "AS col3,\n"
        "   datetime.truncate('day', other, {timezone: 'Europe/Stockholm'}) AS "
        "col4,\n"
        "   datetime.truncate('hour', other, {}) AS col5,\n"
        "   datetime.truncate('hour', other, {nanosecond: 2}) AS col6,\n"
        "   datetime.truncate('hour', other, {timezone: 'Europe/Stockholm'}) "
        "AS col7,\n"
        "   datetime.truncate('minute', other, {}) AS col8,\n"
        "   datetime.truncate('minute', other, {nanosecond: 2}) AS col9,\n"
        "   datetime.truncate('minute', other, {timezone: 'Europe/Stockholm'}) "
        "AS col10");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1980-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1984-01-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[2].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[3].data).AsDateTime().ToString(),
                  "1984-10-08T00:00:00.000000000+01:00:00");
        EXPECT_EQ(std::any_cast<Value>(record[4].data).AsDateTime().ToString(),
                  "1984-10-11T00:00:00.000000000+01:00:00");

        EXPECT_EQ(std::any_cast<Value>(record[5].data).AsDateTime().ToString(),
                  "1984-10-11T12:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[6].data).AsDateTime().ToString(),
                  "1984-10-11T12:00:00.000000002Z");
        // EXPECT_EQ(std::any_cast<Value>(record[7].data).AsDateTime().ToString(),
        //           "1984-10-11T12:00:00.000000000+01:00:00");

        EXPECT_EQ(std::any_cast<Value>(record[8].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[9].data).AsDateTime().ToString(),
                  "1984-10-11T12:31:00.000000002Z");
        // EXPECT_EQ(std::any_cast<Value>(record[10].data).AsDateTime().ToString(),
        //           "1984-10-11T12:31:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 1, day: 1, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   datetime.truncate('weekYear', other, {}) AS col0,\n"
        "   datetime.truncate('weekYear', other, {day: 5}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1983-01-03T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1983-01-05T00:00:00.000000000Z");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt =
        txn->Execute(&rtx,
                     "WITH localdatetime({year: 1984, month: 1, day: 1, hour: "
                     "12, minute: 31, "
                     "second: 14, nanosecond: 645876123}) AS other\n"
                     "RETURN\n"
                     "   datetime.truncate('weekYear', other, {timezone: "
                     "'Europe/Stockholm'}) AS col0");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1983-01-03T00:00:00.000000000+01:00:00");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 11, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('quarter', other, {day: 2}) AS col0,\n"
        "  datetime.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1984-10-02T00:00:00.000000000Z");
        EXPECT_EQ(std::any_cast<Value>(record[1].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000Z");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 11, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "  datetime.truncate('quarter', other, {timezone: 'Europe/Stockholm'}) "
        "AS col0");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(record[0].data).AsDateTime().ToString(),
                  "1984-10-01T00:00:00.000000000+01:00:00");
    }
    txn->Commit();
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

TEST(LocalDateTime, truncateDate2Localdatetime) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 2017, month: 10, day: 11}) AS other\n"
        "RETURN\n"
        "   localdatetime.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   localdatetime.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "2000-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "2000-01-01T00:00:00.000000000");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 1984, month: 10, day: 11}) AS other\n"
        "RETURN\n"
        "  localdatetime.truncate('century', other, {day: 2}) AS col0,\n"
        "  localdatetime.truncate('century', other, {}) AS col1,\n"
        "  localdatetime.truncate('decade', other, {day: 2}) AS col2,\n"
        "  localdatetime.truncate('decade', other, {}) AS col3,\n"
        "  localdatetime.truncate('year', other, {day: 2}) AS col4,\n"
        "  localdatetime.truncate('year', other, {}) AS col5,\n"
        "  localdatetime.truncate('month', other, {day: 2}) AS col6,\n"
        "  localdatetime.truncate('month', other, {}) AS col7,\n"
        "  localdatetime.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
        "  localdatetime.truncate('week', other, {}) AS col9,\n"
        "  localdatetime.truncate('day', other, {}) AS col10,\n"
        "  localdatetime.truncate('day', other, {nanosecond: 2}) AS col11");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1900-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1900-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[2].data).AsLocalDateTime().ToString(),
            "1980-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[3].data).AsLocalDateTime().ToString(),
            "1980-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[4].data).AsLocalDateTime().ToString(),
            "1984-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[5].data).AsLocalDateTime().ToString(),
            "1984-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[6].data).AsLocalDateTime().ToString(),
            "1984-10-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[7].data).AsLocalDateTime().ToString(),
            "1984-10-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[8].data).AsLocalDateTime().ToString(),
            "1984-10-09T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[9].data).AsLocalDateTime().ToString(),
            "1984-10-08T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[10].data).AsLocalDateTime().ToString(),
            "1984-10-11T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[11].data).AsLocalDateTime().ToString(),
            "1984-10-11T00:00:00.000000002");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 1984, month: 2, day: 1}) AS other\n"
        "RETURN\n"
        "   localdatetime.truncate('weekYear', other, {day: 5}) AS col0,\n"
        "   localdatetime.truncate('weekYear', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1984-01-05T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1984-01-02T00:00:00.000000000");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH date({year: 1984, month: 11, day: 11}) AS other\n"
        "RETURN\n"
        "  localdatetime.truncate('quarter', other, {day: 2}) AS col0,\n"
        "  localdatetime.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1984-10-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1984-10-01T00:00:00.000000000");
    }
    txn->Commit();
}

TEST(LocalDateTime, truncateDatetime2Localdatetime) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 2017, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "   localdatetime.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   localdatetime.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "2000-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "2000-01-01T00:00:00.000000000");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 10, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "  localdatetime.truncate('century', other, {day: 2}) AS col0,\n"
        "  localdatetime.truncate('century', other, {}) AS col1,\n"
        "  localdatetime.truncate('decade', other, {day: 2}) AS col2,\n"
        "  localdatetime.truncate('decade', other, {}) AS col3,\n"
        "  localdatetime.truncate('year', other, {day: 2}) AS col4,\n"
        "  localdatetime.truncate('year', other, {}) AS col5,\n"
        "  localdatetime.truncate('month', other, {day: 2}) AS col6,\n"
        "  localdatetime.truncate('month', other, {}) AS col7,\n"
        "  localdatetime.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
        "  localdatetime.truncate('week', other, {}) AS col9,\n"
        "  localdatetime.truncate('day', other, {}) AS col10,\n"
        "  localdatetime.truncate('day', other, {nanosecond: 2}) AS col11,\n"
        "  localdatetime.truncate('hour', other, {}) AS col12,\n"
        "  localdatetime.truncate('hour', other, {nanosecond: 2}) AS col13,\n"
        "  localdatetime.truncate('minute', other, {}) AS col14,\n"
        "  localdatetime.truncate('minute', other, {nanosecond: 2}) AS col15,\n"
        "  localdatetime.truncate('second', other, {}) AS col16,\n"
        "  localdatetime.truncate('second', other, {nanosecond: 2}) AS col17,\n"
        "  localdatetime.truncate('millisecond', other, {}) AS col18,\n"
        "  localdatetime.truncate('millisecond', other, {nanosecond: 2}) AS "
        "col19,\n"
        "  localdatetime.truncate('microsecond', other, {}) AS col20,\n"
        "  localdatetime.truncate('microsecond', other, {nanosecond: 2}) AS "
        "col21");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1900-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1900-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[2].data).AsLocalDateTime().ToString(),
            "1980-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[3].data).AsLocalDateTime().ToString(),
            "1980-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[4].data).AsLocalDateTime().ToString(),
            "1984-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[5].data).AsLocalDateTime().ToString(),
            "1984-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[6].data).AsLocalDateTime().ToString(),
            "1984-10-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[7].data).AsLocalDateTime().ToString(),
            "1984-10-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[8].data).AsLocalDateTime().ToString(),
            "1984-10-09T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[9].data).AsLocalDateTime().ToString(),
            "1984-10-08T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[10].data).AsLocalDateTime().ToString(),
            "1984-10-11T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[11].data).AsLocalDateTime().ToString(),
            "1984-10-11T00:00:00.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[12].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[13].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:00:00.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[14].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[15].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:00.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[16].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[17].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[18].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[19].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[20].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645876000");
        EXPECT_EQ(
            std::any_cast<Value>(record[21].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645876002");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 1, day: 1, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "   localdatetime.truncate('weekYear', other, {}) AS col0,\n"
        "   localdatetime.truncate('weekYear', other, {day: 5}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1983-01-03T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1983-01-05T00:00:00.000000000");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH datetime({year: 1984, month: 11, day: 11, hour: 12, minute: 31, "
        "second: 14, nanosecond: 645876123, timezone: '+01:00'}) AS other\n"
        "RETURN\n"
        "  localdatetime.truncate('quarter', other, {day: 2}) AS col0,\n"
        "  localdatetime.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1984-10-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1984-10-01T00:00:00.000000000");
    }
    txn->Commit();
}

TEST(LocalDateTime, truncateLocaldatetime2Localdatetime) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 2017, month: 10, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   localdatetime.truncate('millennium', other, {day: 2}) AS col0,\n"
        "   localdatetime.truncate('millennium', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "2000-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "2000-01-01T00:00:00.000000000");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 10, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "  localdatetime.truncate('century', other, {day: 2}) AS col0,\n"
        "  localdatetime.truncate('century', other, {}) AS col1,\n"
        "  localdatetime.truncate('decade', other, {day: 2}) AS col2,\n"
        "  localdatetime.truncate('decade', other, {}) AS col3,\n"
        "  localdatetime.truncate('year', other, {day: 2}) AS col4,\n"
        "  localdatetime.truncate('year', other, {}) AS col5,\n"
        "  localdatetime.truncate('month', other, {day: 2}) AS col6,\n"
        "  localdatetime.truncate('month', other, {}) AS col7,\n"
        "  localdatetime.truncate('week', other, {dayOfWeek: 2}) AS col8,\n"
        "  localdatetime.truncate('week', other, {}) AS col9,\n"
        "  localdatetime.truncate('day', other, {}) AS col10,\n"
        "  localdatetime.truncate('day', other, {nanosecond: 2}) AS col11,\n"
        "  localdatetime.truncate('hour', other, {}) AS col12,\n"
        "  localdatetime.truncate('hour', other, {nanosecond: 2}) AS col13,\n"
        "  localdatetime.truncate('minute', other, {}) AS col14,\n"
        "  localdatetime.truncate('minute', other, {nanosecond: 2}) AS col15,\n"
        "  localdatetime.truncate('second', other, {}) AS col16,\n"
        "  localdatetime.truncate('second', other, {nanosecond: 2}) AS col17,\n"
        "  localdatetime.truncate('millisecond', other, {}) AS col18,\n"
        "  localdatetime.truncate('millisecond', other, {nanosecond: 2}) AS "
        "col19,\n"
        "  localdatetime.truncate('microsecond', other, {}) AS col20,\n"
        "  localdatetime.truncate('microsecond', other, {nanosecond: 2}) AS "
        "col21");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1900-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1900-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[2].data).AsLocalDateTime().ToString(),
            "1980-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[3].data).AsLocalDateTime().ToString(),
            "1980-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[4].data).AsLocalDateTime().ToString(),
            "1984-01-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[5].data).AsLocalDateTime().ToString(),
            "1984-01-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[6].data).AsLocalDateTime().ToString(),
            "1984-10-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[7].data).AsLocalDateTime().ToString(),
            "1984-10-01T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[8].data).AsLocalDateTime().ToString(),
            "1984-10-09T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[9].data).AsLocalDateTime().ToString(),
            "1984-10-08T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[10].data).AsLocalDateTime().ToString(),
            "1984-10-11T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[11].data).AsLocalDateTime().ToString(),
            "1984-10-11T00:00:00.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[12].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[13].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:00:00.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[14].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[15].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:00.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[16].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[17].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.000000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[18].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[19].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645000002");
        EXPECT_EQ(
            std::any_cast<Value>(record[20].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645876000");
        EXPECT_EQ(
            std::any_cast<Value>(record[21].data).AsLocalDateTime().ToString(),
            "1984-10-11T12:31:14.645876002");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 1, day: 1, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "   localdatetime.truncate('weekYear', other, {}) AS col0,\n"
        "   localdatetime.truncate('weekYear', other, {day: 5}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1983-01-03T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1983-01-05T00:00:00.000000000");
    }
    txn->Commit();

    txn = graphDB->BeginTransaction();
    resultIt = txn->Execute(
        &rtx,
        "WITH localdatetime({year: 1984, month: 11, day: 11, hour: 12, minute: "
        "31, second: 14, nanosecond: 645876123}) AS other\n"
        "RETURN\n"
        "  localdatetime.truncate('quarter', other, {day: 2}) AS col0,\n"
        "  localdatetime.truncate('quarter', other, {}) AS col1");
    LOG_INFO(resultIt->GetHeader());
    for (; resultIt->Valid(); resultIt->Next()) {
        auto record = resultIt->GetRecord();
        EXPECT_EQ(
            std::any_cast<Value>(record[0].data).AsLocalDateTime().ToString(),
            "1984-10-02T00:00:00.000000000");
        EXPECT_EQ(
            std::any_cast<Value>(record[1].data).AsLocalDateTime().ToString(),
            "1984-10-01T00:00:00.000000000");
    }
    txn->Commit();
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

TEST(Time, timeFromString) {
    EXPECT_EQ(common::Time("21:40:32.142+0100").ToString(), "21:40:32.142000000+01:00:00");
    EXPECT_EQ(common::Time("214032.142Z").ToString(), "21:40:32.142000000Z");
    EXPECT_EQ(common::Time("21:40:32+01:00").ToString(), "21:40:32.000000000+01:00:00");
    EXPECT_EQ(common::Time("214032-0100").ToString(), "21:40:32.000000000-01:00:00");
    EXPECT_EQ(common::Time("21:40-01:30").ToString(), "21:40:00.000000000-01:30:00");
    EXPECT_EQ(common::Time("2140-00:00").ToString(), "21:40:00.000000000Z");
    EXPECT_EQ(common::Time("2140-02").ToString(), "21:40:00.000000000-02:00:00");
    EXPECT_EQ(common::Time("22+18:00").ToString(), "22:00:00.000000000+18:00:00");
}

TEST(Time, timeFromMap) {
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                            {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(789)},
                                            {"millisecond", Value::Integer(123)}, {"microsecond", Value::Integer(456)}})).ToString(),
              "12:31:14.123456789Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                            {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})).ToString(),
              "12:31:14.645876123Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(3)}})).ToString(),
              "12:31:14.000000003Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)}})).ToString(),
              "12:31:14.645876000Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})).ToString(),
              "12:31:14.645000000Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}})).ToString(),
              "12:31:14.000000000Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)}})).ToString(),
              "12:31:00.000000000Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}})).ToString(),
              "12:00:00.000000000Z");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)},
                                       {"timezone", Value::String("+01:00")}})).ToString(),
              "12:31:14.645876123+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                       {"timezone", Value::String("+01:00")}})).ToString(),
              "12:31:14.645876000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)},
                                       {"timezone", Value::String("+01:00")}})).ToString(),
              "12:31:14.645000000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"second", Value::Integer(14)}, {"timezone", Value::String("+01:00")}})).ToString(),
              "12:31:14.000000000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                       {"timezone", Value::String("+01:00")}})).ToString(),
              "12:31:00.000000000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"timezone", Value::String("+01:00")}})).ToString(),
              "12:00:00.000000000+01:00:00");
}

TEST(Time, localtimeNestedMap) {
    EXPECT_EQ(common::LocalTime(Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                               {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                     {"timezone", Value::String("+01:00")}})))).ToString(),
              "12:31:14.645876000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"time", Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                                    {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                                                    {"timezone", Value::String("+01:00")}})))}})).ToString(),
              "12:31:14.645876000");
    EXPECT_EQ(common::LocalTime(Value::Map({
                                    {"time", Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                            {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                                            {"timezone", Value::String("+01:00")}})))},
                                    {"second", Value::Integer(42)}
                                })).ToString(),
              "12:31:42.645876000");
    EXPECT_EQ(common::Time(Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                     {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))).ToString(),
              "12:31:14.645876123Z");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                          {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))}})).ToString(),
              "12:31:14.645876123Z");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                               {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))},
                                       {"timezone", Value::String("+05:00")}})).ToString(),
              "12:31:14.645876123+05:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                               {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))},
                                       {"second", Value::Integer(42)}})).ToString(),
              "12:31:42.645876123Z");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalTime(common::LocalTime(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                               {"second", Value::Integer(14)}, {"nanosecond", Value::Integer(645876123)}})))},
                                       {"second", Value::Integer(42)}, {"timezone", Value::String("+05:00")}})).ToString(),
              "12:31:42.645876123+05:00:00");
    EXPECT_EQ(common::Time(Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                          {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                {"timezone", Value::String("+01:00")}})))).ToString(),
              "12:31:14.645876000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                               {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                                     {"timezone", Value::String("+01:00")}})))}})).ToString(),
              "12:31:14.645876000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                     {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                                     {"timezone", Value::String("+01:00")}})))},
                                       {"timezone", Value::String("+05:00")}})).ToString(),
              "16:31:14.645876000+05:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                     {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                                     {"timezone", Value::String("+01:00")}})))},
                                       {"second", Value::Integer(42)}})).ToString(),
              "12:31:42.645876000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::Time(common::Time(Value::Map({{"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                                                     {"second", Value::Integer(14)}, {"microsecond", Value::Integer(645876)},
                                                                                     {"timezone", Value::String("+01:00")}})))},
                                       {"second", Value::Integer(42)}, {"timezone", Value::String("+05:00")}})).ToString(),
              "16:31:42.645876000+05:00:00");
}

TEST(Time, localdatetimeNestedMap) {
    EXPECT_EQ(common::LocalTime(Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                    {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                    {"dayOfWeek", Value::Integer(3)},
                                    {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                    {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))).ToString(),
              "12:31:14.645000000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"time", Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                    {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                    {"dayOfWeek", Value::Integer(3)},
                                    {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                    {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))}})).ToString(),
              "12:31:14.645000000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"time", Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                    {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                    {"dayOfWeek", Value::Integer(3)},
                                    {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                    {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))},
                                    {"second", Value::Integer(42)}})).ToString(),
              "12:31:42.645000000");
    EXPECT_EQ(common::LocalTime(Value::DateTime(common::DateTime(Value::Map({
                                    {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                    {"day", Value::Integer(11)},
                                    {"hour", Value::Integer(12)}, {"timezone", Value::String("+01:00")}})))).ToString(),
              "12:00:00.000000000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"time", Value::DateTime(common::DateTime(Value::Map({
                                                         {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                                         {"day", Value::Integer(11)},
                                                         {"hour", Value::Integer(12)}, {"timezone", Value::String("+01:00")}})))}})).ToString(),
              "12:00:00.000000000");
    EXPECT_EQ(common::LocalTime(Value::Map({{"time", Value::DateTime(common::DateTime(Value::Map({
                                                         {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                                         {"day", Value::Integer(11)},
                                                         {"hour", Value::Integer(12)}, {"timezone", Value::String("+01:00")}})))},
                                            {"second", Value::Integer(42)}})).ToString(),
              "12:00:42.000000000");

    EXPECT_EQ(common::Time(Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                    {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                    {"dayOfWeek", Value::Integer(3)},
                                    {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                    {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))).ToString(),
              "12:31:14.645000000Z");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                                         {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                                         {"dayOfWeek", Value::Integer(3)},
                                                         {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                         {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))}})).ToString(),
              "12:31:14.645000000Z");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                                         {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                                         {"dayOfWeek", Value::Integer(3)},
                                                         {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                         {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))},
                                            {"timezone", Value::String("+05:00")}})).ToString(),
              "12:31:14.645000000+05:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                                    {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                                    {"dayOfWeek", Value::Integer(3)},
                                                    {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                    {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))},
                                       {"second", Value::Integer(42)}})).ToString(),
              "12:31:42.645000000Z");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                                    {"year", Value::Integer(1984)}, {"week", Value::Integer(10)},
                                                    {"dayOfWeek", Value::Integer(3)},
                                                    {"hour", Value::Integer(12)}, {"minute", Value::Integer(31)},
                                                    {"second", Value::Integer(14)}, {"millisecond", Value::Integer(645)}})))},
                                       {"second", Value::Integer(42)}, {"timezone", Value::String("+05:00")}})).ToString(),
              "12:31:42.645000000+05:00:00");
    EXPECT_EQ(common::Time(Value::DateTime(common::DateTime(Value::Map({
                                    {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                    {"day", Value::Integer(11)},
                                    {"hour", Value::Integer(12)},
                               {"timezone", Value::String("Europe/Stockholm")}})))).ToString(),
              "12:00:00.000000000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::DateTime(common::DateTime(Value::Map({
                                                         {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                                         {"day", Value::Integer(11)},
                                                         {"hour", Value::Integer(12)},
                                                    {"timezone", Value::String("Europe/Stockholm")}})))}})).ToString(),
              "12:00:00.000000000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::DateTime(common::DateTime(Value::Map({
                                                    {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                                    {"day", Value::Integer(11)},
                                                    {"hour", Value::Integer(12)},
                                                    {"timezone", Value::String("Europe/Stockholm")}})))},
                                       {"timezone", Value::String("+05:00")}})).ToString(),
              "16:00:00.000000000+05:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::DateTime(common::DateTime(Value::Map({
                                                         {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                                         {"day", Value::Integer(11)},
                                                         {"hour", Value::Integer(12)}, {"timezone", Value::String("+01:00")}})))},
                                            {"second", Value::Integer(42)}})).ToString(),
              "12:00:42.000000000+01:00:00");
    EXPECT_EQ(common::Time(Value::Map({{"time", Value::DateTime(common::DateTime(Value::Map({
                                                    {"year", Value::Integer(1984)}, {"month", Value::Integer(10)},
                                                    {"day", Value::Integer(11)},
                                                    {"hour", Value::Integer(12)}, {"timezone", Value::String("+01:00")}})))},
                                       {"second", Value::Integer(42)}, {"timezone", Value::String("+05:00")}})).ToString(),
              "16:00:42.000000000+05:00:00");
}

TEST(Time, truncate) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIterator = txn->Execute(&rtx, "WITH time({hour: 12, minute: 31, second: 14, nanosecond: 645876123, timezone: '-01:00'}) AS t\n"
                 "RETURN\n"
                 "  localtime.truncate('day', t) AS truncDay,\n"
                 "  localtime.truncate('hour', t) AS truncHour,\n"
                 "  localtime.truncate('minute', t, {millisecond: 1}) AS truncMinute,\n"
                 "  localtime.truncate('second', t) AS truncSecond,\n"
                 "  localtime.truncate('millisecond', t) AS truncMillisecond,\n"
                 "  localtime.truncate('microsecond', t) AS truncMicrosecond");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(records[0].data).AsLocalTime().ToString(), "00:00:00.000000000");
        EXPECT_EQ(std::any_cast<Value>(records[1].data).AsLocalTime().ToString(), "12:00:00.000000000");
        EXPECT_EQ(std::any_cast<Value>(records[2].data).AsLocalTime().ToString(), "12:31:00.001000000");
        EXPECT_EQ(std::any_cast<Value>(records[3].data).AsLocalTime().ToString(), "12:31:14.000000000");
        EXPECT_EQ(std::any_cast<Value>(records[4].data).AsLocalTime().ToString(), "12:31:14.645000000");
        EXPECT_EQ(std::any_cast<Value>(records[5].data).AsLocalTime().ToString(), "12:31:14.645876000");
    }
    txn->Commit();
    txn = graphDB->BeginTransaction();
    resultIterator = txn->Execute(&rtx, "WITH time({hour: 12, minute: 31, second: 14, nanosecond: 645876123, timezone: '-01:00'}) AS t\n"
                                  "RETURN\n"
                                  "  time.truncate('day', t) AS truncDay,\n"
                                  "  time.truncate('hour', t) AS truncHour,\n"
                                  "  time.truncate('minute', t) AS truncMinute,\n"
                                  "  time.truncate('second', t) AS truncSecond,\n"
                                  "  time.truncate('millisecond', t, {nanosecond: 2}) AS truncMillisecond,\n"
                                  "  time.truncate('microsecond', t) AS truncMicrosecond");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(records[0].data).AsTime().ToString(), "00:00:00.000000000-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(records[1].data).AsTime().ToString(), "12:00:00.000000000-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(records[2].data).AsTime().ToString(), "12:31:00.000000000-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(records[3].data).AsTime().ToString(), "12:31:14.000000000-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(records[4].data).AsTime().ToString(), "12:31:14.645000002-01:00:00");
        EXPECT_EQ(std::any_cast<Value>(records[5].data).AsTime().ToString(), "12:31:14.645876000-01:00:00");
    }
    txn->Commit();
}

TEST(Time, createVE) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    txn->Execute(&rtx, "CREATE (:Val {date: date({year: 1984, month: 10, day: 11})});")->Consume();
    auto resultIterator = txn->Execute(&rtx, "MATCH (n:Val) return n.date;");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(records[0].data).AsDate().ToString(), "1984-10-11");
    }
    txn->Commit();
}

TEST(Duration, fromMap) {
    EXPECT_EQ(common::Duration(Value({{"days", Value::Integer(14)}, {"hours", Value::Integer(16)},
                                      {"minutes", Value::Integer(12)}})).ToString(),
              "P14DT16H12M");
    EXPECT_EQ(common::Duration(Value({{"months", Value::Integer(5)}, {"days", Value::Double(1.5)}})).ToString(),
              "P5M1DT12H");
    EXPECT_EQ(common::Duration(Value({{"months", Value::Double(0.75)}})).ToString(),
              "P22DT19H51M49.5S");
    EXPECT_EQ(common::Duration(Value({{"weeks", Value::Double(2.5)}})).ToString(),
              "P17DT12H");
    EXPECT_EQ(common::Duration(Value({{"years", Value::Integer(12)}, {"months", Value::Integer(5)},
                                      {"days", Value::Integer(14)}, {"hours", Value::Integer(16)},
                                      {"minutes", Value::Integer(12)}, {"seconds", Value::Integer(70)}})).ToString(),
              "P12Y5M14DT16H13M10S");
    EXPECT_EQ(common::Duration(Value({{"days", Value::Integer(14)}, {"seconds", Value::Integer(70)},
                                      {"milliseconds", Value::Integer(1)}})).ToString(),
              "P14DT1M10.001S");
    EXPECT_EQ(common::Duration(Value({{"days", Value::Integer(14)}, {"seconds", Value::Integer(70)},
                                      {"microseconds", Value::Integer(1)}})).ToString(),
              "P14DT1M10.000001S");
    EXPECT_EQ(common::Duration(Value({{"days", Value::Integer(14)}, {"seconds", Value::Integer(70)},
                                      {"nanoseconds", Value::Integer(1)}})).ToString(),
              "P14DT1M10.000000001S");
    EXPECT_EQ(common::Duration(Value({{"minutes", Value::Double(1.5)}, {"seconds", Value::Integer(1)}})).ToString(),
              "PT1M31S");
}

TEST(Duration, durationFromString) {
    EXPECT_EQ(common::Duration("P14DT16H12M").ToString(), "P14DT16H12M");
    EXPECT_EQ(common::Duration("P5M1.5D").ToString(), "P5M1DT12H");
    EXPECT_EQ(common::Duration("P0.75M").ToString(), "P22DT19H51M49.5S");
    EXPECT_EQ(common::Duration("PT0.75M").ToString(), "PT45S");
    EXPECT_EQ(common::Duration("P2.5W").ToString(), "P17DT12H");
    EXPECT_EQ(common::Duration("P12Y5M14DT16H12M70S").ToString(), "P12Y5M14DT16H13M10S");
    EXPECT_EQ(common::Duration("P2012-02-02T14:37:21.545").ToString(), "P2012Y2M2DT14H37M21.545S");
}

TEST(Duration, between) {
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2018-01-01T12:00")),
                                        Value::LocalDateTime(common::LocalDateTime("2018-01-02T10:00"))).ToString(), "PT22H");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2018-01-02T10:00")),
                                        Value::LocalDateTime(common::LocalDateTime("2018-01-01T12:00"))).ToString(), "PT-22H");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2018-01-01T10:00:00.2")),
                                        Value::LocalDateTime(common::LocalDateTime("2018-01-02T10:00:00.1"))).ToString(), "PT23H59M59.9S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2018-01-02T10:00:00.1")),
                                        Value::LocalDateTime(common::LocalDateTime("2018-01-01T10:00:00.2"))).ToString(), "PT-23H-59M-59.9S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2017-10-28T23:00+02:00")),
                                        Value::DateTime(common::DateTime("2017-10-29T04:00+01:00"))).ToString(), "PT6H");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2017-10-29T04:00+01:00")),
                                        Value::DateTime(common::DateTime("2017-10-28T23:00+02:00"))).ToString(), "PT-6H");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Date(common::Date("2015-06-24"))).ToString(), "P30Y8M13D");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142"))).ToString(), "P31Y9M10DT21H45M22.142S");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100"))).ToString(), "P30Y9M10DT21H40M32.142S");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalTime(common::LocalTime("16:30"))).ToString(), "PT16H30M");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Time(common::Time("16:30+0100"))).ToString(), "PT16H30M");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::Date(common::Date("2015-06-24"))).ToString(), "PT-14H-30M");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142"))).ToString(), "PT7H15M22.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100"))).ToString(), "PT7H10M32.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::LocalTime(common::LocalTime("16:30"))).ToString(), "PT2H");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::Time(common::Time("16:30+0100"))).ToString(), "PT2H");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::Date(common::Date("2015-06-24"))).ToString(), "PT-14H-30M");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142"))).ToString(), "PT7H15M22.142S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100"))).ToString(), "PT6H10M32.142S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::LocalTime(common::LocalTime("16:30"))).ToString(), "PT2H");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::Time(common::Time("16:30+0100"))).ToString(), "PT1H");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Date(common::Date("2015-06-24"))).ToString(), "P-27DT-21H-40M-32.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142"))).ToString(), "P1YT4M50S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100"))).ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalTime(common::LocalTime("16:30"))).ToString(), "PT-5H-10M-32.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Time(common::Time("16:30+0100"))).ToString(), "PT-5H-10M-32.142S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Date(common::Date("2015-06-24"))).ToString(), "P11M2DT2H19M23.857S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142"))).ToString(), "P2YT4M45.999S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100"))).ToString(), "P1YT59M55.999S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalTime(common::LocalTime("16:30"))).ToString(), "PT-5H-10M-36.143S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Time(common::Time("16:30+0100"))).ToString(), "PT-4H-10M-36.143S");
//    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("-999999999-01-01")),
//                                        Value::Date(common::Date("+999999999-12-31"))).ToString(), "P1999999998Y11M30D");
}

TEST(Duration, betweenFunction) {
    // months
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Date(common::Date("2015-06-24")), "MONTH").ToString(), "P30Y8M");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "MONTH").ToString(), "P31Y9M");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "MONTH").ToString(), "P30Y9M");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalTime(common::LocalTime("16:30")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Time(common::Time("16:30+0100")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::Date(common::Date("2015-06-24")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::Date(common::Date("2015-06-24")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Date(common::Date("2015-06-24")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "MONTH").ToString(), "P1Y");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalTime(common::LocalTime("16:30")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Time(common::Time("16:30+0100")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Date(common::Date("2015-06-24")), "MONTH").ToString(), "P11M");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "MONTH").ToString(), "P2Y");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "MONTH").ToString(), "P1Y");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalTime(common::LocalTime("16:30")), "MONTH").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Time(common::Time("16:30+0100")), "MONTH").ToString(), "PT0S");
    // days
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Date(common::Date("2015-06-24")), "DAY").ToString(), "P11213D");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "DAY").ToString(), "P11606D");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "DAY").ToString(), "P11240D");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalTime(common::LocalTime("16:30")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Time(common::Time("16:30+0100")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::Date(common::Date("2015-06-24")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::Date(common::Date("2015-06-24")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Date(common::Date("2015-06-24")), "DAY").ToString(), "P-27D");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "DAY").ToString(), "P366D");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalTime(common::LocalTime("16:30")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Time(common::Time("16:30+0100")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Date(common::Date("2015-06-24")), "DAY").ToString(), "P337D");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "DAY").ToString(), "P731D");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "DAY").ToString(), "P365D");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalTime(common::LocalTime("16:30")), "DAY").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Time(common::Time("16:30+0100")), "DAY").ToString(), "PT0S");

    // seconds
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Date(common::Date("2015-06-24")), "SECOND").ToString(), "PT269112H");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "SECOND").ToString(), "PT278565H45M22.142S");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "SECOND").ToString(), "PT269781H40M32.142S");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::LocalTime(common::LocalTime("16:30")), "SECOND").ToString(), "PT16H30M");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("1984-10-11")),
                                        Value::Time(common::Time("16:30+0100")), "SECOND").ToString(), "PT16H30M");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::Date(common::Date("2015-06-24")), "SECOND").ToString(), "PT-14H-30M");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "SECOND").ToString(), "PT7H15M22.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "SECOND").ToString(), "PT7H10M32.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::LocalTime(common::LocalTime("16:30")), "SECOND").ToString(), "PT2H");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("14:30")),
                                        Value::Time(common::Time("16:30+0100")), "SECOND").ToString(), "PT2H");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::Date(common::Date("2015-06-24")), "SECOND").ToString(), "PT-14H-30M");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "SECOND").ToString(), "PT7H15M22.142S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "SECOND").ToString(), "PT6H10M32.142S");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::LocalTime(common::LocalTime("16:30")), "SECOND").ToString(), "PT2H");
    EXPECT_EQ(common::Duration::between(Value::Time(common::Time("14:30")),
                                        Value::Time(common::Time("16:30+0100")), "SECOND").ToString(), "PT1H");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Date(common::Date("2015-06-24")), "SECOND").ToString(), "PT-669H-40M-32.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "SECOND").ToString(), "PT8784H4M50S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "SECOND").ToString(), "PT0S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::LocalTime(common::LocalTime("16:30")), "SECOND").ToString(), "PT-5H-10M-32.142S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2015-07-21T21:40:32.142")),
                                        Value::Time(common::Time("16:30+0100")), "SECOND").ToString(), "PT-5H-10M-32.142S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Date(common::Date("2015-06-24")), "SECOND").ToString(), "PT8090H19M23.857S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:45:22.142")), "SECOND").ToString(), "PT17544H4M45.999S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::DateTime(common::DateTime("2015-07-21T21:40:32.142+0100")), "SECOND").ToString(), "PT8760H59M55.999S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::LocalTime(common::LocalTime("16:30")), "SECOND").ToString(), "PT-5H-10M-36.143S");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2014-07-21T21:40:36.143+0200")),
                                        Value::Time(common::Time("16:30+0100")), "SECOND").ToString(), "PT-4H-10M-36.143S");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2014-07-21T21:40:36.143")),
                                        Value::LocalDateTime(common::LocalDateTime("2014-07-21T21:40:36.142")), "SECOND").ToString(), "PT-0.001S");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("2018-03-11")),
                                        Value::Date(common::Date("2016-06-24")), "MONTH").ToString(), "P-1Y-8M");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date("2018-07-21")),
                                        Value::DateTime(common::DateTime("2016-07-21T21:40:32.142+0100")), "MONTH").ToString(), "P-1Y-11M");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime("2018-07-21T21:40:32.142")),
                                        Value::Date(common::Date("2016-07-21")), "MONTH").ToString(), "P-2Y");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2018-07-21T21:40:36.143+0200")),
                                        Value::LocalDateTime(common::LocalDateTime("2016-07-21T21:40:36.143")), "MONTH").ToString(), "P-2Y");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime("2018-07-21T21:40:36.143+0500")),
                                        Value::DateTime(common::DateTime("1984-07-21T22:40:36.143+0200")), "MONTH").ToString(), "P-33Y-11M");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(0)},
                                            {"timezone", Value::String("Europe/Stockholm")}
                                        }))),
                                        Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(4)},
                                        }))), "SECOND").ToString(), "PT4H");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(0)},
                                            {"timezone", Value::String("Europe/Stockholm")}
                                        }))),
                                        Value::LocalTime(common::LocalTime(Value::Map({
                                            {"hour", Value::Integer(4)},
                                        }))), "SECOND").ToString(), "PT4H");
    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(0)}
                                        }))),
                                        Value::DateTime(common::DateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(4)},
                                            {"timezone", Value::String("Europe/Stockholm")}
                                        }))), "SECOND").ToString(), "PT4H");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime(Value::Map({
                                            {"hour", Value::Integer(0)}
                                        }))),
                                        Value::DateTime(common::DateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(4)},
                                            {"timezone", Value::String("Europe/Stockholm")}
                                        }))), "SECOND").ToString(), "PT4H");
    EXPECT_EQ(common::Duration::between(Value::Date(common::Date(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)}
                                        }))),
                                        Value::DateTime(common::DateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(4)},
                                            {"timezone", Value::String("Europe/Stockholm")}
                                        }))), "SECOND").ToString(), "PT4H");
    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(29)},
                                            {"hour", Value::Integer(0)},
                                            {"timezone", Value::String("Europe/Stockholm")}
                                        }))),
                                        Value::Date(common::Date(Value::Map({
                                            {"year", Value::Integer(2017)},
                                            {"month", Value::Integer(10)},
                                            {"day", Value::Integer(30)}
                                        }))), "SECOND").ToString(), "PT24H");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:54.7")),
                                        Value::LocalTime(common::LocalTime("12:34:54.3")), "SECOND").ToString(), "PT-0.4S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:54.3")),
                                        Value::LocalTime(common::LocalTime("12:34:54.7")), "SECOND").ToString(), "PT0.4S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:54.7")),
                                        Value::LocalTime(common::LocalTime("12:34:55.3")), "SECOND").ToString(), "PT0.6S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:54.7")),
                                        Value::LocalTime(common::LocalTime("12:44:55.3")), "SECOND").ToString(), "PT10M0.6S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:44:54.7")),
                                        Value::LocalTime(common::LocalTime("12:34:55.3")), "SECOND").ToString(), "PT-9M-59.4S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:56")),
                                        Value::LocalTime(common::LocalTime("12:34:55.7")), "SECOND").ToString(), "PT-0.3S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:56")),
                                        Value::LocalTime(common::LocalTime("12:44:55.7")), "SECOND").ToString(), "PT9M59.7S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:44:56")),
                                        Value::LocalTime(common::LocalTime("12:34:55.7")), "SECOND").ToString(), "PT-10M-0.3S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:56.3")),
                                        Value::LocalTime(common::LocalTime("12:34:54.7")), "SECOND").ToString(), "PT-1.6S");
    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime("12:34:54.7")),
                                        Value::LocalTime(common::LocalTime("12:34:56.3")), "SECOND").ToString(), "PT1.6S");
//    EXPECT_EQ(common::Duration::between(Value::LocalTime(common::LocalTime()),
//                                        Value::LocalTime(common::LocalTime()), "SECOND").ToString(), "PT0S");
//    EXPECT_EQ(common::Duration::between(Value::Time(common::Time()),
//                                        Value::Time(common::Time()), "SECOND").ToString(), "PT0S");
//    EXPECT_EQ(common::Duration::between(Value::Date(common::Date()),
//                                        Value::Date(common::Date()), "SECOND").ToString(), "PT0S");
//    EXPECT_EQ(common::Duration::between(Value::LocalDateTime(common::LocalDateTime()),
//                                        Value::LocalDateTime(common::LocalDateTime()), "SECOND").ToString(), "PT0S");
//    EXPECT_EQ(common::Duration::between(Value::DateTime(common::DateTime()),
//                                        Value::DateTime(common::DateTime()), "SECOND").ToString(), "PT0S");
}

TEST(Date, getField) {
    fs::remove_all(test_db);
    auto graphDB = GraphDB::Open(test_db, {});
    cypher::RTContext rtx;
    auto txn = graphDB->BeginTransaction();
    auto resultIterator = txn->Execute(&rtx, "with date({year: 1984, month: 10, day: 11}) as d return d.year, d.quarter, "
                                       "d.month, d.week, d.weekYear, d.day, d.ordinalDay, d.weekDay, d.dayOfQuarter;");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(records[0].data).AsInteger(), 1984);
        EXPECT_EQ(std::any_cast<Value>(records[1].data).AsInteger(), 4);
        EXPECT_EQ(std::any_cast<Value>(records[2].data).AsInteger(), 10);
        EXPECT_EQ(std::any_cast<Value>(records[3].data).AsInteger(), 41);
        EXPECT_EQ(std::any_cast<Value>(records[4].data).AsInteger(), 1984);
        EXPECT_EQ(std::any_cast<Value>(records[5].data).AsInteger(), 11);
        EXPECT_EQ(std::any_cast<Value>(records[6].data).AsInteger(), 285);
        EXPECT_EQ(std::any_cast<Value>(records[7].data).AsInteger(), 4);
        EXPECT_EQ(std::any_cast<Value>(records[8].data).AsInteger(), 11);
    }
    resultIterator = txn->Execute(&rtx, "with date({year: 1984, month: 1, day: 1}) as d return d.year, d.weekYear, "
                                  "d.week, d.weekDay;");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(records[0].data).AsInteger(), 1984);
        EXPECT_EQ(std::any_cast<Value>(records[1].data).AsInteger(), 1983);
        EXPECT_EQ(std::any_cast<Value>(records[2].data).AsInteger(), 52);
        EXPECT_EQ(std::any_cast<Value>(records[3].data).AsInteger(), 7);
    }
    resultIterator = txn->Execute(&rtx, "with localtime({hour: 12, minute: 31, second: 14, nanosecond: 645876123}) as d return d.hour, d.minute, "
                                  "d.second, d.millisecond, d.microsecond, d.nanosecond;");
    LOG_INFO(resultIterator->GetHeader());
    for (; resultIterator->Valid(); resultIterator->Next()) {
        auto records = resultIterator->GetRecord();
        EXPECT_EQ(std::any_cast<Value>(records[0].data).AsInteger(), 12);
        EXPECT_EQ(std::any_cast<Value>(records[1].data).AsInteger(), 31);
        EXPECT_EQ(std::any_cast<Value>(records[2].data).AsInteger(), 14);
        EXPECT_EQ(std::any_cast<Value>(records[3].data).AsInteger(), 645);
        EXPECT_EQ(std::any_cast<Value>(records[4].data).AsInteger(), 645876);
        EXPECT_EQ(std::any_cast<Value>(records[5].data).AsInteger(), 645876123);
    }
    txn->Commit();
}