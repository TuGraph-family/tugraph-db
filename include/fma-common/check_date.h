//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include "tools/date.h"
#include "tools/lgraph_log.h"

namespace fma_common {
inline date::year_month_day MakeDate(int y, int m, int d) {
    return date::year_month_day{date::year(y), date::month(m), date::day(d)};
}

inline int ExpiresIn(int y, int m, int d) {
    auto exp = date::year_month_day{date::year(y), date::month(m), date::day(d)};

    auto today = date::floor<date::days>(std::chrono::system_clock::now());
    auto diff = date::sys_days(exp) - date::sys_days(today);
    return diff.count();
}

void CheckExpire(int y, int m, int d, int warn_before = 30) {
    auto exp = date::year_month_day{date::year(y), date::month(m), date::day(d)};

    auto today = date::floor<date::days>(std::chrono::system_clock::now());
    auto diff = date::sys_days(exp) - date::sys_days(today);
    int day_diff = diff.count();

    LOG_INFO() << "**********************************************************************";
    LOG_INFO() << "This is a trial version, valid until " << exp << ".";
    if (day_diff > 0 && day_diff <= warn_before) {
        LOG_WARN() << "";
        LOG_WARN() << "Program is about to expire in " << day_diff << " days.";
        LOG_WARN() << "Please contact your supplier to obtain a new copy before it expires.";
    }
    if (day_diff <= 0) {
        LOG_ERROR() << "Your copy of the program has expired. Please contact your "
                       "supplier to obtain a new copy.";
    }
    LOG_INFO() << "**********************************************************************";
}
}  // namespace fma_common
