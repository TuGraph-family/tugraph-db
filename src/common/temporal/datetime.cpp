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
#include "common/temporal/datetime.h"

namespace common {

LocalDateTime::LocalDateTime() {
    auto t = make_zoned(date::current_zone(), std::chrono::system_clock::now());
    nanoseconds_since_epoch_ = t.get_local_time().time_since_epoch().count();
}

LocalDateTime::LocalDateTime(const std::string& str) {
    std::istringstream in(str);
    date::local_time<std::chrono::nanoseconds> tp;
    in >> date::parse("%FT%T", tp);
    if (in.fail()) {
        THROW_CODE(InputError, "Failed to parse {} into LocalDateTime", str);
    }
    nanoseconds_since_epoch_ = tp.time_since_epoch().count();
}

std::string LocalDateTime::ToString() const {
    date::local_time<std::chrono::nanoseconds> tp(
        (std::chrono::nanoseconds(nanoseconds_since_epoch_)));
    return date::format("%Y-%m-%dT%H:%M:%S", tp);
}

bool LocalDateTime::operator<(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ < rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator<=(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ <= rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator>(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ > rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator>=(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ >= rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator==(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ == rhs.nanoseconds_since_epoch_;
}

bool LocalDateTime::operator!=(const LocalDateTime& rhs) const noexcept {
    return nanoseconds_since_epoch_ != rhs.nanoseconds_since_epoch_;
}

}  // namespace common
