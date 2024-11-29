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
#include <optional>
#include <string>
#include <unordered_map>

// forword declaration to avoid include value.h here
class Value;

namespace common {

class Date {
   private:
    int64_t days_since_epoch_ = 0;

    void fromYearMonthDay(int year, unsigned month, unsigned day);
    void fromYearWeekDow(int year, unsigned week, unsigned dow);
    void fromYearWeekDow(const Date& base_date, std::optional<int> year,
                         std::optional<unsigned> week,
                         std::optional<unsigned> dow);
    void fromYearQuarterDoq(int year, int quarter, int doq);
    void fromYearDoy(int year, int doy);
    void fromTimeZone(std::string timezone);

   public:
    Date();

    /**
     * Construct a Date object from a temporal value
     *
     * \param   str: a string representation of a temporal value
     */
    explicit Date(const std::string& str);
    explicit Date(int64_t days) : days_since_epoch_(days){};

    /**
     * Construct a Date object from a user supplied map
     *
     * \param   params: a map containing the single key 'timezone', or a map
     *                  containing temporal values ('date', 'year', 'month',
     *                  'day', 'week', 'dayOfWeek', 'quarter',
     *                  'dayOfQuarter', 'ordinalDay') as components.
     */
    explicit Date(const Value& params);
    [[nodiscard]] int64_t GetStorage() const { return days_since_epoch_; }
    [[nodiscard]] std::string ToString() const;
    [[nodiscard]] Value GetUnit(std::string unit) const;
    bool operator<(const Date& rhs) const noexcept;
    bool operator<=(const Date& rhs) const noexcept;
    bool operator>(const Date& rhs) const noexcept;
    bool operator>=(const Date& rhs) const noexcept;
    bool operator==(const Date& rhs) const noexcept;
    bool operator!=(const Date& rhs) const noexcept;
};

}  // namespace common
