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
#include "common/temporal/datetime.h"
#include "common/temporal/temporal_pattern.h"
#include "common/temporal/time.h"
#include "common/value.h"

namespace common {

DateTime::DateTime() {
    auto t = make_zoned(date::current_zone(), std::chrono::system_clock::now());
    nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
}

DateTime::DateTime(const std::string& str) {
    std::smatch match;
    if (!std::regex_match(str, match, DATETIME_REGEX)) {
        THROW_CODE(InputError, "Failed to parse {} into DateTime");
    }
    auto pos = str.find('T');
    int64_t days_since_epoch = 0;
    int64_t nanoseconds_since_begin_of_day = 0;
    if (pos == std::string::npos) {
        days_since_epoch = Date(str).GetStorage();
    } else {
        days_since_epoch = Date(str.substr(0, pos)).GetStorage();
        auto t = Time(str.substr(pos + 1));
        nanoseconds_since_begin_of_day = std::get<0>(t.GetStorage());
        tz_offset_seconds_ = std::get<1>(t.GetStorage());
    }

    nanoseconds_since_epoch_ =
        std::chrono::nanoseconds(date::days(days_since_epoch)).count() +
        nanoseconds_since_begin_of_day;
}

DateTime::DateTime(const Value& params) {
    std::unordered_map<std::string, Value> dateParams;
    std::unordered_map<std::string, Value> timeParams;
    for (const auto& [key, v] : params.AsMap()) {
        auto s = key;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        // handle "timezone" in Time
        if (s != DATE_TIMEZONE && validDateFields.count(s)) {
            dateParams.emplace(s, v);
        } else {
            timeParams.emplace(s, v);
        }
    }

    int64_t days_since_epoch = 0;
    int64_t nanoseconds_since_begin_of_day = 0;
    days_since_epoch = Date(Value(std::move(dateParams))).GetStorage();
    if (!timeParams.empty()) {
        auto t = Time(Value(std::move(timeParams)));
        nanoseconds_since_begin_of_day = std::get<0>(t.GetStorage());
        tz_offset_seconds_ = std::get<1>(t.GetStorage());
    }

    nanoseconds_since_epoch_ =
        std::chrono::nanoseconds(date::days(days_since_epoch)).count() +
        nanoseconds_since_begin_of_day;
}

std::string DateTime::ToString() const {
    date::local_time<std::chrono::nanoseconds> tp(
        (std::chrono::nanoseconds(nanoseconds_since_epoch_)));
    if (tz_offset_seconds_ == 0) {
        return date::format("%Y-%m-%dT%H:%M:%S", tp) + "Z";
    } else {
        char time_offset[32];
        auto abs_second = std::abs(tz_offset_seconds_);
        sprintf(time_offset, "%c%02ld:%02ld:%02ld",
                tz_offset_seconds_ < 0 ? '-' : '+', abs_second / 60 / 60,
                abs_second / 60 % 60, abs_second % 60);
        return date::format("%Y-%m-%dT%H:%M:%S", tp) + std::string(time_offset);
    }
}

Value DateTime::GetUnit(std::string unit) const {
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
    if (unit == "year" || unit == "month" || unit == "day" || unit == "weekyear" || unit == "week" ||
        unit == "weekday" || unit == "ordinalday" || unit == "quarter" || unit == "dayofquarter") {
        return Date(nanoseconds_since_epoch_ / NANOS_PER_SECOND / SECONDS_PER_DAY).GetUnit(unit);
    } else {
        return Time(nanoseconds_since_epoch_ % (NANOS_PER_SECOND * SECONDS_PER_DAY), tz_offset_seconds_).GetUnit(unit);
    }
}

bool DateTime::operator<(const DateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ < rhs.nanoseconds_since_epoch_;
}

bool DateTime::operator<=(const DateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ <= rhs.nanoseconds_since_epoch_;
}

bool DateTime::operator>(const DateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ > rhs.nanoseconds_since_epoch_;
}

bool DateTime::operator>=(const DateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ >= rhs.nanoseconds_since_epoch_;
}

bool DateTime::operator==(const DateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ == rhs.nanoseconds_since_epoch_;
}

bool DateTime::operator!=(const DateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ != rhs.nanoseconds_since_epoch_;
}

}  // namespace common
