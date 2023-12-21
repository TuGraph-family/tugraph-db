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

#include "fma-common/fma_stream.h"
#include "../test/ut_utils.h"

void TuGraphTest::setup() {
    if (_ut_buffer_log) {
        lgraph_log::LoggerManager::GetInstance().EnableBufferMode();
    }
}

void TuGraphTest::teardown() {
    if (_ut_buffer_log) {
        const ::testing::TestInfo* const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        if (test_info->result()->Failed()) {
            lgraph_log::LoggerManager::GetInstance().FlushBufferLog();
        }
        lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    }
}

void TuGraphTest::SetUp() { setup(); }

void TuGraphTest::TearDown() { teardown(); }
