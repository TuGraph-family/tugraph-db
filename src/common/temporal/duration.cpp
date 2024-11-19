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
#include "common/temporal/duration.h"
#include "common/temporal/temporal_pattern.h"
#include "common/value.h"

namespace common {
Duration::Duration(int64_t m, int64_t d, int64_t s, int64_t n) : months(m), days(d), seconds(s), nanos(n) {
    seconds += nanos / NANOS_PER_SECOND;
    nanos %= NANOS_PER_SECOND;
    // normalize nanos to be between 0 and NANOS_PER_SECOND-1
    if (nanos < 0) {
        seconds -= 1;
        nanos += NANOS_PER_SECOND;
    }
}

Duration Duration::approximate(double m, double d, double s, double n, int sign) {
    int64_t monthsAsLong = safeDoubleToLong(m);

    double monthDiffInNanos = AVG_NANOS_PER_MONTH * (m - monthsAsLong);
    d += monthDiffInNanos / (NANOS_PER_SECOND * SECONDS_PER_DAY);
    int64_t daysAsLong = safeDoubleToLong(d);

    double daysDiffInNanos = NANOS_PER_SECOND * SECONDS_PER_DAY * (d - daysAsLong);
    s += daysDiffInNanos / NANOS_PER_SECOND;
    int64_t secondsAsLong = safeDoubleToLong(s);

    double secondsDiffInNanos = NANOS_PER_SECOND * (s - secondsAsLong);
    n += secondsDiffInNanos;
    int64_t nanosAsLong = safeDoubleToLong(n);

    return Duration(sign * monthsAsLong, sign * daysAsLong, sign * secondsAsLong, sign * nanosAsLong);
}

int64_t Duration::safeDoubleToLong(double d) {
    if (d > std::numeric_limits<int64_t>::max() || d < std::numeric_limits<int64_t>::min()) {
        THROW_CODE(ParserException, "long overflow");
    }
    return (int64_t) d;
}

double Duration::safeCastFloatingPoint(const Value &v) {
    if (v.IsInteger()) return double(v.AsInteger());
    if (v.IsFloat()) return v.AsFloat();
    if (v.IsDouble()) return v.AsDouble();
    THROW_CODE(InvalidParameter, "{} must be a number value", v.ToString());
}

Duration::Duration(const Value &params) {
    std::unordered_map<std::string, Value> parse_params_map;
    bool allIntegralValues = true;
    for (const auto &kv : params.AsMap()) {
        auto s = kv.first;
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
        parse_params_map.emplace(s, kv.second);
        if (!kv.second.IsInteger()) {
            allIntegralValues = false;
        }
    }
    for (const auto &k : DURATION_KEYS) {
        if (!parse_params_map.count(k)) {
            parse_params_map.emplace(k, Value::Integer(0));
        }
    }
    Duration t;
    if (allIntegralValues) {
        t = Duration(
            parse_params_map["years"].AsInteger() * 12 + parse_params_map["months"].AsInteger(),
            parse_params_map["weeks"].AsInteger() * 7 + parse_params_map["days"].AsInteger(),
            parse_params_map["hours"].AsInteger() * 3600 + parse_params_map["minutes"].AsInteger() * 60 + parse_params_map["seconds"].AsInteger(),
            parse_params_map["milliseconds"].AsInteger() * 1000000 + parse_params_map["microseconds"].AsInteger() * 1000 + parse_params_map["nanoseconds"].AsInteger());
    } else {
        t = approximate(safeCastFloatingPoint(parse_params_map["years"]) * 12 + safeCastFloatingPoint(parse_params_map["months"]),
                    safeCastFloatingPoint(parse_params_map["weeks"]) * 7 + safeCastFloatingPoint(parse_params_map["days"]),
                    safeCastFloatingPoint(parse_params_map["hours"]) * 3600 + safeCastFloatingPoint(parse_params_map["minutes"]) * 60 + safeCastFloatingPoint(parse_params_map["seconds"]),
                    safeCastFloatingPoint(parse_params_map["milliseconds"]) * 1000000 + safeCastFloatingPoint(parse_params_map["microseconds"]) * 1000 + safeCastFloatingPoint(parse_params_map["nanoseconds"]));
    }
    months = t.months;
    days = t.days;
    seconds = t.seconds;
    nanos = t.nanos;
}

void Duration::append(std::string &str, int64_t quantity, char unit) {
    if (quantity != 0) {
        str += std::to_string(quantity) + unit;
    }
}

std::string Duration::ToString() const {
    if (months == 0 && days == 0 && seconds == 0 && nanos == 0) {
        return "PT0S"; // no need to allocate a string builder if we know the result
    }

    std::string str = "P";
    append(str, months / 12, 'Y');
    append(str, months % 12, 'M');
    append(str, days, 'D');
    if (seconds != 0 || nanos != 0) {
        bool negative = seconds < 0;
        int64_t s = seconds;
        int64_t n = nanos;
        if (negative && nanos != 0) {
            s++;
            n -= NANOS_PER_SECOND;
        }
        str += "T";
        append(str, s / 3600, 'H');
        s %= 3600;
        append(str, s / 60, 'M');
        s %= 60;
        if (s != 0) {
            if (negative && s >= 0 && n != 0) {
                str += "-";
            }
            str += std::to_string(s);
            if (n != 0) {
                nanosecond(str, n);
            }
            str += "S";
        } else if (n != 0) {
            if (negative) {
                str += "-";
            }
            str += "0";
            nanosecond(str, n);
            str += "S";
        }
    }
    if (str.length() == 1) { // this was all zeros (but not ZERO for some reason), ensure well formed output:
        str.append("T0S");
    }
    return str;
}
void Duration::nanosecond(std::string &str, int64_t inanos) {
    str += ".";
    int64_t n = inanos < 0 ? -inanos : inanos;
    for (int64_t mod = NANOS_PER_SECOND; mod > 1 && n > 0; n %= mod) {
        mod /= 10;
        str += std::to_string(n / mod);
    }
}

bool Duration::operator<(const Duration& rhs) const noexcept {
    if (months == rhs.months) {
        if (days == rhs.days) {
            if (seconds == rhs.seconds) {
                return nanos < rhs.nanos;
            }
            return seconds < rhs.seconds;
        }
        return days < rhs.days;
    }
    return months < rhs.months;
}

bool Duration::operator<=(const Duration& rhs) const noexcept {
    if (months == rhs.months) {
        if (days == rhs.days) {
            if (seconds == rhs.seconds) {
                return nanos <= rhs.nanos;
            }
            return seconds <= rhs.seconds;
        }
        return days <= rhs.days;
    }
    return months <= rhs.months;
}

bool Duration::operator>(const Duration& rhs) const noexcept {
    if (months == rhs.months) {
        if (days == rhs.days) {
            if (seconds == rhs.seconds) {
                return nanos > rhs.nanos;
            }
            return seconds > rhs.seconds;
        }
        return days > rhs.days;
    }
    return months > rhs.months;
}

bool Duration::operator>=(const Duration& rhs) const noexcept {
    if (months == rhs.months) {
        if (days == rhs.days) {
            if (seconds == rhs.seconds) {
                return nanos >= rhs.nanos;
            }
            return seconds >= rhs.seconds;
        }
        return days >= rhs.days;
    }
    return months >= rhs.months;
}

bool Duration::operator==(const Duration& rhs) const noexcept {
    return months == rhs.months && days == rhs.days && seconds == rhs.seconds && nanos == rhs.nanos;
}

bool Duration::operator!=(const Duration& rhs) const noexcept {
    return months != rhs.months || days != rhs.days || seconds != rhs.seconds || nanos != rhs.nanos;
}
}