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

#include "./antlr4-runtime.h"
#include "cypher/execution_plan/plan_cache/plan_cache_param.h"
#include "cypher/execution_plan/plan_cache/plan_cache.h"

#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "core/data_type.h"
#include "./test_tools.h"

class TestPlanCache : public TuGraphTest {};

TEST_F(TestPlanCache, basicCaching) {
    cypher::LRUPlanCache<int> cache(512);

    cache.add_plan("1", 1);
    int value;
    cache.get_plan("1", value);
    ASSERT_EQ(value, 1);

    cache.add_plan("2", 2);
    cache.get_plan("2", value);
    ASSERT_EQ(value, 2);

    ASSERT_EQ(cache.current_size(), 2);
}

TEST_F(TestPlanCache, eviction) {
    cypher::LRUPlanCache<int> cache(512);

    for (int i = 0; i < 522; i++) {
        cache.add_plan(std::to_string(i), i);
    }

    for (int i = 0; i < 10; i++) {
        int val;
        bool res = cache.get_plan(std::to_string(i), val);
        ASSERT_EQ(res, false);
    }

    for (int i = 10; i < 522; i++) {
        int val;
        bool res = cache.get_plan(std::to_string(i), val);
        ASSERT_EQ(res, true);
    }
}
