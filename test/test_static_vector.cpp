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

#include "fma-common/logger.h"
#include "gtest/gtest.h"

#include "core/task_tracker.h"
#include "./ut_utils.h"

class TestStaticVector : public TuGraphTest {};

class Dummy {
    static int n_dummy;
    int my_id;

 public:
    Dummy() {
        my_id = n_dummy;
        UT_LOG() << "Construct Dummy number " << n_dummy++;
    }

    ~Dummy() { UT_LOG() << "Destruct Dummy number " << --n_dummy; }

    int CurrentN() { return n_dummy; }

    int MyId() const { return my_id; }
};

int Dummy::n_dummy = 0;

TEST_F(TestStaticVector, StaticVector) {
    using namespace lgraph;
    {
        StaticCacheAlignedVector<Dummy, 10> dummies;
        UT_EXPECT_EQ(dummies[0].CurrentN(), 10);
        for (size_t i = 0; i < dummies.size(); i++) {
            UT_EXPECT_EQ(dummies[i].MyId(), i);
        }
    }
}
