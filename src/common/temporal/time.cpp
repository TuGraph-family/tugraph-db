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

#include "common/temporal/time.h"

#include <date/tz.h>

#include <boost/algorithm/string.hpp>
#include <boost/endian/conversion.hpp>

#include "common/exceptions.h"
#include "common/temporal/temporal_pattern.h"
#include "common/value.h"

namespace common {

Time::Time() {
    auto t = date::make_zoned("UTC", std::chrono::system_clock::now());
    nanoseconds_since_today_with_of_ = t.get_local_time().time_since_epoch().count();
}

Time::Time(const std::string& str) {
    int64_t hour = 0, minute = 0, second = 0, nanoseconds = 0;
    int64_t zoneHour = 0, zoneMinute = 0, zoneSecond = 0;
    std::string timezoneName = "UTC";
    std::smatch match;
    if (!std::regex_match(str, match, TIME_REGEX)) {
        THROW_CODE(InputError, "Failed to parse {} into Time", str);
    }

    try {
        if (match[TIME_LONG_HOUR].matched) {  // long format
            hour = std::stoi(match[TIME_LONG_HOUR].str());
            if (match[TIME_LONG_MINUTE].matched) {
                minute = std::stoi(match[TIME_LONG_MINUTE].str());
            }
            if (match[TIME_LONG_SECOND].matched) {
                second = std::stoi(match[TIME_LONG_SECOND].str());
            }
            if (match[TIME_LONG_FRACTION].matched) {
                auto tmp = match[TIME_LONG_FRACTION].str();
                if (tmp.size() < 9) {
                    tmp.resize(9, '0');
                }
                nanoseconds = std::stoi(tmp);
            }
        } else if (match[TIME_SHORT_HOUR].matched) {  // short format
            hour = std::stoi(match[TIME_SHORT_HOUR].str());
            if (match[TIME_SHORT_MINUTE].matched) {
                minute = std::stoi(match[TIME_SHORT_MINUTE].str());
            }
            if (match[TIME_SHORT_SECOND].matched) {
                second = std::stoi(match[TIME_SHORT_SECOND].str());
            }
            if (match[TIME_SHORT_FRACTION].matched) {
                auto tmp = match[TIME_SHORT_FRACTION].str();
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
        if (match[TIME_ZONE].matched) {
            auto tmp = match[TIME_ZONE].str();
            if (tmp == "Z" || tmp == "z") {
                timezoneName = "UTC";
            } else {
                if (match[TIME_ZONE_NAME].matched) {
                    THROW_CODE(InvalidParameter, "Using a named time zone e.g. "
                               "[UTC] is not valid for a time without a date. Instead, "
                               "use a specific time zone string e.g. +00:00.");
                }
                if (match[TIME_ZONE_HOUR].matched) {
                    zoneHour = std::stoi(match[TIME_ZONE_HOUR].str());
                }
                if (match[TIME_ZONE_MINUTE].matched) {
                    zoneMinute = std::stoi(match[TIME_ZONE_MINUTE].str());
                }
                if (match[TIME_ZONE_SECOND].matched) {
                    zoneSecond = std::stoi(match[TIME_ZONE_SECOND].str());
                }
                tz_offset_seconds_ = zoneHour * 60 * 60 + zoneMinute * 60 + zoneSecond;
                if (tmp[0] == '-') {
                    tz_offset_seconds_ *= -1;
                }
            }
        }
        std::chrono::nanoseconds ns{hour * 60 * 60 * 1000000000 + minute * 60 * 1000000000 + second * 1000000000 + nanoseconds};
        date::local_time<std::chrono::nanoseconds> tp{ns};
        auto t = make_zoned(timezoneName, tp);
        nanoseconds_since_today_with_of_ = t.get_local_time().time_since_epoch().count();
    } catch (const std::exception& e) {
        THROW_CODE(InputError, "Failed to parse {} into Date, exception: {}",
                   str, e.what());
    }
}

Time::Time(const Value& params) {
    std::string timezoneName = "UTC";
    if (params.IsLocalTime()) {
        date::local_time<std::chrono::nanoseconds> tp{std::chrono::nanoseconds(params.AsLocalTime().GetStorage())};
        auto t = make_zoned(timezoneName, tp);
        nanoseconds_since_today_with_of_ = t.get_local_time().time_since_epoch().count();
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
        auto tmp = parse_params_map["timezone"].AsString();
        if (tmp == "z" || tmp == "Z") {
            timezoneName = "UTC";
        } else if (tmp[0] == '+' || tmp[0] == '-') {
            std::smatch match;
            int64_t zoneHour = 0, zoneMinute = 0, zoneSecond = 0;
            if (!std::regex_match(tmp, match, OFFSET_REGEX)) {
                THROW_CODE(InputError, "Failed to parse {} into Time", tmp);
            }
            if (match[2].matched) {
                zoneHour = std::stoi(match[2].str());
            }
            if (match[3].matched) {
                zoneMinute = std::stoi(match[3].str());
            }
            if (match[4].matched) {
                zoneSecond = std::stoi(match[3].str());
            }
            tz_offset_seconds_ = zoneHour * 60 * 60 + zoneMinute * 60 + zoneSecond;
            if (tmp[0] == '-') {
                tz_offset_seconds_ *= -1;
            }
        } else {
            std::smatch match;
            if (!std::regex_match(tmp, match, ZONENAME_REGEX)) {
                THROW_CODE(InputError, "Failed to parse {} into Time", tmp);
            }
            std::replace(tmp.begin(), tmp.end(), ' ', '_');
            tz_offset_seconds_ = date::zoned_time(tmp).get_info().offset.count();
        }
        if (parse_params_map.size() == 1) {
            nanoseconds_since_today_with_of_ = std::chrono::system_clock::now().time_since_epoch().count() 
                                                  + tz_offset_seconds_ * 1000000000;
            if (nanoseconds_since_today_with_of_ < 0) {
                nanoseconds_since_today_with_of_ += 86400L * 1000000000;
            }
            return;
        }
    }
    int64_t hour = 0, minute = 0, second = 0, millisecond = 0, microsecond = 0, nanosecond = 0;
    if (parse_params_map.count("time")) {
        auto v = parse_params_map["time"].AsLocalTime().GetStorage();
        nanosecond = v % 1000;
        microsecond = v / 1000 % 1000;
        millisecond = v / 1000000 % 1000;
        second = v / 1000000000 % 60;
        minute = v / 60000000000 % 60;
        hour = v / 3600000000000 % 24;
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
    auto t = make_zoned(timezoneName, tp);
    nanoseconds_since_today_with_of_ = t.get_local_time().time_since_epoch().count();
}

std::string Time::ToString() const {
    date::local_time<std::chrono::nanoseconds> tp(
        (std::chrono::nanoseconds(nanoseconds_since_today_with_of_)));
    if (tz_offset_seconds_ == 0) {
        return date::format("%H:%M:%S", tp) + "Z";
    } else {
        char time_offset[32];
        auto abs_second = std::abs(tz_offset_seconds_);
        sprintf(time_offset, "%c%02ld:%02ld:%02ld", tz_offset_seconds_ < 0 ? '-' : '+', abs_second / 60 / 60,
                abs_second / 60 % 60, abs_second % 60);
        return date::format("%H:%M:%S", tp) + std::string(time_offset);
    }
}

bool Time::operator<(const Time& rhs) const noexcept {
    return nanoseconds_since_today_with_of_ < rhs.nanoseconds_since_today_with_of_;
}

bool Time::operator<=(const Time& rhs) const noexcept {
    return nanoseconds_since_today_with_of_ <= rhs.nanoseconds_since_today_with_of_;
}

bool Time::operator>(const Time& rhs) const noexcept {
    return nanoseconds_since_today_with_of_ > rhs.nanoseconds_since_today_with_of_;
}

bool Time::operator>=(const Time& rhs) const noexcept {
    return nanoseconds_since_today_with_of_ >= rhs.nanoseconds_since_today_with_of_;
}

bool Time::operator==(const Time& rhs) const noexcept {
    return nanoseconds_since_today_with_of_ == rhs.nanoseconds_since_today_with_of_;
}

bool Time::operator!=(const Time& rhs) const noexcept {
    return nanoseconds_since_today_with_of_ != rhs.nanoseconds_since_today_with_of_;
}

}  // namespace common