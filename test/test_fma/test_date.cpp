/**
 * Copyright 2022 AntGroup CO., Ltd.
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

#include <ctime>
#include "tools/date.h"
#include "fma-common/logger.h"
#include "./unit_test_utils.h"

FMA_SET_TEST_PARAMS(Date, "");

FMA_UNIT_TEST(Date) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    // using namespace date;
    // year_month_day tp{year(2019), month(2), day(29)};
    // auto sd = sys_days(tp);
    // FMA_LOG() << std::chrono::duration_cast<std::chrono::seconds>(sd.time_since_epoch()).count();
    tm t;
    t.tm_year = 2019;
    t.tm_mon = 2;
    t.tm_mday = 29;
    FMA_LOG() << t.tm_year << t.tm_mon << t.tm_mday;

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
