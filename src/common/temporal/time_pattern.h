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

namespace {

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
    "(?:(Z|[+-]([0-9]{2})(?::?([0-9]{2}))?))?"
    "(?:\\[([a-zA-Z0-9~._ /+-]+)])?";

enum TIME_PATTERN_MATCH {
    LONG_HOUR = 1,
    LONG_MINUTE,
    LONG_SECOND,
    LONG_FRACTION,
    SHORT_HOUR,
    SHORT_MINUTE,
    SHORT_SECOND,
    SHORT_FRACTION,
    ZONE,
    ZONE_HOUR,
    ZONE_MINUTE,
    ZONE_NAME
};
const std::regex LOCALTIME_REGEX(LOCALTIME_PATTERN);
const std::regex TIME_REGEX(TIME_PATTERN);
}  // namespace