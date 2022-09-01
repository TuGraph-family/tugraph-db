/* Copyright (c) 2022 AntGroup. All Rights Reserved. */
#include <cstddef>
#include "fma-common/configuration.h"
#include "core/cache_aligned_vector.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

class TestCacheAllignedVector : public TuGraphTest {};

struct Foo {
    double x;
    double y;
    int z;
};
TEST_F(TestCacheAllignedVector, CacheAllignedVector) {
    lgraph::StaticCacheAlignedVector<Foo, 64> foos;
    for (size_t i = 0; i < foos.size(); i++) {
       EXPECT_EQ((uint64_t)&foos[i] % 64, 0);
    }
}
