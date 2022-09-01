/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
