//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include "tools/date.h"
#include "fma-common/logger.h"

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

    FMA_LOG() << "**********************************************************************";
    FMA_LOG() << "This is a trial version, valid until " << exp << ".";
    if (day_diff > 0 && day_diff <= warn_before) {
        FMA_WARN() << "";
        FMA_WARN() << "Program is about to expire in " << day_diff << " days.";
        FMA_WARN() << "Please contact your supplier to obtain a new copy before it expires.";
    }
    if (day_diff <= 0) {
        FMA_ERR() << "Your copy of the program has expired. Please contact your supplier to obtain "
                     "a new copy.";
    }
    FMA_LOG() << "**********************************************************************";
}
}  // namespace fma_common
