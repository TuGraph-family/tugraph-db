/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Copyright (c) "Neo4j"
 * Neo4j Sweden AB [https://neo4j.com]
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

/*
 * written by botu.wzy, inspired by Neo4j Go Driver
 */
#pragma once
#include <string>

namespace bolt {

struct Date {
    int64_t days;
};

struct Time {
    int64_t nanoseconds;
    int64_t tz_offset_seconds;
};

struct LocalTime {
    int64_t nanoseconds;
};

struct DateTime {
    int64_t seconds;
    int64_t nanoseconds;
    int64_t tz_offset_seconds;
};

struct DateTimeZoneId {
    int64_t seconds;
    int64_t nanoseconds;
    std::string tz_id;
};

struct LocalDateTime {
    int64_t seconds;
    int64_t nanoseconds;
};

struct LegacyDateTime {
    int64_t seconds;
    int64_t nanoseconds;
    int64_t tz_offset_seconds;
};

struct LegacyDateTimeZoneId {
    int64_t seconds;
    int64_t nanoseconds;
    std::string tz_id;
};

// Duration represents temporal amount containing months, days, seconds and nanoseconds.
// Supports longer durations than time.Duration
struct Duration {
    int64_t months;
    int64_t days;
    int64_t seconds;
    int64_t nanos;
};

}  // namespace bolt
