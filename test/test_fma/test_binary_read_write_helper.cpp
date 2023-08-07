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

#include "fma-common/binary_read_write_helper.h"
#include "fma-common/fma_stream.h"
#include "fma-common/local_file_stream.h"
#include "fma-common/logging.h"
#include "fma-common/snappy_stream.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

using namespace std;

struct SomePod {
    bool operator==(const SomePod &rhs) { return x == rhs.x && y == rhs.y && z == rhs.z; }

    double z;
    int x;
    char y;
};

struct NonPod {
    int x;

    virtual void foo() {}
};

namespace some_namespace {
class Serializable {
 public:
    int x;
    int y;

    virtual void foo() {}

    size_t Serialize(OutputFmaStream &s) const { return BinaryWrite(s, x + 10); }

    size_t Deserialize(InputFmaStream &s) { return BinaryRead(s, x); }
};
}  // namespace some_namespace

FMA_SET_TEST_PARAMS(BinaryReadWriteHelper, "");

FMA_UNIT_TEST(BinaryReadWriteHelper) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    OutputFmaStream outfile("test.stream");
    // test premitive types
    BinaryWrite(outfile, (int)1);
    BinaryWrite(outfile, 'a');
    SomePod p;
    p.x = 10;
    p.y = 20;
    p.z = 30;
    BinaryWrite(outfile, p);
    // test container of pod
    std::vector<int> v;
    for (int i = 0; i < 10; i++) {
        v.push_back(i + 100);
    }
    BinaryWrite(outfile, v);
    // test non sequentail container of non pod
    std::map<int, std::vector<int>> m;
    for (int i = 0; i < 10; i++) {
        m[i] = v;
    }
    BinaryWrite(outfile, m);
    // the following code shoud fail compiling
    NonPod non_pod;
    // BinaryWrite(outfile, non_pod);

    // test serializable
    some_namespace::Serializable serializable;
    serializable.x = 101;
    BinaryWrite(outfile, serializable);
    outfile.Close();

    // reading back
    InputFmaStream infile("test.stream");
    int one;
    BinaryRead(infile, one);
    FMA_UT_CHECK_EQ(one, 1);
    char a;
    BinaryRead(infile, a);
    FMA_UT_CHECK_EQ(a, 'a');
    SomePod p2;
    BinaryRead(infile, p2);
    CHECK(p2 == p);
    std::vector<int> v2;
    BinaryRead(infile, v2);
    FMA_UT_CHECK_EQ(v2.size(), v.size());
    for (size_t i = 0; i < v2.size(); i++) {
        CHECK(v[i] == v2[i]);
    }
    std::map<int, std::vector<int>> m2;
    BinaryRead(infile, m2);
    FMA_UT_CHECK_EQ(m2.size(), m.size());
    for (auto &kv : m2) {
        CHECK(m[kv.first].size() == v.size());
    }
    int sum = 0;
    for (const auto &kv : m) {
        for (const auto &i : kv.second) sum += i;
    }
    int sum2 = 0;
    for (const auto &kv : m2) {
        for (const auto &i : kv.second) sum2 += i;
    }
    FMA_UT_CHECK_EQ(sum, sum2);
    // the following should fail compiling
    // BinaryRead(infile, non_pod);

    serializable.x = 0;
    BinaryRead(infile, serializable);
    FMA_UT_CHECK_EQ(serializable.x, 111);

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
