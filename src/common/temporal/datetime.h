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

#include <string>

// forword declaration to avoid include value.h here
class Value;

namespace common {
class DateTime {
   private:
    int64_t nanoseconds_since_epoch_ = 0;
    int64_t tz_offset_seconds_ = 0;

   public:
    DateTime();
    /**
     * Construct a DateTime object from a temporal value
     *
     * \param   str: a string representation of a temporal value
     */
    explicit DateTime(const std::string& str);
    explicit DateTime(int64_t nanoseconds, int64_t offset_seconds = 0)
        : nanoseconds_since_epoch_(nanoseconds),
          tz_offset_seconds_(offset_seconds){};

    /**
     * Construct a DateTime object from a user supplied map
     *
     * \param params: a map containing the single key 'timezone', or a map
     *                containing temporal values ('year', 'month', 'day',
     *                'hour', 'minute', 'second',
     *                'millisecond', 'microsecond', 'nanosecond') as components.
     */
    explicit DateTime(const Value& params);
    [[nodiscard]] std::tuple<int64_t, int64_t> GetStorage() const {
        return {nanoseconds_since_epoch_, tz_offset_seconds_};
    }
    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Value GetUnit(std::string unit) const;
    bool operator<(const DateTime& rhs) const noexcept;
    bool operator<=(const DateTime& rhs) const noexcept;
    bool operator>(const DateTime& rhs) const noexcept;
    bool operator>=(const DateTime& rhs) const noexcept;
    bool operator==(const DateTime& rhs) const noexcept;
    bool operator!=(const DateTime& rhs) const noexcept;
};

}  // namespace common
