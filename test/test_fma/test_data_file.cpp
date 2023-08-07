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

#include "fma-common/data_file.h"
#include "fma-common/logging.h"
#include "./unit_test_utils.h"

using namespace fma_common;

FMA_UNIT_TEST(DataFile) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    DataFileWriter<int, void> writer("test.bin", 1024, 1);
    DataFileWriter<int, int> writer2("t2.bin", 1, 1);

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
