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

#include <set>

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/graph_data_pack.h"
#include "./ut_utils.h"
using namespace fma_common;
using namespace lgraph;

class TestUnaligned : public TuGraphTest {};

static const int data_size = 1 << 20;
#ifdef _WIN32
static char value_data[data_size];
#else
static char value_data[data_size] __attribute__((aligned(4)));
#endif
static const int count = data_size / 4;
int64_t off = 0, sum_id = 0, sum_k = 0;

int CompareKeys(const MDB_val* a, const MDB_val* b) {
    const uint8_t* pa = (const uint8_t*)a->mv_data;
    size_t sa = a->mv_size;
    const uint8_t* pb = (const uint8_t*)b->mv_data;
    size_t sb = b->mv_size;
    int r = memcmp(pa, pb, std::min(sa, sb));
    return r < 0 ? -1 : r > 0 ? 1 : sa > sb ? 1 : sa < sb ? -1 : 0;
}

/* result:
 * for SefOffset and GetOffset, memcpy is as fast as unaligned access
 * */

TEST_F(TestUnaligned, Unaligned) {
    int loops = 1 << 28;
    fma_common::Configuration config;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    config.Add(loops, "l,loops", true);
    config.Parse(argc, argv);
    config.Finalize();
    using namespace lgraph::graph;
    UT_LOG() << "Testing " << __func__;

    double t1;
    t1 = fma_common::GetTime();
    for (int i = 0; i < loops; i++) {
        lgraph::_detail::SetOffset(value_data, i % count, i);
    }
    double t_set_off = fma_common::GetTime() - t1;

    t1 = fma_common::GetTime();
    for (int i = 0; i < loops; i++) {
        off += lgraph::_detail::GetOffset(value_data, i % count);
    }
    double t_get_off = fma_common::GetTime() - t1;

    t1 = fma_common::GetTime();
    for (int i = 0; i < loops; i++) {
        lgraph::_detail::WriteVid(value_data + 5 * (i % 1024), i);
    }
    double t_write_vid = fma_common::GetTime() - t1;

    t1 = fma_common::GetTime();
    for (int i = 0; i < loops; i++) {
        sum_id += lgraph::_detail::GetVid(value_data + 5 * (i % 1024));
    }
    double t_get_vid = fma_common::GetTime() - t1;

    // KeyCompareFunc performance
    Value v1 = KeyPacker::CreatePackedDataKey(2);
    Value v2 = KeyPacker::CreatePackedDataKey(1);
    MDB_val k1 = v1.MakeMdbVal();
    MDB_val k2 = v2.MakeMdbVal();
    t1 = fma_common::GetTime();
    for (int i = 0; i < loops; i++) {
        if (CompareKeys(&k1, &k2)) sum_k++;
    }
    double t_short_key_cmp = fma_common::GetTime() - t1;
    v1 = KeyPacker::CreateOutEdgeKey(EdgeUid(2, 0, 0, 1, 100));
    v2 = KeyPacker::CreateOutEdgeKey(EdgeUid(1, 0, 0, 2, 100));
    k1 = v1.MakeMdbVal();
    k2 = v2.MakeMdbVal();
    for (int i = 0; i < loops; i++) {
        if (CompareKeys(&k1, &k2)) sum_k++;
    }
    double t_long_key_cmp = fma_common::GetTime() - t1;

    UT_LOG() << "t_set_off = " << t_set_off << "(" << (double)loops / (1 << 20) / t_set_off
             << "mops)"
             << ", t_get_off = " << t_get_off << "(" << (double)loops / (1 << 20) / t_get_off
             << "mops)"
             << ", off = " << std::hex << off << ", t_write_vid = " << t_write_vid << "("
             << (double)loops / (1 << 20) / t_write_vid << "mops)"
             << ", t_get_vid = " << t_get_vid << "(" << (double)loops / (1 << 20) / t_get_vid
             << "mops)"
             << ", sum_id =" << std::hex << sum_id << ", t_short_key_cmp = " << t_short_key_cmp
             << "(" << (double)loops / (1 << 20) / t_short_key_cmp << "mops)"
             << ", t_long_key_cmp = " << t_long_key_cmp << "("
             << (double)loops / (1 << 20) / t_long_key_cmp << "mops)"
             << ", sum_k = " << sum_k;
}
