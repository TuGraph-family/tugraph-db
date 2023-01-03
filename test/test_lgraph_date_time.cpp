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

#include "fma-common/date.h"
#include "fma-common/logger.h"
#include "gtest/gtest.h"

#include "lgraph/lgraph_date_time.h"
#include "./ut_utils.h"

class TestDateTime : public TuGraphTest {};

int64_t Floor2Day(int64_t seconds_since_epoch) {
    return seconds_since_epoch - (seconds_since_epoch % (24 * 3600));
}

TEST_F(TestDateTime, DateTime) {
    using namespace lgraph_api;

    {
        Date d(std::chrono::system_clock::now());
        UT_LOG() << d.ToString();
        DateTime dt(std::chrono::system_clock::now());
        UT_LOG() << dt.ToString();
        UT_LOG() << "Date min: " << Date("0000-01-01").DaysSinceEpoch();
        UT_LOG() << "Date max: " << Date("9999-12-31").DaysSinceEpoch();
        UT_LOG() << "DateTime min: " << DateTime("0000-01-01 00:00:00").SecondsSinceEpoch();
        UT_LOG() << "DateTime max: " << DateTime("9999-12-31 23:59:59").SecondsSinceEpoch();
    }

    {{UT_LOG() << "Testing constructors...";
    const std::string dstr = "2019-12-29";
    Date d(dstr);
    int32_t days_since_epoch = d.DaysSinceEpoch();
    Date d2(days_since_epoch);
    UT_EXPECT_EQ(d, d2);
    UT_EXPECT_EQ(d2.ToString(), dstr);

    const std::string dtstr = "2019-02-29 11:23:34";
    DateTime dt(dtstr);
    DateTime dt2 = DateTime(dt.SecondsSinceEpoch());
    UT_EXPECT_EQ(dt, dt2);
    UT_EXPECT_EQ(dt.ToString(),
                 "2019-03-01 11:23:34");  // there is no 02-29 in 2019
    UT_EXPECT_ANY_THROW(DateTime("2019-03-01 11:3:34"));
}

{
    UT_LOG() << "Testing min and max...";
    Date dmin("0000-01-01");
    UT_EXPECT_EQ(dmin.ToString(), "0000-01-01");
    UT_EXPECT_EQ(Date(MinDaysSinceEpochForDate()), dmin);
    UT_EXPECT_EQ(dmin.DaysSinceEpoch(), MinDaysSinceEpochForDate());
    Date dmax("9999-12-31");
    UT_EXPECT_EQ(dmax.ToString(), "9999-12-31");
    UT_EXPECT_EQ(dmax, Date(MaxDaysSinceEpochForDate()));
    UT_EXPECT_EQ(dmax.DaysSinceEpoch(), MaxDaysSinceEpochForDate());
    UT_EXPECT_EQ(Date("1969-09-01").ToString(), "1969-09-01");

    DateTime dtmin("0000-01-01 00:00:00");
    UT_EXPECT_EQ(dtmin.ToString(), "0000-01-01 00:00:00");
    UT_EXPECT_EQ(dtmin.SecondsSinceEpoch(), MinSecondsSinceEpochForDateTime());
    UT_EXPECT_EQ(dtmin, DateTime(MinSecondsSinceEpochForDateTime()));
    DateTime dtmax("9999-12-31 23:59:59");
    UT_EXPECT_EQ(dtmax.ToString(), "9999-12-31 23:59:59");
    UT_EXPECT_EQ(dtmax.SecondsSinceEpoch(), MaxSecondsSinceEpochForDateTime());
    UT_EXPECT_EQ(dtmax, DateTime(MaxSecondsSinceEpochForDateTime()));
    UT_EXPECT_EQ(DateTime("1969-09-01 09:58:45").ToString(), "1969-09-01 09:58:45");
}

{
    UT_LOG() << "Testing parse...";
    Date d;
    UT_EXPECT_TRUE(Date::Parse("1234-11-11", d));
    UT_EXPECT_TRUE(!Date::Parse("12-11-11", d));    // year
    UT_EXPECT_TRUE(!Date::Parse("-12-11-11", d));   // year
    UT_EXPECT_TRUE(!Date::Parse("1234-31-11", d));  // month
    UT_EXPECT_TRUE(!Date::Parse("1234-1-11", d));   // month
    UT_EXPECT_TRUE(!Date::Parse("1234-01-32", d));  // day
    UT_EXPECT_TRUE(!Date::Parse("1234-01-2", d));   // day
    UT_EXPECT_TRUE(!Date::Parse("123401-32", d));   // -
    UT_EXPECT_TRUE(!Date::Parse("1234-0132", d));   // -

    DateTime dt;
    UT_EXPECT_TRUE(DateTime::Parse("1234-11-11 23:59:33", dt));
    UT_EXPECT_TRUE(!DateTime::Parse("123-11-11 23:59:33", dt));   // year
    UT_EXPECT_TRUE(!DateTime::Parse("1234-21-11 23:59:33", dt));  // month
    UT_EXPECT_TRUE(!DateTime::Parse("1234-12-1 23:59:33", dt));   // day
    UT_EXPECT_TRUE(!DateTime::Parse("1234-12-1123:59:33", dt));   //
    UT_EXPECT_TRUE(!DateTime::Parse("1234-12-11 24:59:33", dt));  // hour
    UT_EXPECT_TRUE(!DateTime::Parse("1234-12-11 24:9:33", dt));   // minute
    UT_EXPECT_TRUE(!DateTime::Parse("1234-12-11 24:59:73", dt));  // second
    UT_EXPECT_TRUE(!DateTime::Parse("1234-12-11 2459:33", dt));   // :
    UT_EXPECT_TRUE(!DateTime::Parse("1234-12-11 24-59:33", dt));  // :
}

{
    UT_LOG() << "Testing compare...";
    UT_EXPECT_TRUE(Date("1970-01-02") > Date());
    UT_EXPECT_TRUE(Date("1969-12-31") < Date());
    UT_EXPECT_TRUE(Date("1970-01-02") > Date("1900-01-02"));
    UT_EXPECT_TRUE(Date("0017-01-02") <= Date("1900-01-02"));
    auto tp = Date("1998-12-31");
    UT_EXPECT_TRUE(Date(tp) == Date(tp));
    UT_EXPECT_TRUE(tp == tp + 0);
    UT_EXPECT_TRUE(tp + 1 == Date("1999-01-01"));
    UT_EXPECT_TRUE(tp != tp + 1);
    UT_EXPECT_TRUE(tp + 365 == Date("1999-12-31"));

    DateTime epoch;
    UT_EXPECT_TRUE(DateTime("1970-01-01 00:00:00") == epoch);
    UT_EXPECT_TRUE(DateTime("1970-01-01 00:00:01") > epoch);
    UT_EXPECT_TRUE(DateTime("1970-01-01 00:00:01") >= epoch);
    UT_EXPECT_TRUE(DateTime("1970-01-01 00:00:59") < epoch.operator+(60));
    UT_EXPECT_TRUE(DateTime("1970-01-01 01:00:00") <= epoch.operator+(3600));
    UT_EXPECT_TRUE(DateTime("1998-01-01 01:00:00").operator+(365 * 24 * 3600) ==
                   DateTime("1999-01-01 01:00:00"));
    UT_EXPECT_NE(epoch, epoch.operator+(1));
}

{
    using namespace std::chrono;
    UT_LOG() << "Testing convert...";
    UT_EXPECT_EQ(Date().TimePoint(), std::chrono::system_clock::time_point());
    UT_EXPECT_EQ(DateTime().TimePoint(), std::chrono::system_clock::time_point());
    auto now = std::chrono::system_clock::now();
    Date d(now);
    UT_EXPECT_EQ(d.TimePoint(), ::date::floor<::date::days>(now));
    DateTime dt(now);
    UT_EXPECT_EQ(dt.TimePoint(), ::date::floor<::std::chrono::seconds>(now));
    UT_EXPECT_EQ((Date)dt, d);
    int64_t seconds_since_epoch =
        Floor2Day(std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count());
    DateTime d_round(seconds_since_epoch);
    UT_EXPECT_EQ((DateTime)d, d_round);

    UT_EXPECT_EQ(Date("1998-01-01"), (Date)DateTime("1998-01-01 00:00:01"));
    UT_EXPECT_EQ(Date("1998-01-01"), (Date)DateTime("1998-01-01 23:59:59"));

    system_clock::time_point epoch;
    Date oldd("1899-12-30");
    UT_EXPECT_EQ(oldd.TimePoint(), epoch + seconds(oldd.DaysSinceEpoch() * 24 * 3600));
    DateTime olddate("1899-12-30 12:30:31");
    UT_EXPECT_EQ(olddate.TimePoint(), epoch + seconds(olddate.SecondsSinceEpoch()));
    UT_EXPECT_EQ(oldd, (Date)olddate);

    UT_EXPECT_EQ(Date("1298-01-01"), (Date)DateTime("1298-01-01 00:00:01"));
    UT_EXPECT_EQ(Date("1198-01-01"), (Date)DateTime("1198-01-01 23:59:59"));
    UT_EXPECT_EQ(DateTime(olddate.GetYMDHMS()).ToString(), "1899-12-30 12:30:31");
    UT_EXPECT_EQ(DateTime(olddate.TimePoint()).ToString(), "1899-12-30 12:30:31");
    UT_LOG() << DateTime().ToString();
    DateTime newdate = olddate + 1;
    UT_EXPECT_TRUE(newdate > olddate);
    UT_EXPECT_TRUE(newdate >= olddate);
    newdate = olddate - 1;
    UT_EXPECT_TRUE(newdate < olddate);
    UT_EXPECT_TRUE(newdate <= olddate);
    UT_EXPECT_NE(newdate, olddate);
    olddate += 1;
    olddate -= 1;
    UT_EXPECT_EQ(olddate.ToString(), "1899-12-30 12:30:31");
}
}

{
    UT_LOG() << "Testing LocalTime...";
    TimeZone tz = TimeZone::LocalTimeZone();
    UT_LOG() << "Time diff from UTC: " << tz.UTCDiffSeconds();
    TimeZone::UpdateLocalTimeZone();
    UT_LOG() << "Time diff from UTC: " << tz.UTCDiffSeconds();

    DateTime local_now = DateTime::LocalNow();
    UT_LOG() << "Local now: " << local_now.ToString();

    DateTime utc_now = DateTime::Now();
    UT_LOG() << "UTC now: " << utc_now.ToString();

    DateTime utc_now2 = utc_now;
    DateTime local_now2 = local_now;

    UT_EXPECT_EQ(utc_now.ConvertToLocal(), local_now);
    UT_EXPECT_EQ(utc_now2, local_now.ConvertToUTC());

    Date local_day = Date::LocalNow();
    UT_LOG() << "Local day: " << local_day.ToString();

    Date utc_day = Date::Now();
    UT_LOG() << "UTC day: " << utc_day.ToString();

    UT_EXPECT_ANY_THROW(TimeZone(-25));
    UT_EXPECT_ANY_THROW(TimeZone(25));
    UT_EXPECT_EQ(local_now2, tz.FromUTC(tz.ToUTC(local_now2)));
    UT_EXPECT_EQ(DateTime(tz.ToUTC(local_now2).ToString()), utc_now2);

    tz = TimeZone(14);
    UT_EXPECT_EQ(tz.FromUTC(DateTime("2020-01-02 10:05:06")).ToString(), "2020-01-03 00:05:06");
    tz = TimeZone(-10);
    UT_EXPECT_EQ(tz.FromUTC(DateTime("2020-01-02 07:05:06")).ToString(), "2020-01-01 21:05:06");
}
}
