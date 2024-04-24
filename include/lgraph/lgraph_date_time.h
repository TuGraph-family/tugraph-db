//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/**
 * @file lgraph_date_time.h
 * @brief Implemnets the DateTime, Date and TimeZone classes.
 */

#pragma once

#include <chrono>
#include <cstdint>
#include <ctime>
#include <string>

namespace lgraph_api {

class DateTime;
class Date;

/** @brief   A class that represents a time zone. */
class TimeZone {
    int64_t time_diff_microseconds_;
    static TimeZone GetLocalTZ();
    static TimeZone& LocalTZ();

 public:
    /**
     * @brief   Create a timezone which has time difference with UTC in hours time_diff_hours.
     *
     * @exception   InvalidParameter   Thrown if time_diff_hours is invalid.
     *
     * @param   time_diff_hours (Optional) Difference between local timezone and UTC. Must be &gt;
     *                          =-10 &amp;&amp; &lt;=14. Otherwise the function will throw.
     */
    explicit TimeZone(int time_diff_hours = 0);

    /**
     * @brief   Convert a DateTime from UTC time to this time zone.
     *
     * @exception   OutOfRange Thrown if the resulting value is out of range.
     *
     * @param   dt  DateTime in UTC time.
     *
     * @returns DateTime in local timezone.
     */
    DateTime FromUTC(const DateTime& dt) const;

    /**
     * @brief   Convert a DateTime from this timezone to UTC time.
     *
     * @exception   OutOfRange Thrown if the resulting value is out of range.
     *
     * @param   dt  DateTime in local timezone.
     *
     * @returns DateTime in UTC.
     */
    DateTime ToUTC(const DateTime& dt) const;

    /**
     * @brief   Returns diff from UTC in seconds, this is used in Date and DateTime
     *
     * @returns Number of seconds local timezone is from UTC.
     */
    int64_t UTCDiffSeconds() const noexcept;

    /**
     * @brief   Returns diff from UTC in hours, this is used in Date and DateTime
     *
     * @returns Number of hours local timezone is from UTC.
     */
    int64_t UTCDiffHours() const noexcept;

    /**
     * @brief   Get local timezone.
     *
     * @returns A const reference to local timezone.
     */
    static const TimeZone& LocalTimeZone() noexcept;

    /**
     * @brief   Update local timezone, used only when daylight saving time changes. Daylight
     *          saving time may change after LocalTZ was initialized, in which case we need to
     *          update it. This function will update all references returned by LocalTimeZone().
     */
    static void UpdateLocalTimeZone() noexcept;
};

/** @brief   Implements the Date class. Range of dates is from 0/1/1 to 12/31/9999. */
class Date {
    /** @brief   The days since epoch */
    int32_t days_since_epoch_;

 public:
    /** @brief   Structure representing a date in the format of year, month and day. */
    struct YearMonthDay {
        /** @brief   The year, 0-9999 */
        int year;
        /** @brief   Month, 1-12 */
        unsigned month;
        /** @brief   Day, 1-31 */
        unsigned day;
    };

    /** @brief   Construct a new Date object with the date set to 1970/1/1. */
    Date();

    /**
     * @brief   Construct a new Date object with date set to the specified time. The time point
     *          must be in the range of 0/1/1 to 12/31/9999.
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     *
     * @param   tp  Time point to set the date to.
     */
    explicit Date(const std::chrono::system_clock::time_point& tp);

    /**
     * @brief   Construct a new Date object with date set to the specified date. The date must be
     *          in the range of 0/1/1 to 12/31/9999.
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     *
     * @param   ymd Date in the form of year, month and day.
     */
    explicit Date(const YearMonthDay& ymd);

    /**
     * @brief   Construct a new Date object with date set to an offset from epoch,
     *          i.e. the date is set to the specified number of days from epoch. The result date
     *          must be in the range of 0/1/1 to 12/31/9999.
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     *
     * @param   days_since_epoch    Number of days since epoch.
     */
    explicit Date(int32_t days_since_epoch);

    /**
     * @brief   Parse date from a YYYY-MM-DD string.
     *
     * @exception   InputError  if the string is not in the correct format.
     *
     * @param   str The string.
     */
    explicit Date(const std::string& str);

    /**
     * @brief   Parse date from YYYY-MM-DD, save value in d.
     *
     * @param       str The string.
     * @param [out] d   The resulting Date.
     *
     * @returns True if success, otherwise false.
     */
    static bool Parse(const std::string& str, Date& d) noexcept;

    /**
     * @brief   Parse date from YYYY-MM-DD, save value in d.
     *
     * @param       beg The beg.
     * @param       end The end.
     * @param [out] d   The result.
     *
     * @returns Number of bytes parsed (must be 10), 0 if failed.
     */
    static size_t Parse(const char* beg, const char* end, Date& d) noexcept;

    /**
     * @brief   Returns the current Date.
     *
     * @returns Current Date in UTC.
     */
    static Date Now() noexcept;

    /**
     * @brief   Returns the current Date in local timezone.
     *
     * @returns Current Date in local timezone.
     */
    static Date LocalNow() noexcept;

    /**
     * @brief   Returns the current Date in the form of year, month and day.
     *
     * @returns The year month day.
     */
    YearMonthDay GetYearMonthDay() const noexcept;

    /**
     * @brief   Add a number of days to the Date object.
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     *
     * @param   days    Number of days to add.
     *
     * @returns The resulting Date object.
     */
    Date operator+(int days) const;

    /**
     * @brief   Add a number of days to the current Date object. In case of overflow, current
     *          object is not modified.
     *
     * @exception   OutOfRange Thrown if the resulting date is out of range.
     *
     * @param   days    Number of days to add.
     *
     * @returns Reference to the current date.
     */
    Date& operator+=(int days);

    /**
     * @brief   Subtract a number of days from the Date object.
     *
     * @exception   OutOfRange Thrown if the resulting date is out of range.
     *
     * @param   days    Number of days to subtract.
     *
     * @returns The resulting Date object.
     */
    Date operator-(int days) const;

    /**
     * @brief   Subtract a number of days from the current Date object. In case of overflow,
     *          current object is not modified.
     *
     * @exception   OutOfRange Thrown if the resulting date is out of range.
     *
     * @param   days    Number of days to subtract.
     *
     * @returns Reference to the current Date object.
     */
    Date& operator-=(int days);

    bool operator<(const Date& rhs) const noexcept;
    bool operator<=(const Date& rhs) const noexcept;
    bool operator>(const Date& rhs) const noexcept;
    bool operator>=(const Date& rhs) const noexcept;
    bool operator==(const Date& rhs) const noexcept;
    bool operator!=(const Date& rhs) const noexcept;

    /** @brief   Returns the number of days this date is since epoch. */
    int32_t DaysSinceEpoch() const noexcept;

    /** @brief   Returns the number of days this date is since epoch. */
    int32_t GetStorage() const noexcept;

    /** @brief   Returns the number of days this date is since epoch. */
    explicit operator int32_t() const noexcept;

    /** @brief   Returns the timepoint corresponding to this date at 00:00 am. */
    std::chrono::system_clock::time_point TimePoint() const noexcept;

    /** @brief   Get the string representation of the date in the format of YYYY-MM-DD. */
    std::string ToString() const noexcept;

    /** Get the DateTime object corresponding to 00:00 am on this date. */
    explicit operator DateTime() const noexcept;
};

/** @brief   min and max values that Date can hold */
static inline constexpr int32_t MinDaysSinceEpochForDate() { return -719528; }

/** @brief   Maximum days since epoch for date */
static inline constexpr int32_t MaxDaysSinceEpochForDate() { return 2932896; }

/**
 * @brief   Implements a DateTime class that holds DateTime in the range of
 *          0000-01-01 00:00:00.000000 to 9999-12-31 23:59:59.999999.
 */
class DateTime {
    int64_t microseconds_since_epoch_;

 public:
    /** @brief   Representation of a DateTime in year, month, day, hour, minute, second,
     *  fraction. */
    struct YMDHMSF {
        /** @brief   The year, 0-9999 */
        int year;
        /** @brief   The month, 1-12 */
        unsigned month;
        /** @brief   The day, 1-31 */
        unsigned day;
        /** @brief   The hour, 0-23 */
        unsigned hour;
        /** @brief   The minute, 0-59 */
        unsigned minute;
        /** @brief   The second, 0-59 */
        unsigned second;
        /** @brief   The fraction 000000-999999*/
        unsigned fraction;
    };

    /**
     * @brief   Construct a new DateTime object with date set to the epoch time,
     *          i.e., 1970-1-1 00:00:00.
     */
    DateTime();

    /**
     * @brief   Construct a new DateTime object with date set to the specified timepoint.
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     *
     * @param   tp  Timepoint to set the DateTime to.
     */
    explicit DateTime(const std::chrono::system_clock::time_point& tp);

    /**
     * @brief   Construct a new DateTime object with date set to the specified date and time
     *          given in YMDHMSF.
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     *
     * @param   ymdhmsf  Date and time to set the DateTime to.
     */
    explicit DateTime(const YMDHMSF& ymdhmsf);

    /**
     * @brief   Construct a new DateTime object with date set to specified number of microseconds
     *          since epoch.
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     *
     * @param   microseconds_since_epoch Number of microseconds since epoch.
     */
    explicit DateTime(int64_t microseconds_since_epoch);

    /**
     * @brief   Construct a new DateTime object with date set to the specified date and time
     *          given in the form of YYYY-MM-DD HH:MM:SS[.FFFFFF].
     *
     * @exception   OutOfRange Thrown if the time point is out of range.
     * @exception   InputError Thrown if str has invalid format.
     *
     * @param   str String representation of the date and time in the form of YYYY-MM-DD HH:MM:SS.
     */
    explicit DateTime(const std::string& str);

    /**
     * @brief   Parse date from YYYY-MM-DD HH:MM:SS(.FFFFFF), save value in d.
     *
     * @param       str The string.
     * @param [out] d   A DateTime to process.
     *
     * @returns True if success, otherwise false.
     */
    static bool Parse(const std::string& str, DateTime& d) noexcept;

    /**
     * @brief   Parse date from YYYY-MM-DD HH:MM:SS(.FFFFFF), save value in d.
     *
     * @param           beg The beg.
     * @param           end The end.
     * @param [in,out]  d   A DateTime to process.
     * 
     * @returns Number of bytes parsed (must be 19 or 26), 0 if failed.
     */
    static size_t Parse(const char* beg, const char* end, DateTime& d) noexcept;

    /**
     * @brief   Get current DateTime in UTC.
     *
     * @returns A DateTime.
     */
    static DateTime Now() noexcept;

    /**
     * @brief   Get current DateTime in local timezone.
     *
     * @returns A DateTime.
     */
    static DateTime LocalNow() noexcept;

    /**
     * @brief   Get current DateTime in the form of year, month, day, hour, minute, second, fraction.
     *
     * @returns The ymdhmsf.
     */
    YMDHMSF GetYMDHMSF() const noexcept;

    /**
     * @brief   Add a number of microseconds to the DateTime.
     *
     * @exception   OutOfRange Thrown if the resulting DateTime is out of range.
     *
     * @param   n_microseconds   Number of micorseconds to add.
     *
     * @returns The resulting DateTime.
     */
    DateTime operator+(int64_t n_microseconds) const;

    /**
     * @brief   Adds a number of microseconds to the current DateTime object. In case of overflow, the
     *          current DateTime object is not modified.
     *
     * @exception   OutOfRange Thrown if the resulting DateTime is out of range.
     *
     * @param   n_microseconds   Number of microseconds to add.
     *
     * @returns A reference to current object.
     */
    DateTime& operator+=(int64_t n_microseconds);

    /**
     * @brief   Subtract a number of microseconds from the DateTime.
     *
     * @exception   OutOfRange Thrown if the resulting DateTime is out of range.
     *
     * @param   n_microseconds   Number of microseconds to subtract.
     *
     * @returns The resulting DateTime.
     */
    DateTime operator-(int64_t n_microseconds) const;

    /**
     * @brief   Subtract a number of microseconds from the current DateTime object. In case of
     *          overflow, the current DateTime object is not modified.
     *
     * @exception   OutOfRange Thrown if the resulting DateTime is out of range.
     *
     * @param   n_microseconds   Number of microseconds to subtract.
     *
     * @returns A reference to current object.
     */
    DateTime& operator-=(int64_t n_microseconds);

    bool operator<(const DateTime& rhs) const noexcept;
    bool operator<=(const DateTime& rhs) const noexcept;
    bool operator>(const DateTime& rhs) const noexcept;
    bool operator>=(const DateTime& rhs) const noexcept;
    bool operator==(const DateTime& rhs) const noexcept;
    bool operator!=(const DateTime& rhs) const noexcept;

    /** Get the number of microseconds this DateTime is since epoch. */
    int64_t MicroSecondsSinceEpoch() const noexcept;

    /** @brief   Get the number of microseconds this DateTime is since epoch. */
    int64_t GetStorage() const noexcept;

    /** @brief   Get the number of microseconds this DateTime is since epoch. */
    explicit operator int64_t() const noexcept;

    /** @brief   Get the timepoint correponding to this DateTime. */
    std::chrono::system_clock::time_point TimePoint() const noexcept;

    /**
     * @brief   Get string representation of the date and time in the form of
     *          YYYY-MM-DD HH:MM:SS.[ffffff] */
    std::string ToString() const noexcept;

    /** @brief   Get the Date object corresponding to this DateTime. */
    explicit operator Date() const noexcept;

    /**
     * @brief   Get the UTC time corresponding to this DateTime, assuming current DateTime
     *           is a local time.
     *
     * @returns Object converted to an UTC.
     *
     * @exception   OutOfRange Thrown if the resulting DateTime is out of range.
     */
    DateTime ConvertToUTC();

    /**
     * @brief   Get the local time corresponding to this DateTime, assuming current DateTime
     *           is a UTC time.
     *
     * @returns Object converted to a local.
     *
     * @exception   OutOfRange Thrown if the resulting DateTime is out of range.
     */
    DateTime ConvertToLocal();
};

/** @brief   min and max values that Date can hold */
static inline constexpr int64_t MinMicroSecondsSinceEpochForDateTime() {
    return -62167219200000000LL;
}

/** @brief   Maximum microseconds since epoch for date time */
static inline constexpr int64_t MaxMicroSecondsSinceEpochForDateTime() {
    return 253402300799999999LL;
}

}  // namespace lgraph_api
