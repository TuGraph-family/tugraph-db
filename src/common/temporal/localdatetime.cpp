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
    std::unordered_map<std::string, Value> dateParams;
    std::unordered_map<std::string, Value> timeParams;
    for (const auto& [key, v] : params.AsMap()) {
        auto s = key;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        if (validDateFields.count(s)) {
            dateParams.emplace(s, v);
        } else {
            timeParams.emplace(s, v);
        }
    }

    int64_t days_since_epoch = 0;
    int64_t nanoseconds_since_begin_of_day = 0;
    days_since_epoch = Date(Value(std::move(dateParams))).GetStorage();
    if (!timeParams.empty()) {
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

}  // namespace common
