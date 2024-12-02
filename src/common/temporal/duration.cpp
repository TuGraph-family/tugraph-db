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

size_t Duration::fractionPoint(const std::string &field) {
    if (field.empty()) {
        return std::string::npos;
    }
    auto fractionPoint = field.find('.');
    if (fractionPoint == std::string::npos) {
        fractionPoint = field.find(',');
    }
    return fractionPoint;
}

double Duration::parseFractional(const std::string &input, size_t pos) {
    return std::stod(
        input[pos] == '.' ? input : (input.substr(0, pos) + "." + input.substr(pos + 1)));
}

int64_t Duration::optLong(const std::string &value) {
    return value.empty() ? 0 : std::stoi(value);
}

int64_t Duration::getTimeValue(const Value& d) {
    if (d.IsDate()) {
        return d.AsDate().GetStorage() * SECONDS_PER_DAY * NANOS_PER_SECOND;
    } else if (d.IsDateTime()) {
        return std::get<0>(d.AsDateTime().GetStorage()) - std::get<1>(d.AsDateTime().GetStorage()) * NANOS_PER_SECOND;
    } else if (d.IsLocalDateTime()) {
        return d.AsLocalDateTime().GetStorage();
    } else if (d.IsTime()) {
        return std::get<0>(d.AsTime().GetStorage()) - std::get<1>(d.AsTime().GetStorage()) * NANOS_PER_SECOND;
    } else if (d.IsLocalTime()) {
        return d.AsLocalTime().GetStorage();
    } else {
        THROW_CODE(InvalidParameter, "The type of {} is not time", d.ToString());
    }
}

int64_t Duration::getTimeValueWithOffset(const Value &d) {
    if (d.IsDate()) {
        return d.AsDate().GetStorage() * SECONDS_PER_DAY * NANOS_PER_SECOND;
    } else if (d.IsDateTime()) {
        return std::get<0>(d.AsDateTime().GetStorage());
    } else if (d.IsLocalDateTime()) {
        return d.AsLocalDateTime().GetStorage();
    } else if (d.IsTime()) {
        return std::get<0>(d.AsTime().GetStorage());
    } else if (d.IsLocalTime()) {
        return d.AsLocalTime().GetStorage();
    } else {
        THROW_CODE(InvalidParameter, "The type of {} is not time", d.ToString());
    }
}

bool Duration::hasDate(const Value &d) {
    return d.IsDate() || d.IsDateTime() || d.IsLocalDateTime();
}

bool Duration::hasZone(const Value &d) {
    return d.IsDateTime() || d.IsTime();
}

int64_t Duration::getZoneOffsetTime(const Value &d) {
    if (d.IsDateTime()) {
        return std::get<1>(d.AsDateTime().GetStorage()) * NANOS_PER_SECOND;
    } else if (d.IsTime()) {
        return std::get<1>(d.AsTime().GetStorage()) * NANOS_PER_SECOND;
    } else {
        THROW_CODE(InvalidParameter, "do not have zone");
    }
}

Duration Duration::between(const Value& from, const Value& to, const std::string& unit) {
    int64_t from_nanos = getTimeValue(from), to_nanos = getTimeValue(to);
    if (hasDate(from) && !hasDate(to)) {
        to_nanos = to_nanos % (SECONDS_PER_DAY * NANOS_PER_SECOND) + getTimeValueWithOffset(from) / (SECONDS_PER_DAY * NANOS_PER_SECOND) * (SECONDS_PER_DAY * NANOS_PER_SECOND);
    } else if (!hasDate(from) && hasDate(to)) {
        from_nanos = from_nanos % (SECONDS_PER_DAY * NANOS_PER_SECOND) + getTimeValueWithOffset(to) / (SECONDS_PER_DAY * NANOS_PER_SECOND) * (SECONDS_PER_DAY * NANOS_PER_SECOND);
    }
    if (hasZone(from) && !hasZone(to)) {
        to_nanos -= getZoneOffsetTime(from);
    } else if (!hasZone(from) && hasZone(to)) {
        from_nanos -= getZoneOffsetTime(to);
    }
    int64_t bet = to_nanos - from_nanos;
    int64_t month = 0, day = 0, all_day = 0;
    if (std::abs(bet) >= SECONDS_PER_DAY * NANOS_PER_SECOND) {
        bet %= (SECONDS_PER_DAY * NANOS_PER_SECOND);
        auto from_day_nanos = from_nanos % (SECONDS_PER_DAY * NANOS_PER_SECOND);
        auto to_day_nanos = to_nanos % (SECONDS_PER_DAY * NANOS_PER_SECOND);
        auto start_ymd = date::year_month_day{date::floor<date::days>(date::local_time<std::chrono::nanoseconds>(std::chrono::nanoseconds(from_nanos)))};
        auto end_ymd = date::year_month_day{date::floor<date::days>(date::local_time<std::chrono::nanoseconds>(std::chrono::nanoseconds(to_nanos)))};
        month = (date::year_month(end_ymd.year(), end_ymd.month()) - date::year_month(start_ymd.year(), start_ymd.month())).count();
        all_day = (date::sys_days(end_ymd) - date::sys_days(start_ymd)).count();
        if (month > 0 && (start_ymd + date::months(month) > end_ymd || (start_ymd + date::months(month) == end_ymd && from_day_nanos > to_day_nanos))) {
            month--;
        }
        if (month < 0 && (start_ymd + date::months(month) < end_ymd || (start_ymd + date::months(month) == end_ymd && from_day_nanos < to_day_nanos))) {
            month++;
        }
        start_ymd += date::months(month);
        day = (date::sys_days(end_ymd) - date::sys_days(start_ymd)).count();
        if (day > 0 && from_day_nanos > to_day_nanos) {
            day--;
            all_day--;
        }
        if (day < 0 && from_day_nanos < to_day_nanos) {
            day++;
            all_day++;
        }
    }
    if (unit.empty()) {
        return Duration(month, day, bet / NANOS_PER_SECOND, bet % NANOS_PER_SECOND);
    } else if (unit == "MONTH") {
        return Duration(month, 0, 0, 0);
    } else if (unit == "DAY") {
        return Duration(0, all_day, 0, 0);
    } else if (unit == "SECOND") {
        bet = to_nanos - from_nanos;
        return Duration(0, 0, bet / NANOS_PER_SECOND, bet % NANOS_PER_SECOND);
    } else {
        THROW_CODE(InvalidParameter, "Unsupported unit: {}", unit);
    }
}

Duration Duration::parseDuration(int sign, int64_t month, int64_t day, std::smatch &match,
                                 bool strict, const std::string &hour, const std::string &min,
                                 const std::string &sec, const std::string &sub) {
    if (!strict) {
        size_t pos;
        if ((pos = fractionPoint(hour)) != std::string::npos) {
            if (!min.empty() || !sec.empty()) {
                THROW_CODE(InvalidParameter, "Text cannot be parsed to a Duration {}", match[0].str());
            }
            return approximate(month, day, parseFractional(hour, pos) * 3600, 0, sign);
        }
        int64_t secondsFromHours = optLong(hour) * 3600;
        if ((pos = fractionPoint(min)) != std::string::npos) {
            if (!sec.empty()) {
                THROW_CODE(InvalidParameter, "Text cannot be parsed to a Duration {}", match[0].str());
            }
            return approximate(month, day, secondsFromHours + parseFractional(min, pos) * 60, 0, sign);
        }
    }
    int64_t hours = optLong(hour);
    int64_t minutes = optLong(min);
    int64_t seconds = optLong(sec);
    if (strict) {
        if (hours > 24) {
            THROW_CODE(InvalidParameter, "hours out of range: {}", hours);
        }
        if (minutes > 60) {
            THROW_CODE(InvalidParameter, "minutes out of range: {}", minutes);
        }
        if (seconds > 60) {
            THROW_CODE(InvalidParameter, "seconds out of range: {}", seconds);
        }
    }

    int64_t nanos = optLong(sub);
    if (nanos != 0) {
        for (int i = sub.length(); i < 9; i++) {
            nanos *= 10;
        }
        if (sec[0] == '-') {
            nanos = -nanos;
        }
    }

    int64_t secondsAcc = seconds + hours * 3600 + minutes * 60;
    return Duration(sign * month, sign * day, sign * secondsAcc, sign * nanos);
}

Duration Duration::parseDateDuration(std::smatch& match) {
    int64_t months = 0, days = 0;
    if (match[DURATION_YEAR].matched) {
        int64_t month, day;
        if (match[DURATION_LONG_MONTH].matched) {
            month = std::stoi(match[DURATION_LONG_MONTH].str());
            day = std::stoi(match[DURATION_LONG_DAY].str());
        } else {
            month = std::stoi(match[DURATION_SHORT_MONTH].str());
            day = std::stoi(match[DURATION_SHORT_DAY].str());
        }
        months = month;
        if (months > 12) {
            THROW_CODE(InvalidParameter, "months is out of range: ", month);
        }
        months += std::stoi(match[DURATION_YEAR].str()) * 12;
        days = day;
        if (days > 31) {
            THROW_CODE(InvalidParameter, "days is out of range: ", days);
        }
    }
    int sign = match[1].str() == "-" ? -1 : 1;
    if (match[DURATION_TIME].matched) {
        if (match[DURATION_LONG_HOUR].matched) {
            return parseDuration(
                sign, months, days, match, true,
                match[DURATION_LONG_HOUR], match[DURATION_LONG_MINUTE],
                match[DURATION_LONG_SECOND], match[DURATION_LONG_SUB]);
        } else {
            return parseDuration(
                sign, months, days, match, true,
                match[DURATION_SHORT_HOUR], match[DURATION_SHORT_MINUTE],
                match[DURATION_SHORT_SECOND], match[DURATION_SHORT_SUB]);
        }
    } else {
        return Duration(sign * months, sign * days, 0, 0);
    }
}

Duration::Duration(const std::string &str) {
    std::smatch match;
    if (!std::regex_match(str, match, DURATION_REGEX)) {
        THROW_CODE(InputError, "Failed to parse {} into Duration", str);
    }
    if (match[DURATION_YEAR].matched || match[DURATION_TIME].matched) {
        *this = parseDateDuration(match);
    } else {
        *this = parseDuration(match);
    }
}

Duration Duration::parseDuration(std::smatch &match) {
    int sign = "-" == match[DURATION_SIGN].str() ? -1 : 1;
    std::string y = match[DURATION_YEARS].str();
    std::string m = match[DURATION_MONTHS].str();
    std::string w = match[DURATION_WEEKS].str();
    std::string d = match[DURATION_DAYS].str();
    std::string t = match[DURATION_T].str();
    if ((y.empty() && m.empty() && w.empty() && d.empty() && t.empty()) || "T" == t || "t" == t) {
        THROW_CODE(InvalidParameter, "Text cannot be parsed to a Duration {}", match[0].str());
    }
    size_t pos;
    if ((pos = fractionPoint(y)) != std::string::npos) {
        if (!m.empty() || !w.empty() || !d.empty() || !t.empty()) {
            THROW_CODE(InvalidParameter, "Text cannot be parsed to a Duration {}", match[0].str());
        }
        return approximate(parseFractional(y, pos) * 12, 0, 0, 0, sign);
    }

    int64_t years = optLong(y);
    int64_t monthsAcc = years * 12;
    if ((pos = fractionPoint(m)) != std::string::npos) {
        if (!w.empty() || !d.empty() || !t.empty()) {
            THROW_CODE(InvalidParameter, "Text cannot be parsed to a Duration {}", match[0].str());
        }
        return approximate(monthsAcc + parseFractional(m, pos), 0, 0, 0, sign);
    }

    monthsAcc += optLong(m);
    if ((pos = fractionPoint(w)) != std::string::npos) {
        if (!d.empty() || !t.empty()) {
            THROW_CODE(InvalidParameter, "Text cannot be parsed to a Duration {}", match[0].str());
        }
        return approximate(monthsAcc, parseFractional(w, pos) * 7, 0, 0, sign);
    }

    int64_t weeks = optLong(w);
    int64_t daysAcc = weeks * 7;
    if ((pos = fractionPoint(d)) != std::string::npos) {
        if (!t.empty()) {
            THROW_CODE(InvalidParameter, "Text cannot be parsed to a Duration {}", match[0].str());
        }
        return approximate(monthsAcc, daysAcc + parseFractional(d, pos), 0, 0, sign);
    }

    int64_t days = optLong(d);
    daysAcc += days;
    return parseDuration(sign, monthsAcc, daysAcc, match, false,
                         match[DURATION_HOURS].str(), match[DURATION_MINUTES].str(),
                         match[DURATION_SECONDS].str(), match[DURATION_SUB_SECONDS].str());
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
    if (allIntegralValues) {
        *this = Duration(
            parse_params_map["years"].AsInteger() * 12 + parse_params_map["months"].AsInteger(),
            parse_params_map["weeks"].AsInteger() * 7 + parse_params_map["days"].AsInteger(),
            parse_params_map["hours"].AsInteger() * 3600 + parse_params_map["minutes"].AsInteger() * 60 + parse_params_map["seconds"].AsInteger(),
            parse_params_map["milliseconds"].AsInteger() * 1000000 + parse_params_map["microseconds"].AsInteger() * 1000 + parse_params_map["nanoseconds"].AsInteger());
    } else {
        *this = approximate(safeCastFloatingPoint(parse_params_map["years"]) * 12 + safeCastFloatingPoint(parse_params_map["months"]),
                    safeCastFloatingPoint(parse_params_map["weeks"]) * 7 + safeCastFloatingPoint(parse_params_map["days"]),
                    safeCastFloatingPoint(parse_params_map["hours"]) * 3600 + safeCastFloatingPoint(parse_params_map["minutes"]) * 60 + safeCastFloatingPoint(parse_params_map["seconds"]),
                    safeCastFloatingPoint(parse_params_map["milliseconds"]) * 1000000 + safeCastFloatingPoint(parse_params_map["microseconds"]) * 1000 + safeCastFloatingPoint(parse_params_map["nanoseconds"]));
    }
}

Value Duration::GetUnit(std::string unit) const {
    std::transform(unit.begin(), unit.end(), unit.begin(), ::tolower);
    if (unit == "years") {
        return Value::Integer(months / 12);
    } else if (unit == "quarters") {
        return Value::Integer(months / 3);
    } else if (unit == "months") {
        return Value::Integer(months);
    } else if (unit == "weeks") {
        return Value::Integer(days / 7);
    } else if (unit == "days") {
        return Value::Integer(days);
    } else if (unit == "hours") {
        return Value::Integer(seconds / 3600);
    } else if (unit == "minutes") {
        return Value::Integer(seconds / 60);
    } else if (unit == "seconds") {
        return Value::Integer(seconds);
    } else if (unit == "milliseconds") {
        return Value::Integer(seconds * 1000 + nanos / 1000000);
    } else if (unit == "microseconds") {
        return Value::Integer(seconds * 1000000 + nanos / 1000);
    } else if (unit == "nanoseconds") {
        return Value::Integer(seconds * 1000000000 + nanos);
    } else if (unit == "quartersofyear") {
        return Value::Integer(months % 12 / 3);
    } else if (unit == "monthsofquarter") {
        return Value::Integer(months % 3);
    } else if (unit == "monthsofyear") {
        return Value::Integer(months % 12);
    } else if (unit == "daysofweek") {
        return Value::Integer(days % 7);
    } else if (unit == "minutesofhour") {
        return Value::Integer(seconds / 60 % 60);
    } else if (unit == "secondsofminute") {
        return Value::Integer(seconds % 60);
    } else if (unit == "millisecondsofsecond") {
        return Value::Integer(nanos / 1000000);
    } else if (unit == "microsecondsofsecond") {
        return Value::Integer(nanos / 1000);
    } else if (unit == "nanosecondsofsecond") {
        return Value::Integer(nanos);
    } else {
        THROW_CODE(InvalidParameter, "No such field: {}", unit);
    }
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

Duration &Duration::operator=(const Duration &other) {
    if (this != &other) {
        this->months = other.months;
        this->days = other.days;
        this->seconds = other.seconds;
        this->nanos = other.nanos;
    }
    return *this;
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

Duration Duration::operator+(const Duration &duration) const {
    return Duration(months + duration.months, days + duration.days, seconds + duration.days, nanos + duration.nanos);
}

Duration Duration::operator-(const Duration &duration) const {
    return Duration(months - duration.months, days - duration.days, seconds - duration.days, nanos - duration.nanos);
}

Duration Duration::operator*(double num) const {
    return Duration::approximate(months * num, days * num, seconds * num, nanos * num);
}

Duration Duration::operator/(double num) const {
    return Duration::approximate(months / num, days / num, seconds / num, nanos / num);
}

}