/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <unistd.h>
#include "lgraph/lgraph_olap.h"
#include "./ut_utils.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

class TestParallelVector : public TuGraphTest {};

TEST_F(TestParallelVector, ParallelVector) {
    ParallelVector<int> v(100, 10);
    UT_EXPECT_EQ(v.Size(), 10);
    v.Fill(1);
    for (int i = 0; i < 10; i++) {
        UT_EXPECT_EQ(1, v[i]);
    }
    v.Resize(20, 2);
    UT_EXPECT_EQ(v.Size(), 20);
    for (int i = 10; i < 20; i++) {
        UT_EXPECT_EQ(2, v[i]);
    }
    v.Resize(30);
    UT_EXPECT_EQ(v.Size(), 30);
    for (int i = 20; i < 30; i++) {
        v[i] = 3;
        UT_EXPECT_EQ(v[i], 3);
    }
    UT_EXPECT_THROW_MSG(v.Resize(10), "The new size is smaller than the current one.");
    UT_EXPECT_THROW_MSG(v.Resize(200), "out of capacity.");
    v.Clear();
    UT_EXPECT_EQ(0, v.Size());

    v.ReAlloc(200);
    v.Resize(10, 1);
    UT_EXPECT_EQ(v.Size(), 10);

    for (int i = 0; i < 10; i++) {
        UT_EXPECT_EQ(1, v[i]);
    }
#pragma omp parallel for
    for (int i = 0; i < 50; i++) {
        v.Append(i);
    }
    UT_EXPECT_EQ(60, v.Size());
    bool flag[50];
    memset(flag, 0, sizeof(bool) * 50);
    for (int i = 0; i < 50; i++) {
        int t = v[i + 10];
        UT_EXPECT_TRUE(!flag[t] && t >= 0 && t < 50);
        flag[t] = true;
    }
#pragma omp parallel for
    for (int i = 0; i < 25; i++) {
        int buff[2] = {i << 1, (i << 1) + 1};
        v.Append(buff, 2);
    }
    memset(flag, 0, sizeof(bool) * 50);
    for (int i = 0; i < 50; i++) {
        int t = v[i + 60];
        UT_EXPECT_TRUE(!flag[t] && t >= 0 && t < 50);
        flag[t] = true;
    }
#pragma omp parallel for
    for (int i = 0; i < 25; i++) {
        ParallelVector<int> other(2, 2);
        other[0] = i << 1;
        other[1] = (i << 1) + 1;
        v.Append(other);
    }
    memset(flag, 0, sizeof(bool) * 50);
    for (int i = 0; i < 50; i++) {
        int t = v[i + 110];
        UT_EXPECT_TRUE(!flag[t] && t >= 0 && t < 50);
        flag[t] = true;
    }

    auto v_copy = v.Copy();
    UT_EXPECT_EQ(v_copy.Size(), v.Size());
    for (int i = 0; i < v.Size(); i++) {
        UT_EXPECT_EQ(v[i], v_copy[i]);
    }

    ParallelVector<int> v_swap(1);
    UT_EXPECT_EQ(v_swap.Size(), 0);
    v.Swap(v_swap);
    UT_EXPECT_EQ(v_swap.Size(), v_copy.Size());
    UT_EXPECT_EQ(v.Size(), 0);
    for (int i = 0; i < v_swap.Size(); i++) {
        UT_EXPECT_EQ(v_swap[i], v_copy[i]);
    }
}
