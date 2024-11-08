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
#include "common/value.h"

namespace common {

LocalTime::LocalTime() {
    auto t = make_zoned(date::current_zone(), std::chrono::system_clock::now());
    nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
}

LocalTime::LocalTime(const Value& params) {
    if (params.IsLocalTime()) {
        date::local_time<std::chrono::nanoseconds> tp{std::chrono::nanoseconds(params.AsLocalTime().nanoseconds_since_epoch_)};
        auto t = make_zoned(date::current_zone(), tp);
        nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
        return;
    }
    std::unordered_map<std::string, Value> parse_params_map;
    for (const auto &kv : params.AsMap()) {
        auto s = kv.first;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        parse_params_map.emplace(s, kv.second);
    }
    if (parse_params_map.empty()) {
        THROW_CODE(InvalidParameter, "At least one temporal unit must be specified.");
    }
    if (parse_params_map.count("timezone")) {
        if (parse_params_map.size() > 1) {
            THROW_CODE(InvalidParameter, "Cannot assign time zone if also assigning other fields.");
        }
        auto t = date::make_zoned(parse_params_map["timezone"].AsString(), std::chrono::system_clock::now());
        nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
        return;
    }
    int64_t hour = 0, minute = 0, second = 0, millisecond = 0, microsecond = 0, nanosecond = 0;
    if (parse_params_map.count("time")) {
        auto v = parse_params_map["time"].AsLocalTime().nanoseconds_since_epoch_;
        nanosecond = v % 1000;
        microsecond = v / 1000 % 1000;
        millisecond = v / 1000000 % 1000;
        second = v / 1000000000 % 60;
        minute = v / 60000000000 % 60;
        hour = v / 3600000000000;
    } else {
        std::vector<std::pair<std::string, bool>> has_value = {
            {"hour", parse_params_map.count("hour")},
            {"minute", parse_params_map.count("minute")},
            {"second", parse_params_map.count("second")},
            {"subsecond", parse_params_map.count("millisecond") || parse_params_map.count("microsecond") || parse_params_map.count("nanosecond")}
        };
        if (!has_value[0].second) {
            THROW_CODE(InvalidParameter, "hour must be specified");
        }
        std::string firstNotAssigned;
        for (const auto &value : has_value) {
            if (!value.second) {
                if (firstNotAssigned.empty()) {
                    firstNotAssigned = value.first;
                }
            } else if (!firstNotAssigned.empty()) {
                THROW_CODE(InvalidParameter, "{} cannot be specified without {}", value.first, firstNotAssigned);
            }
        }
    }
    // hh-mm-ss exception
    if (!parse_params_map["hour"].IsNull()) {
        hour = parse_params_map["hour"].AsInteger();
    }
    if (!parse_params_map["minute"].IsNull()) {
        minute = parse_params_map["minute"].AsInteger();
    }
    if (!parse_params_map["second"].IsNull()) {
        second = parse_params_map["second"].AsInteger();
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

    // sub second exception
    auto millisecondValue = parse_params_map["millisecond"],
         microsecondValue = parse_params_map["microsecond"], nanosecondValue = parse_params_map["nanosecond"];
    if (!millisecondValue.IsNull() && (millisecondValue.AsInteger() < 0 || millisecondValue.AsInteger() >= 1000)) {
        THROW_CODE(InvalidParameter, "Invalid value for Millisecond: {}", millisecondValue.AsInteger());
    }
    if (!microsecondValue.IsNull() && (microsecondValue.AsInteger() < 0 || microsecondValue.AsInteger() >= (!millisecondValue.IsNull() ? 1000 : 1000000))) {
        THROW_CODE(InvalidParameter, "Invalid value for Microsecond: {}", microsecondValue.AsInteger());
    }
    if (!nanosecondValue.IsNull() && (nanosecondValue.AsInteger() < 0 || nanosecondValue.AsInteger() >= (!microsecondValue.IsNull() ? 1000 : !millisecondValue.IsNull() ? 1000000 : 1000000000))) {
        THROW_CODE(InvalidParameter, "Invalid value for Nanosecond: {}", nanosecondValue.AsInteger());
    }
    if (!millisecondValue.IsNull() || !microsecondValue.IsNull() || !nanosecondValue.IsNull()) {
        millisecond = microsecond = nanosecond = 0;
    }
    if (!millisecondValue.IsNull()) {
        millisecond = parse_params_map["millisecond"].AsInteger();
    }
    if (!microsecondValue.IsNull()) {
        microsecond = parse_params_map["microsecond"].AsInteger();
    }
    if (!nanosecondValue.IsNull()) {
        nanosecond = parse_params_map["nanosecond"].AsInteger();
    }

    // all time
    std::chrono::nanoseconds ns{hour * 60 * 60 * 1000000000 + minute * 60 * 1000000000 + second * 1000000000 +
                                millisecond * 1000000 + microsecond * 1000 + nanosecond};
    date::local_time<std::chrono::nanoseconds> tp{ns};
    auto t = make_zoned(date::current_zone(), tp);
    nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
}

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