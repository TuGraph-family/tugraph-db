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
class Duration {
   private:
    static Duration approximate(double m, double d, double s, double n) {
        return approximate(m, d, s, n, 1);
    }
    static Duration approximate(double m, double d, double s, double n, int sign);
    static void append(std::string &str, int64_t quantity, char unit);
    static void nanosecond(std::string &str, int64_t inanos);
    static int64_t safeDoubleToLong(double d);
    static double safeCastFloatingPoint(const Value &v);
    static Duration parseDateDuration(std::smatch& match);
    static Duration parseDuration(int sign, int64_t month, int64_t day,
                                  std::smatch& match, bool strict, const std::string& hour,
                                  const std::string& min, const std::string& sec, const std::string& sub);
    static Duration parseDuration(std::smatch& matcher);
    static size_t fractionPoint(const std::string& field);
    static double parseFractional(const std::string& input, size_t pos);
    static int64_t optLong(const std::string& value);
   public:
    int64_t months{};
    int64_t days{};
    int64_t seconds{};
    int64_t nanos{};
    explicit Duration(const Value& params);
    explicit Duration(const std::string& str);
    explicit Duration(int64_t m = 0, int64_t d = 0, int64_t s = 0, int64_t n = 0);

    [[nodiscard]] std::string ToString() const;
    Duration& operator=(const Duration& other);
    bool operator<(const Duration& rhs) const noexcept;
    bool operator<=(const Duration& rhs) const noexcept;
    bool operator>(const Duration& rhs) const noexcept;
    bool operator>=(const Duration& rhs) const noexcept;
    bool operator==(const Duration& rhs) const noexcept;
    bool operator!=(const Duration& rhs) const noexcept;
};
}