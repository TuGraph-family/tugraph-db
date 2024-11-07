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

#include <date/tz.h>

#include "common/exceptions.h"
#include "common/temporal/date.h"
#include "common/value.h"

namespace {

// Regular expression pattern for parsing dates
const std::string DATE_PATTERN =
    "(?:([0-9]{4})(?:([0-9]{2})([0-9]{2})?|W([0-9]{2})([0-9])?|Q([0-9])([0-9]{"
    "2})?|([0-9]{3}))|((?:[0-9]{4}|[+-][0-9]{1,9}))(?:-([0-9]{1,2})(?:-([0-9]{"
    "1,2}))?|-?W([0-9]{1,2})(?:-([0-9]))?|-?Q([0-9])(?:-([0-9]{1,2}))?|-([0-9]{"
    "3}))?)";

constexpr int SHORT_YEAR = 1;
constexpr int SHORT_MONTH = 2;
constexpr int SHORT_DAY = 3;
constexpr int SHORT_WEEK = 4;
constexpr int SHORT_DOW = 5;
constexpr int SHORT_QUARTER = 6;
constexpr int SHORT_DOQ = 7;
constexpr int SHORT_DOY = 8;
constexpr int LONG_YEAR = 9;
constexpr int LONG_MONTH = 10;
constexpr int LONG_DAY = 11;
constexpr int LONG_WEEK = 12;
constexpr int LONG_DOW = 13;
constexpr int LONG_QUARTER = 14;
constexpr int LONG_DOQ = 15;
constexpr int LONG_DOY = 16;

const std::regex DATE_REGEX(DATE_PATTERN);
}  // namespace

namespace common {

void Date::fromYearMonthDay(int year, unsigned month, unsigned day) {
    date::year_month_day ymd = date::year_month_day{
        date::year{year}, date::month{month}, date::day{day}};
    if (ymd.ok()) {
        date::local_days ld = (date::local_days)ymd;
        days_since_epoch_ = ld.time_since_epoch().count();
    } else {
        std::ostringstream oss;
        oss << ymd;
        THROW_CODE(InputError, "{}", oss.str());
    }
}

void Date::fromYearWeekDow(int year, int week, int dow) {
    auto ymd = date::year{year} / 1 / 1;
    if (ymd.ok()) {
        date::local_days ld = (date::local_days)ymd;
        auto wkd = date::weekday(ld);
        auto wkd_index = wkd.c_encoding();
        int days = (week - 1) * 7 + dow - wkd_index;
        if (days < 0) {
            fromYearMonthDay(year - 1, 12, 31 + days + 1);
        } else {
            fromYearDoy(year, days + 1);
        }
    } else {
        std::ostringstream oss;
        oss << ymd;
        THROW_CODE(InputError, "{}", oss.str());
    }
}

void Date::fromYearQuarterDoq(int year, int quarter, int doq) {
    date::year_month_day ymd;
    switch (quarter) {
        case 1: {
            ymd = date::year{year} / 1 / 1;
            break;
        }
        case 2: {
            ymd = date::year{year} / 4 / 1;
            break;
        }
        case 3: {
            ymd = date::year{year} / 7 / 1;
            break;
        }
        case 4: {
            ymd = date::year{year} / 10 / 1;
            break;
        }

        default:
            THROW_CODE(
                InputError,
                "Invalid value for QuarterOfYear (valid values 1 - 4): {}",
                quarter);
    }
    auto days = (date::local_days)ymd;
    days += date::days(doq - 1);
    ymd = date::year_month_day(days);

    if (ymd.ok()) {
        date::local_days ld = (date::local_days)ymd;
        days_since_epoch_ = ld.time_since_epoch().count();
    } else {
        std::ostringstream oss;
        oss << ymd;
        THROW_CODE(InputError, "{}", oss.str());
    }
}

void Date::fromYearDoy(int year, int doy) {
    date::year_month_day ymd =
        date::year_month_day{date::year{year}, date::month{1}, date::day{1}};
    auto days = (date::local_days)ymd;
    days += date::days(doy - 1);
    ymd = date::year_month_day(days);

    if (ymd.ok()) {
        date::local_days ld = (date::local_days)ymd;
        days_since_epoch_ = ld.time_since_epoch().count();
    } else {
        std::ostringstream oss;
        oss << ymd;
        THROW_CODE(InputError, "{}", oss.str());
    }
}

Date::Date() {
    auto t = make_zoned(date::current_zone(), std::chrono::system_clock::now());
    days_since_epoch_ = std::chrono::duration_cast<date::days>(
                            t.get_local_time().time_since_epoch())
                            .count();
}

Date::Date(const std::string& str) {
    int year = 0, doy = 1;
    unsigned month = 1, day = 1;
    int week = 1, dow = 1;
    int quarter = 1, doq = 1;

    std::smatch match;
    if (!std::regex_match(str, match, DATE_REGEX)) {
        THROW_CODE(InputError, "Failed to parse {} into Date", str);
    }

    try {
        if (match[SHORT_YEAR].matched) {  // short format
            year = std::stoi(match[SHORT_YEAR].str());
            if (match[SHORT_MONTH].matched) {
                month = std::stoi(match[SHORT_MONTH].str());
                if (match[SHORT_DAY].matched) {
                    day = std::stoi(match[SHORT_DAY].str());
                }
                fromYearMonthDay(year, month, day);
            } else if (match[SHORT_WEEK].matched) {
                week = std::stoi(match[SHORT_WEEK].str());
                if (match[SHORT_DOW].matched) {
                    dow = std::stoi(match[SHORT_DOW].str());
                }
                fromYearWeekDow(year, week, dow);
            } else if (match[SHORT_QUARTER].matched) {
                quarter = std::stoi(match[SHORT_QUARTER].str());
                if (match[SHORT_DOQ].matched) {
                    doq = std::stoi(match[SHORT_DOQ].str());
                }
                fromYearQuarterDoq(year, quarter, doq);
            } else if (match[SHORT_DOY].matched) {
                int doy = std::stoi(match[SHORT_DOY].str());
                fromYearDoy(year, doy);
            } else {
                fromYearMonthDay(year, month, day);
            }
        } else if (match[LONG_YEAR].matched) {  // long format
            year = std::stoi(match[LONG_YEAR].str());
            if (match[LONG_MONTH].matched) {
                month = std::stoi(match[LONG_MONTH].str());
                if (match[LONG_DAY].matched) {
                    day = std::stoi(match[LONG_DAY].str());
                }
                fromYearMonthDay(year, month, day);
            } else if (match[LONG_WEEK].matched) {
                week = std::stoi(match[LONG_WEEK].str());
                if (match[LONG_DOW].matched) {
                    dow = std::stoi(match[LONG_DOW].str());
                }
                fromYearWeekDow(year, week, dow);
            } else if (match[LONG_QUARTER].matched) {
                quarter = std::stoi(match[LONG_QUARTER].str());
                if (match[LONG_DOQ].matched) {
                    doq = std::stoi(match[LONG_DOQ].str());
                }
                fromYearQuarterDoq(year, quarter, doq);
            } else if (match[LONG_DOY].matched) {
                day = std::stoi(match[LONG_DOY].str());
                fromYearDoy(year, day);
            } else {
                fromYearMonthDay(year, month, day);
            }
        }
    } catch (const std::exception& e) {
        THROW_CODE(InputError, "Failed to parse {} into Date, exception: {}",
                   str, e.what());
    }
}

Date::Date(const Value& params) { auto& params_map = params.AsMap(); }

std::string Date::ToString() const {
    date::local_days tp((date::days(days_since_epoch_)));
    return date::format("%Y-%m-%d", tp);
}

bool Date::operator<(const Date& rhs) const noexcept {
    return days_since_epoch_ < rhs.days_since_epoch_;
}

bool Date::operator<=(const Date& rhs) const noexcept {
    return days_since_epoch_ <= rhs.days_since_epoch_;
}

bool Date::operator>(const Date& rhs) const noexcept {
    return days_since_epoch_ > rhs.days_since_epoch_;
}

bool Date::operator>=(const Date& rhs) const noexcept {
    return days_since_epoch_ >= rhs.days_since_epoch_;
}

bool Date::operator==(const Date& rhs) const noexcept {
    return days_since_epoch_ == rhs.days_since_epoch_;
}

bool Date::operator!=(const Date& rhs) const noexcept {
    return days_since_epoch_ != rhs.days_since_epoch_;
}

}  // namespace common
