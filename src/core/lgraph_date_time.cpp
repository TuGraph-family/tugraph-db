/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/date.h"
#include "fma-common/string_formatter.h"
#include "fma-common/text_parser.h"

#include "lgraph/lgraph_date_time.h"
#include "lgraph/lgraph_exceptions.h"

#ifdef _MSC_VER
#include <timezoneapi.h>
#endif

namespace lgraph_api {
using namespace std::chrono;

TimeZone TimeZone::GetLocalTZ() {
#ifdef _MSC_VER
    TIME_ZONE_INFORMATION temp;
    auto rt = GetTimeZoneInformation(&temp);
    if (rt == TIME_ZONE_ID_INVALID) {
        throw std::runtime_error("Failed to get local timezone.");
    }
    return TimeZone(temp.Bias / 60);
#else
    DateTime utc(std::chrono::system_clock::now());
    time_t rawtime = (time_t)(utc.SecondsSinceEpoch());
    struct tm timeinfo;
    localtime_r(&rawtime, &timeinfo);
    char buf[128];
    size_t s = std::strftime(buf, 128, "%Y-%m-%d %H:%M:%S", &timeinfo);
    if (s != 19) throw InputError("failed to convert from DateTime to Local DateTime");
    DateTime local(buf);
    int time_diff_seconds = static_cast<int>(local.SecondsSinceEpoch() - utc.SecondsSinceEpoch());
    FMA_DBG_CHECK_EQ(time_diff_seconds % 3600, 0);
    return TimeZone(time_diff_seconds / 3600);
#endif
}

TimeZone& TimeZone::LocalTZ() {
    static TimeZone tz = GetLocalTZ();
    return tz;
}

TimeZone::TimeZone(int time_diff_hours) {
    if (time_diff_hours < -10 || time_diff_hours > 14)
        throw InvalidParameterError("time_diff_hours must be within [-24, 24]");
    time_diff_seconds_ = time_diff_hours * 3600;
}

DateTime TimeZone::FromUTC(const DateTime& dt) const {
    return DateTime(dt.SecondsSinceEpoch() + time_diff_seconds_);
}

DateTime TimeZone::ToUTC(const DateTime& dt) const {
    return DateTime(dt.SecondsSinceEpoch() - time_diff_seconds_);
}

int64_t TimeZone::UTCDiffSeconds() const noexcept { return time_diff_seconds_; }

int64_t TimeZone::UTCDiffHours() const noexcept { return time_diff_seconds_ / 3600; }

const TimeZone& TimeZone::LocalTimeZone() noexcept { return LocalTZ(); }

void TimeZone::UpdateLocalTimeZone() noexcept { LocalTZ() = GetLocalTZ(); }

static bool TryParseInt(const char** beg, const char* end, int& data, size_t expect_size, int min,
                        int max, char expect_after) {
    const char* b = *beg;
    size_t r = fma_common::TextParserUtils::ParseT(b, end, data);
    if (r != expect_size || data < min || data > max) return false;
    b += r;
    if (expect_after != '\0' && (b >= end || *b != expect_after)) return false;
    b++;
    *beg = b;
    return true;
}

template <size_t N>
static char* PrintNDigits(char* buf, unsigned d) {
    for (size_t i = 0; i < N; i++) {
        buf[i] = '0' + d % 10;
        d = d / 10;
    }
    std::reverse(buf, buf + N);
    return buf + N;
}

void CheckDaysOverflow(int32_t days) {
    if (days < MinDaysSinceEpochForDate() || days > MaxDaysSinceEpochForDate()) {
        throw OutOfRangeError(std::string("Failed to represent value with Date: out-of-bound.") +
                         "Value=" + std::to_string(days));
    }
}

void CheckDateTimeOverflow(int64_t seconds_since_epoch) {
    if (seconds_since_epoch < MinSecondsSinceEpochForDateTime() ||
        seconds_since_epoch > MaxSecondsSinceEpochForDateTime()) {
        throw OutOfRangeError(
            std::string("Failed to represent value with DateTime: out-of-bound.") +
                         "Value=" + std::to_string(seconds_since_epoch));
    }
}

Date::Date() : days_since_epoch_(0) {}

Date::Date(const std::chrono::system_clock::time_point& tp) {
    days_since_epoch_ = duration_cast<hours>(tp.time_since_epoch()).count() / 24;
    CheckDaysOverflow(days_since_epoch_);
}

Date::Date(const YearMonthDay& ymd) {
    date::year_month_day tp{date::year(ymd.year), date::month(ymd.month), date::day(ymd.day)};
    days_since_epoch_ = ((date::sys_days)tp).time_since_epoch().count();
    CheckDaysOverflow(days_since_epoch_);
}

Date::Date(int32_t days_since_epoch) {
    days_since_epoch_ = days_since_epoch;
    CheckDaysOverflow(days_since_epoch_);
}

Date::Date(const std::string& str) {
    bool r = Parse(str, *this);
    if (!r) throw InputError("failed to parse string " + str + " into Date");
    CheckDaysOverflow(days_since_epoch_);
}

bool Date::Parse(const std::string& str, Date& d) noexcept {
    size_t s = Parse(str.data(), str.data() + str.size(), d);
    return s != 0 && s == str.size();
}

size_t Date::Parse(const char* beg, const char* end, Date& d) noexcept {
    // YYYY-MM-DD
    int year, month, day;
    if (!TryParseInt(&beg, end, year, 4, 0, 9999, '-') ||
        !TryParseInt(&beg, end, month, 2, 1, 12, '-') ||
        !TryParseInt(&beg, end, day, 2, 1, 31, '\0')) {
        return 0;
    }
    date::year_month_day tp{date::year(year), date::month(month), date::day(day)};
    d.days_since_epoch_ = ((date::sys_days)tp).time_since_epoch().count();
    return 10;
}

Date Date::Now() noexcept { return Date(std::chrono::system_clock::now()); }

Date Date::LocalNow() noexcept {
    return Date(std::chrono::system_clock::now() +
                std::chrono::seconds(TimeZone::LocalTimeZone().UTCDiffSeconds()));
}

Date::YearMonthDay Date::GetYearMonthDay() const noexcept {
    date::year_month_day ymd(date::sys_days() + date::days(days_since_epoch_));
    return YearMonthDay({int(ymd.year()), unsigned(ymd.month()), unsigned(ymd.day())});
}

Date Date::operator+(int days) const { return Date(days_since_epoch_ + days); }

Date& Date::operator+=(int days) {
    auto new_days = days_since_epoch_ + days;
    CheckDaysOverflow(new_days);
    days_since_epoch_ = new_days;
    return *this;
}

Date Date::operator-(int days) const { return Date(days_since_epoch_ - days); }

Date& Date::operator-=(int days) {
    auto new_days = days_since_epoch_ - days;
    CheckDaysOverflow(new_days);
    days_since_epoch_ = new_days;
    return *this;
}

bool Date::operator<(const Date& rhs) const noexcept {
    return days_since_epoch_ < rhs.days_since_epoch_;
}

bool Date::operator<=(const Date& rhs) const noexcept {
    return days_since_epoch_ <= rhs.days_since_epoch_;
}

bool Date::operator>(const Date& rhs) const noexcept {
    return days_since_epoch_ > rhs.days_since_epoch_;
}

bool Date::operator>=(const Date& rhs) const noexcept {
    return days_since_epoch_ >= rhs.days_since_epoch_;
}

bool Date::operator==(const Date& rhs) const noexcept {
    return days_since_epoch_ == rhs.days_since_epoch_;
}

bool Date::operator!=(const Date& rhs) const noexcept {
    return days_since_epoch_ != rhs.days_since_epoch_;
}

int32_t Date::DaysSinceEpoch() const noexcept { return days_since_epoch_; }

int32_t Date::GetStorage() const noexcept { return days_since_epoch_; }

Date::operator int32_t() const noexcept { return days_since_epoch_; }

std::chrono::system_clock::time_point Date::TimePoint() const noexcept {
    return system_clock::time_point() + seconds(days_since_epoch_ * 24 * 3600);
}

std::string Date::ToString() const noexcept {
    YearMonthDay d = GetYearMonthDay();
    std::string ret = "YYYY-MM-HH";
    char* buf = &ret[0];
    buf = PrintNDigits<4>(buf, d.year);
    buf++;
    buf = PrintNDigits<2>(buf, d.month);
    buf++;
    buf = PrintNDigits<2>(buf, d.day);
    return ret;
}

Date::operator DateTime() const noexcept {
    return DateTime((int64_t)days_since_epoch_ * 24 * 3600);
}

DateTime::DateTime() : seconds_since_epoch_(0) {}

DateTime::DateTime(const std::chrono::system_clock::time_point& tp)
    : seconds_since_epoch_(duration_cast<seconds>(tp.time_since_epoch()).count()) {
    CheckDateTimeOverflow(seconds_since_epoch_);
}

DateTime::DateTime(const YMDHMS& ymdhms) {
    date::year_month_day tp{date::year(ymdhms.year), date::month(ymdhms.month),
                            date::day(ymdhms.day)};
    seconds_since_epoch_ = duration_cast<seconds>(((date::sys_days)tp).time_since_epoch()).count();
    seconds_since_epoch_ += ymdhms.hour * 3600 + ymdhms.minute * 60 + ymdhms.second;
    CheckDateTimeOverflow(seconds_since_epoch_);
}

DateTime::DateTime(int64_t seconds_since_epoch) : seconds_since_epoch_(seconds_since_epoch) {
    CheckDateTimeOverflow(seconds_since_epoch_);
}

DateTime::DateTime(const std::string& str) {
    if (!Parse(str, *this)) throw InputError("failed to parse string " + str + " into DateTime");
    CheckDateTimeOverflow(seconds_since_epoch_);
}

bool DateTime::Parse(const std::string& str, DateTime& d) noexcept {
    // YYYY-MM-DD HH:MM:SS
    size_t s = Parse(str.data(), str.data() + str.size(), d);
    return s != 0 && s == str.size();
}

size_t DateTime::Parse(const char* beg, const char* end, DateTime& d) noexcept {
    // YYYY-MM-DD HH:MM:SS
    int year, month, day, hour, minute, second;
    if (!TryParseInt(&beg, end, year, 4, 0, 9999, '-') ||
        !TryParseInt(&beg, end, month, 2, 1, 12, '-') ||
        !TryParseInt(&beg, end, day, 2, 1, 31, ' ') ||
        !TryParseInt(&beg, end, hour, 2, 0, 23, ':') ||
        !TryParseInt(&beg, end, minute, 2, 0, 59, ':') ||
        !TryParseInt(&beg, end, second, 2, 0, 59, '\0')) {
        return 0;
    }
    date::year_month_day tp{date::year(year), date::month(month), date::day(day)};
    d.seconds_since_epoch_ =
        duration_cast<seconds>(((date::sys_days)tp).time_since_epoch()).count();
    d.seconds_since_epoch_ += hour * 3600 + minute * 60 + second;
    return 19;
}

DateTime DateTime::Now() noexcept { return DateTime(std::chrono::system_clock::now()); }

DateTime DateTime::LocalNow() noexcept {
    return DateTime(std::chrono::system_clock::now() +
                    std::chrono::seconds(TimeZone::LocalTimeZone().UTCDiffSeconds()));
}

DateTime::YMDHMS DateTime::GetYMDHMS() const noexcept {
    int64_t day_count;
    int64_t seconds_per_day = 24 * 3600;
    if (seconds_since_epoch_ < 0) {
        day_count = (seconds_since_epoch_ - (seconds_per_day - 1)) / seconds_per_day;
    } else {
        // round down
        day_count = seconds_since_epoch_ / seconds_per_day;
    }
    date::year_month_day dat{date::sys_days() + date::days(day_count)};
    auto dur = seconds(seconds_since_epoch_ -
                       duration_cast<seconds>(((date::sys_days)dat).time_since_epoch()).count());
    unsigned hours = std::chrono::duration_cast<std::chrono::hours>(dur).count() % 24;
    unsigned minutes = std::chrono::duration_cast<std::chrono::minutes>(dur).count() % 60;
    unsigned seconds = std::chrono::duration_cast<std::chrono::seconds>(dur).count() % 60;
    return YMDHMS(
        {int(dat.year()), unsigned(dat.month()), unsigned(dat.day()), hours, minutes, seconds});
}

DateTime DateTime::operator+(int64_t n_seconds) const {
    return DateTime(seconds_since_epoch_ + n_seconds);
}

DateTime& DateTime::operator+=(int64_t n_seconds) {
    auto tmp = seconds_since_epoch_ + n_seconds;
    CheckDateTimeOverflow(tmp);
    seconds_since_epoch_ = tmp;
    return *this;
}

DateTime DateTime::operator-(int64_t n_seconds) const {
    return DateTime(seconds_since_epoch_ - n_seconds);
}

DateTime& DateTime::operator-=(int64_t n_seconds) {
    auto tmp = seconds_since_epoch_ - n_seconds;
    CheckDateTimeOverflow(tmp);
    seconds_since_epoch_ = tmp;
    return *this;
}

bool DateTime::operator<(const DateTime& rhs) const noexcept {
    return seconds_since_epoch_ < rhs.seconds_since_epoch_;
}

bool DateTime::operator<=(const DateTime& rhs) const noexcept {
    return seconds_since_epoch_ <= rhs.seconds_since_epoch_;
}

bool DateTime::operator>(const DateTime& rhs) const noexcept {
    return seconds_since_epoch_ > rhs.seconds_since_epoch_;
}

bool DateTime::operator>=(const DateTime& rhs) const noexcept {
    return seconds_since_epoch_ >= rhs.seconds_since_epoch_;
}

bool DateTime::operator==(const DateTime& rhs) const noexcept {
    return seconds_since_epoch_ == rhs.seconds_since_epoch_;
}

bool DateTime::operator!=(const DateTime& rhs) const noexcept {
    return seconds_since_epoch_ != rhs.seconds_since_epoch_;
}

int64_t DateTime::SecondsSinceEpoch() const noexcept { return seconds_since_epoch_; }

int64_t DateTime::GetStorage() const noexcept { return seconds_since_epoch_; }

DateTime::operator int64_t() const noexcept { return seconds_since_epoch_; }

std::chrono::system_clock::time_point DateTime::TimePoint() const noexcept {
    return system_clock::time_point() + seconds(seconds_since_epoch_);
}

std::string DateTime::ToString() const noexcept {
    YMDHMS d = GetYMDHMS();
    std::string ret = "YYYY-MM-DD hh:mm:ss";
    char* buf = &ret[0];
    buf = PrintNDigits<4>(buf, d.year);
    buf++;
    buf = PrintNDigits<2>(buf, d.month);
    buf++;
    buf = PrintNDigits<2>(buf, d.day);
    buf++;
    buf = PrintNDigits<2>(buf, d.hour);
    buf++;
    buf = PrintNDigits<2>(buf, d.minute);
    buf++;
    buf = PrintNDigits<2>(buf, d.second);
    return ret;
}

DateTime::operator Date() const noexcept {
    if (seconds_since_epoch_ >= 0) {
        return Date((int32_t)(seconds_since_epoch_ / (24 * 3600)));
    } else {
        return Date((int32_t)(-((-seconds_since_epoch_ + 24 * 3600 - 1) / (24 * 3600))));
    }
}

DateTime DateTime::ConvertToUTC() {
    DateTime ret = *this;
    ret.seconds_since_epoch_ -= TimeZone::LocalTimeZone().UTCDiffSeconds();
    CheckDateTimeOverflow(ret.seconds_since_epoch_);
    return ret;
}

DateTime DateTime::ConvertToLocal() {
    DateTime ret = *this;
    ret.seconds_since_epoch_ += TimeZone::LocalTimeZone().UTCDiffSeconds();
    CheckDateTimeOverflow(ret.seconds_since_epoch_);
    return ret;
}
}  // namespace lgraph_api
