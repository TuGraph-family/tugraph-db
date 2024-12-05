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
#include "common/temporal/localdatetime.h"
#include "common/temporal/temporal_pattern.h"
#include "common/temporal/localtime.h"
#include "common/value.h"

namespace common {

LocalDateTime::LocalDateTime() {
    auto t = make_zoned(date::current_zone(), std::chrono::system_clock::now());
    nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
}

LocalDateTime::LocalDateTime(const std::string& str) {
    std::smatch match;
    if (!std::regex_match(str, match, LOCALDATETIME_REGEX)) {
        THROW_CODE(InputError, "Failed to parse {} into LocalDateTime");
    }
    auto pos = str.find('T');
    int64_t days_since_epoch = 0;
    int64_t nanoseconds_since_begin_of_day = 0;
    if (pos == std::string::npos) {
        days_since_epoch = Date(str).GetStorage();
    } else {
        days_since_epoch = Date(str.substr(0, pos)).GetStorage();
        nanoseconds_since_begin_of_day =
            LocalTime(str.substr(pos + 1)).GetStorage();
    }

    nanoseconds_since_epoch_ =
        std::chrono::nanoseconds(date::days(days_since_epoch)).count() +
        nanoseconds_since_begin_of_day;
}

LocalDateTime::LocalDateTime(const Value& params) {
    if (params.IsDateTime()) {
        nanoseconds_since_epoch_ = std::get<0>(params.AsDateTime().GetStorage());
        return;
    }
    if (params.IsLocalDateTime()) {
        nanoseconds_since_epoch_ = params.AsLocalDateTime().GetStorage();
        return;
    }

    std::unordered_map<std::string, Value> dateParams;
    std::unordered_map<std::string, Value> timeParams;
    for (const auto& [key, v] : params.AsMap()) {
        auto s = key;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (validDateFields.count(s)) {
            if (s == DATE_DATETIME) {
                dateParams.emplace("date", v);
                timeParams.emplace("time", v);
            } else {
                dateParams.emplace(s, v);
            }
        } else {
            timeParams.emplace(s, v);
        }
    }

    int64_t days_since_epoch = 0;
    int64_t nanoseconds_since_begin_of_day = 0;
    days_since_epoch = Date(Value(std::move(dateParams))).GetStorage();
    if (!timeParams.empty()) {
        if (!timeParams.count("hour") && !timeParams.count("time")) {
            timeParams["hour"] = Value::Integer(0);
        }
        nanoseconds_since_begin_of_day =
            LocalTime(Value(std::move(timeParams))).GetStorage();
    }

    nanoseconds_since_epoch_ =
        std::chrono::nanoseconds(date::days(days_since_epoch)).count() +
        nanoseconds_since_begin_of_day;
}

std::string LocalDateTime::ToString() const {
    date::local_time<std::chrono::nanoseconds> tp(
        (std::chrono::nanoseconds(nanoseconds_since_epoch_)));
    return date::format("%Y-%m-%dT%H:%M:%S", tp);
}

Value LocalDateTime::GetUnit(std::string unit) const {
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
    if (unit == "epochseconds") {
        return Value::Integer(nanoseconds_since_epoch_ / NANOS_PER_SECOND);
    } else if (unit == "epochmillis") {
        return Value::Integer(nanoseconds_since_epoch_ / 1000000);
    } if (unit == "year" || unit == "month" || unit == "day" || unit == "weekyear" || unit == "week" ||
        unit == "weekday" || unit == "ordinalday" || unit == "quarter" || unit == "dayofquarter") {
        return Date(nanoseconds_since_epoch_ / NANOS_PER_SECOND / SECONDS_PER_DAY).GetUnit(unit);
    } else {
        return LocalTime(nanoseconds_since_epoch_ % (NANOS_PER_SECOND * SECONDS_PER_DAY)).GetUnit(unit);
    }
}

bool LocalDateTime::operator<(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ < rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator<=(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ <= rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator>(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ > rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator>=(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ >= rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator==(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ == rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator!=(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ != rhs.nanoseconds_since_epoch_;
}

LocalDateTime LocalDateTime::operator+(const Duration& duration) const {
    auto days = nanoseconds_since_epoch_ / (NANOS_PER_SECOND * SECONDS_PER_DAY);
    auto nanos = nanoseconds_since_epoch_ % (NANOS_PER_SECOND * SECONDS_PER_DAY) + duration.seconds * NANOS_PER_SECOND + duration.nanos;
    if (nanos < 0) {
        auto day = int64_t(-floor(nanos * 1.0 / (NANOS_PER_SECOND * SECONDS_PER_DAY)));
        days -= day;
        nanos += day * NANOS_PER_SECOND * SECONDS_PER_DAY;
    }
    if (nanos >= NANOS_PER_SECOND * SECONDS_PER_DAY) {
        auto day = int64_t(-floor(nanos * 1.0 / (NANOS_PER_SECOND * SECONDS_PER_DAY)));
        days += day;
        nanos -= day * NANOS_PER_SECOND * SECONDS_PER_DAY;
    }
    date::year_month_day ymd((date::local_days((date::days(days)))));
    ymd += date::months(duration.months);
    return LocalDateTime((date::local_days(ymd).time_since_epoch().count() + duration.days) * NANOS_PER_SECOND * SECONDS_PER_DAY + nanos);
}

LocalDateTime LocalDateTime::operator-(const Duration& duration) const {
    auto days = nanoseconds_since_epoch_ / (NANOS_PER_SECOND * SECONDS_PER_DAY);
    auto nanos = nanoseconds_since_epoch_ % (NANOS_PER_SECOND * SECONDS_PER_DAY) - duration.seconds * NANOS_PER_SECOND - duration.nanos;
    if (nanos < 0) {
        auto day = int64_t(-floor(nanos * 1.0 / (NANOS_PER_SECOND * SECONDS_PER_DAY)));
        days -= day;
        nanos += day * NANOS_PER_SECOND * SECONDS_PER_DAY;
    }
    if (nanos >= NANOS_PER_SECOND * SECONDS_PER_DAY) {
        auto day = int64_t(-floor(nanos * 1.0 / (NANOS_PER_SECOND * SECONDS_PER_DAY)));
        days += day;
        nanos -= day * NANOS_PER_SECOND * SECONDS_PER_DAY;
    }
    date::year_month_day ymd((date::local_days((date::days(days)))));
    ymd -= date::months(duration.months);
    return LocalDateTime((date::local_days(ymd).time_since_epoch().count() - duration.days) * NANOS_PER_SECOND * SECONDS_PER_DAY + nanos);
}

}  // namespace common
