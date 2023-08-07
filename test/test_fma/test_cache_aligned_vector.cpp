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

#include "fma-common/cache_aligned_vector.h"
#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "./unit_test_utils.h"

FMA_SET_TEST_PARAMS(CacheAlignedVector, "");

FMA_UNIT_TEST(CacheAlignedVector) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    fma_common::StaticCacheAlignedVector<int, 1> v1;
    int x = 10;
    fma_common::StaticCacheAlignedVector<std::string, 1> v2;
    int y = 10;
    FMA_UT_CHECK_EQ((uint64_t)&v1[0] % fma_common::FMA_CACHE_LINE_SIZE, 0);
    FMA_UT_CHECK_EQ((uint64_t)&v1[0] % fma_common::FMA_CACHE_LINE_SIZE, 0);
    FMA_UT_CHECK_EQ(x + y, 20);

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
