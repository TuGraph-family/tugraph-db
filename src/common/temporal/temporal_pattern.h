
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
#include <unordered_set>

namespace {

// Regular expression pattern for parsing dates
const std::string DATE_PATTERN =
    "(?:([0-9]{4})"
    "(?:([0-9]{2})([0-9]{2})?|"
    "W([0-9]{2})([0-9])?|Q([0-9])([0-9]{2})?|"
    "([0-9]{3}))|((?:[0-9]{4}|[+-][0-9]{1,9}))"
    "(?:-([0-9]{1,2})(?:-([0-9]{1,2}))?|"
    "-?W([0-9]{1,2})(?:-([0-9]))?|"
    "-?Q([0-9])(?:-([0-9]{1,2}))?|-([0-9]{3}))?)";

enum DATE_PATTERN_GROUP {
    DATE_SHORT_YEAR = 1,
    DATE_SHORT_MONTH,
    DATE_SHORT_DAY,
    DATE_SHORT_WEEK,
    DATE_SHORT_DOW,
    DATE_SHORT_QUARTER,
    DATE_SHORT_DOQ,
    DATE_SHORT_DOY,
    DATE_LONG_YEAR,
    DATE_LONG_MONTH,
    DATE_LONG_DAY,
    DATE_LONG_WEEK,
    DATE_LONG_DOW,
    DATE_LONG_QUARTER,
    DATE_LONG_DOQ,
    DATE_LONG_DOY,
};

const std::string DATE_TIMEZONE = "timezone";
const std::string DATE_DATE = "date";
const std::string DATE_YEAR = "year";
const std::string DATE_MONTH = "month";
const std::string DATE_DAY = "day";
const std::string DATE_WEEK = "week";
const std::string DATE_DOW = "dayofweek";
const std::string DATE_QUARTER = "quarter";
const std::string DATE_DOQ = "dayofquarter";
const std::string DATE_ORDINAL = "ordinalday";

const std::unordered_set<std::string> validDateFields{
    DATE_TIMEZONE, DATE_DATE, DATE_YEAR,    DATE_MONTH, DATE_DAY,
    DATE_WEEK,     DATE_DOW,  DATE_QUARTER, DATE_DOQ,   DATE_ORDINAL};

const std::string OFFSET_PATTERN = "(Z|[+-]([0-9]{2})(?::?([0-9]{2}))?(?::?([0-9]{2}))?)";
const std::string ZONENAME_PATTERN = "([a-zA-Z0-9~._ /+-]+)";

// Regular expression pattern for parsing local times
const std::string LOCALTIME_PATTERN =
    "(?:(?:([0-9]{1,2})(?::([0-9]{1,2})"
    "(?::([0-9]{1,2})(?:[\\.,]([0-9]{1,9}))?)?)?)|"
    "(?:([0-9]{2})(?:([0-9]{2})"
    "(?:([0-9]{2})(?:[\\.,]([0-9]{1,9}))?)?)?))";

// Regular expression pattern for parsing times
const std::string TIME_PATTERN =
    "(?:(?:([0-9]{1,2})(?::([0-9]{1,2})"
    "(?::([0-9]{1,2})(?:[\\.,]([0-9]{1,9}))?)?)?)|"
    "(?:([0-9]{2})(?:([0-9]{2})"
    "(?:([0-9]{2})(?:[\\.,]([0-9]{1,9}))?)?)?))"
    "(?:(Z|[+-]([0-9]{2})(?::?([0-9]{2}))?(?::?([0-9]{2}))?))?"
    "(?:\\[([a-zA-Z0-9~._ /+-]+)])?";

enum TIME_PATTERN_GROUP {
    TIME_LONG_HOUR = 1,
    TIME_LONG_MINUTE,
    TIME_LONG_SECOND,
    TIME_LONG_FRACTION,
    TIME_SHORT_HOUR,
    TIME_SHORT_MINUTE,
    TIME_SHORT_SECOND,
    TIME_SHORT_FRACTION,
    TIME_ZONE,
    TIME_ZONE_HOUR,
    TIME_ZONE_MINUTE,
    TIME_ZONE_SECOND,
    TIME_ZONE_NAME
};

// Regular expression pattern for parsing local datetimes & datetimes
const std::string DATETIME_PATTERN = DATE_PATTERN + "(T" + TIME_PATTERN + ")?";
const std::string LOCALDATETIME_PATTERN =
    DATE_PATTERN + "(T" + LOCALTIME_PATTERN + ")?";

enum DATETIME_PATTERN_GROUP {
    DATETIME_SHORT_YEAR = 1,
    DATETIME_SHORT_MONTH,
    DATETIME_SHORT_DAY,
    DATETIME_SHORT_WEEK,
    DATETIME_SHORT_DOW,
    DATETIME_SHORT_QUARTER,
    DATETIME_SHORT_DOQ,
    DATETIME_SHORT_DOY,
    DATETIME_LONG_YEAR,
    DATETIME_LONG_MONTH,
    DATETIME_LONG_DAY,
    DATETIME_LONG_WEEK,
    DATETIME_LONG_DOW,
    DATETIME_LONG_QUARTER,
    DATETIME_LONG_DOQ,
    DATETIME_LONG_DOY,
    DATETIME_LONG_HOUR,
    DATETIME_LONG_MINUTE,
    DATETIME_LONG_SECOND,
    DATETIME_LONG_FRACTION,
    DATETIME_SHORT_HOUR,
    DATETIME_SHORT_MINUTE,
    DATETIME_SHORT_SECOND,
    DATETIME_SHORT_FRACTION,
    DATETIME_ZONE,
    DATETIME_ZONE_HOUR,
    DATETIME_ZONE_MINUTE,
    DATETIME_ZONE_NAME
};

// Regular expression pattern for parsing datetimes

const std::regex DATE_REGEX(DATE_PATTERN);
const std::regex LOCALTIME_REGEX(LOCALTIME_PATTERN);
const std::regex TIME_REGEX(TIME_PATTERN);
const std::regex LOCALDATETIME_REGEX(LOCALDATETIME_PATTERN);
const std::regex DATETIME_REGEX(DATETIME_PATTERN);
const std::regex OFFSET_REGEX(OFFSET_PATTERN);
const std::regex ZONENAME_REGEX(ZONENAME_PATTERN);

// duration
const int64_t NANOS_PER_SECOND = 1000000000L;
const int64_t AVG_NANOS_PER_MONTH = 2629746000000000L;
const int64_t SECONDS_PER_DAY = 86400L;
const int64_t AVG_SECONDS_PER_MONTH = 2629746;
const std::vector<std::string> DURATION_KEYS = {"years", "months",
      "weeks", "days", "hours", "minutes", "seconds", "milliseconds",
      "microseconds", "nanoseconds"};
const std::string UNIT_BASED_PATTERN = "(?:([-+]?[0-9]+(?:[.,][0-9]+)?)Y)?"
    "(?:([-+]?[0-9]+(?:[.,][0-9]+)?)M)?"
    "(?:([-+]?[0-9]+(?:[.,][0-9]+)?)W)?"
    "(?:([-+]?[0-9]+(?:[.,][0-9]+)?)D)?"
    "(T"
    "(?:([-+]?[0-9]+(?:[.,][0-9]+)?)H)?"
    "(?:([-+]?[0-9]+(?:[.,][0-9]+)?)M)?"
    "(?:([-+]?[0-9]+)(?:[.,]([0-9]{1,9}))?S)?)?";
const std::string DATE_BASED_PATTERN = "(?:"
    "([0-9]{4})(?:"
    "-([0-9]{2})-([0-9]{2})|"
    "([0-9]{2})([0-9]{2}))"
    ")?(T"
    "(?:([0-9]{2})(?:([0-9]{2})"
    "(?:([0-9]{2})(?:[.,]([0-9]{1,9}))?)?)?|"
    "([0-9]{2}):([0-9]{2})"
    "(?::([0-9]{2})(?:[.,]([0-9]{1,9}))?)?))?";
const std::string DURATION_PATTERN = "([-+]?)P(?:" + UNIT_BASED_PATTERN + "|" + DATE_BASED_PATTERN + ")";
const std::regex DURATION_REGEX(DURATION_PATTERN);

enum DURATION_PATTERN_GROUP {
    DURATION_SIGN = 1,
    DURATION_YEARS,
    DURATION_MONTHS,
    DURATION_WEEKS,
    DURATION_DAYS,
    DURATION_T,
    DURATION_HOURS,
    DURATION_MINUTES,
    DURATION_SECONDS,
    DURATION_SUB_SECONDS,
    DURATION_YEAR,
    DURATION_LONG_MONTH,
    DURATION_LONG_DAY,
    DURATION_SHORT_MONTH,
    DURATION_SHORT_DAY,
    DURATION_TIME,
    DURATION_SHORT_HOUR,
    DURATION_SHORT_MINUTE,
    DURATION_SHORT_SECOND,
    DURATION_SHORT_SUB,
    DURATION_LONG_HOUR,
    DURATION_LONG_MINUTE,
    DURATION_LONG_SECOND,
    DURATION_LONG_SUB
};
}  // namespace