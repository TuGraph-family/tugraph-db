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
    int64_t nanoseconds_since_today_ = 0;
    std::string timezone;
    std::string time_offset;

   public:
    Time();
    explicit Time(const std::string& str);
    explicit Time(const Value& params);
    explicit Time(int64_t nanoseconds)
        : nanoseconds_since_today_(nanoseconds){};
    [[nodiscard]] int64_t GetStorage() const {
        return nanoseconds_since_today_;
    }
    [[nodiscard]] std::string ToString() const;
    bool operator<(const Time& rhs) const noexcept;
    bool operator<=(const Time& rhs) const noexcept;
    bool operator>(const Time& rhs) const noexcept;
    bool operator>=(const Time& rhs) const noexcept;
    bool operator==(const Time& rhs) const noexcept;
    bool operator!=(const Time& rhs) const noexcept;
};
}