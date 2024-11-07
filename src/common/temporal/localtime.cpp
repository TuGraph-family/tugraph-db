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
#include "common/temporal/localtime.h"
#include "common/exceptions.h"

namespace common {

LocalTime::LocalTime() {
    auto t = make_zoned(date::current_zone(), std::chrono::system_clock::now());
    nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
}

LocalTime::LocalTime(const Value& params) {}

LocalTime::LocalTime(const std::string& str) {
    int64_t hour = 0, minute = 0, second = 0, nanoseconds = 0;
    std::smatch match;
    if (!std::regex_match(str, match, LOCALTIME_REGEX)) {
        THROW_CODE(InputError, "Failed to parse {} into LocalTime", str);
    }

    try {
        if (match[LONG_HOUR].matched) {  // long format
            hour = std::stoi(match[LONG_HOUR].str());
            if (match[LONG_MINUTE].matched) {
                minute = std::stoi(match[LONG_MINUTE].str());
            }
            if (match[LONG_SECOND].matched) {
                second = std::stoi(match[LONG_SECOND].str());
            }
            if (match[LONG_FRACTION].matched) {
                auto tmp = match[LONG_FRACTION].str();
                if (tmp.size() < 9) {
                    tmp.resize(9, '0');
                }
                nanoseconds = std::stoi(tmp);
            }
        } else if (match[SHORT_HOUR].matched) {  // short format
            hour = std::stoi(match[SHORT_HOUR].str());
            if (match[SHORT_MINUTE].matched) {
                minute = std::stoi(match[SHORT_MINUTE].str());
            }
            if (match[SHORT_SECOND].matched) {
                second = std::stoi(match[SHORT_SECOND].str());
            }
            if (match[SHORT_FRACTION].matched) {
                auto tmp = match[SHORT_FRACTION].str();
                if (tmp.size() < 9) {
                    tmp.resize(9, '0');
                }
                nanoseconds = std::stoi(tmp);
            }
        }
        if (hour >= 24) {
            THROW_CODE(InputError, "Invalid value for HourOfDay (valid values 0 - 23): {}", hour);
        }
        if (minute >= 60) {
            THROW_CODE(InputError, "Invalid value for MinuteOfHour (valid values 0 - 59): {}", minute);
        }
        if (second >= 60) {
            THROW_CODE(InputError, "Invalid value for SecondOfMinute (valid values 0 - 59): {}", second);
        }
        std::chrono::nanoseconds ns{hour * 60 * 60 * 1000000000 + minute * 60 * 1000000000 + second * 1000000000 + nanoseconds};
        date::local_time<std::chrono::nanoseconds> tp{ns};
        auto t = make_zoned(date::current_zone(), tp);
        nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
    } catch (const std::exception& e) {
        THROW_CODE(InputError, "Failed to parse {} into Date, exception: {}",
                   str, e.what());
    }
}

std::string LocalTime::ToString() const {
    date::local_time<std::chrono::nanoseconds> tp(
        (std::chrono::nanoseconds(nanoseconds_since_epoch_)));
    return date::format("%H:%M:%S", tp);
}

bool LocalTime::operator<(const LocalTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ < rhs.nanoseconds_since_epoch_;
}

bool LocalTime::operator<=(const LocalTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ <= rhs.nanoseconds_since_epoch_;
}

bool LocalTime::operator>(const LocalTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ > rhs.nanoseconds_since_epoch_;
}

bool LocalTime::operator>=(const LocalTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ >= rhs.nanoseconds_since_epoch_;
}

bool LocalTime::operator==(const LocalTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ == rhs.nanoseconds_since_epoch_;
}

bool LocalTime::operator!=(const LocalTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ != rhs.nanoseconds_since_epoch_;
}

}  // namespace common