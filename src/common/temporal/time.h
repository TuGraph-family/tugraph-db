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

#pragma once

#include <any>
#include <regex>
#include <string>
#include <unordered_map>

class Value;

namespace common {
class Time {
   private:
    int64_t nanoseconds_since_today_with_of_ = 0;
    int64_t tz_offset_seconds_ = 0;
    std::string timezone_name_ = "Z";
    [[nodiscard]] inline std::string timeOffsetToTimezone() const;

   public:
    Time();
    explicit Time(const std::string& str);
    explicit Time(const Value& params, int64_t truncate = 0);
    explicit Time(int64_t nanoseconds, int64_t tz_offset_seconds_ = 0)
        : nanoseconds_since_today_with_of_(nanoseconds), tz_offset_seconds_(tz_offset_seconds_){};
    [[nodiscard]] std::tuple<int64_t, int64_t> GetStorage() const {
        return {nanoseconds_since_today_with_of_, tz_offset_seconds_};
    }
    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] std::string GetTimezoneName() const;
    [[nodiscard]] Value GetUnit(std::string unit) const;
    bool operator<(const Time& rhs) const noexcept;
    bool operator<=(const Time& rhs) const noexcept;
    bool operator>(const Time& rhs) const noexcept;
    bool operator>=(const Time& rhs) const noexcept;
    bool operator==(const Time& rhs) const noexcept;
    bool operator!=(const Time& rhs) const noexcept;
};
}