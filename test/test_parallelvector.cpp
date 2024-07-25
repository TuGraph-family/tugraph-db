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

#include <unistd.h>
#include "lgraph/olap_on_db.h"
#include "./ut_utils.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

class TestParallelVector : public TuGraphTest {};

TEST_F(TestParallelVector, ParallelVector) {
    ParallelVector<int> v(100, 10);
    UT_EXPECT_EQ(v.Capacity(), 100);
    UT_EXPECT_EQ(v.Size(), 10);
    v.Fill(1);
    for (auto iter = v.begin(); iter != v.end(); iter++) {
        UT_EXPECT_EQ(*iter, 1);
    }
    UT_EXPECT_EQ(v.Back(), 1);
    for (int i = 0; i < 10; i++) {
        UT_EXPECT_EQ(1, v[i]);
    }
    UT_EXPECT_EQ(*v.Data(), 1);
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
    UT_EXPECT_THROW_MSG(v.Resize(300, 2), "out of capacity.");
    UT_EXPECT_THROW_MSG(v.Resize(1, 2), "The new size is smaller than the current one.");
    UT_EXPECT_THROW_MSG(v.Resize(10), "The new size is smaller than the current one.");
    UT_EXPECT_THROW_MSG(v.Resize(200), "out of capacity.");
    v.ReAlloc(200);
    v.Clear();
    UT_EXPECT_EQ(0, v.Size());

    v.ReAlloc(200);
    UT_EXPECT_EQ(v.Capacity(), 200);
    UT_EXPECT_THROW_MSG(v.ReAlloc(0), "The new capacity is smaller than the current one.");
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

    ParallelVector<int> l;
    UT_EXPECT_THROW_MSG(l.ReAlloc(0), "Capacity cannot be 0");

    ParallelVector<int> s(11, 10);
    {
        int buff[2] = {26 << 1, (26 << 1) + 1};
        UT_EXPECT_THROW_MSG(s.Append(buff, 2, false), "out of capacity");
    }
    {
        int buff[1] = {2};
        s.Append(buff, 1, false);
        UT_EXPECT_EQ(s.Size(), 11);
    }
    auto v_copy = v.Copy();
    UT_EXPECT_EQ(v_copy.Size(), v.Size());
    for (int i = 0; i < v.Size(); i++) {
        UT_EXPECT_EQ(v[i], v_copy[i]);
    }

    ParallelVector<int> v_swap(1);
    UT_EXPECT_EQ(v_swap.Capacity(), 1);
    UT_EXPECT_EQ(v_swap.Size(), 0);
    v.Swap(v_swap);
    UT_EXPECT_EQ(v_swap.Size(), v_copy.Size());
    UT_EXPECT_EQ(v.Size(), 0);
    for (int i = 0; i < v_swap.Size(); i++) {
        UT_EXPECT_EQ(v_swap[i], v_copy[i]);
    }

    ParallelVector<int> v_move(std::move(v_swap));
    UT_EXPECT_EQ(v_move.Size(), v_copy.Size());
    UT_EXPECT_EQ(v_move.Capacity(), v_copy.Capacity());
    for (int i = 0; i < v_move.Size(); i++) {
        UT_EXPECT_EQ(v_move[i], v_copy[i]);
    }
    UT_EXPECT_EQ(v_swap.Size(), 0);
    UT_EXPECT_EQ(v_swap.Capacity(), 0);
    UT_EXPECT_EQ(v_swap.Data(), nullptr);
    UT_EXPECT_EQ(v_swap.Destroyed(), true);

    class Example {
     public:
        int x_;
        int y_;
        int num = 0;
        Example(int x, int y) : x_(x), y_(y) {num += 1;}

        ~Example(){
            num -= 1;
        }
    };
    ParallelVector<Example> t(2);
    t.Append(Example(1, 1));
    UT_EXPECT_EQ(t.Size(), 1);
    t.Clear();
    UT_EXPECT_EQ(t.Size(), 0);
}
