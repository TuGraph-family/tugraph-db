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
#include "fma-common/snappy_stream.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"
#include "lgraph/lgraph_date_time.h"
#include "lgraph/lgraph_spatial.h"

using namespace fma_common;
using namespace lgraph_api;

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

    // write fielddata
    BinaryWrite(outfile, FieldData());
    BinaryWrite(outfile, FieldData((int8_t)10));
    BinaryWrite(outfile, FieldData((int16_t)10));
    BinaryWrite(outfile, FieldData((int32_t)10));
    BinaryWrite(outfile, FieldData((int64_t)10));
    BinaryWrite(outfile, FieldData((float)10.0));
    BinaryWrite(outfile, FieldData((double)10.0));
    BinaryWrite(outfile, FieldData(Date("2088-01-22")));
    BinaryWrite(outfile, FieldData(DateTime("2088-01-22 11:22:33")));
    BinaryWrite(outfile, FieldData("hello"));
    std::string WKB_Point = "0101000000000000000000f03f0000000000000040";
    std::string EWKB_Point = "0101000020E6100000000000000000F03F0000000000000040";
    std::string EWKB_LineString =
        "0102000020E61000000300000000000000000000000000"
        "000000000000000000000000004000000000000000400000000000000840000000000000F03F";
    std::string EWKB_Polygon =
        "0103000020E6100000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004"
        "00000000000000040000000000000000000000000000000000000000000000000";
    std::vector<float> vec = {1.111, 2.111, 3.111, 4.111, 5.111, 6.111};
    BinaryWrite(outfile, FieldData::Point(EWKB_Point));
    BinaryWrite(outfile, FieldData::LineString(EWKB_LineString));
    BinaryWrite(outfile, FieldData::Polygon(EWKB_Polygon));
    BinaryWrite(outfile,
                FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0, WKB_Point)));
    BinaryWrite(outfile, FieldData::FloatVector(vec));

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
    FMA_UT_ASSERT(p2 == p);
    std::vector<int> v2;
    BinaryRead(infile, v2);
    FMA_UT_CHECK_EQ(v2.size(), v.size());
    for (size_t i = 0; i < v2.size(); i++) {
        FMA_UT_ASSERT(v[i] == v2[i]);
    }
    std::map<int, std::vector<int>> m2;
    BinaryRead(infile, m2);
    FMA_UT_CHECK_EQ(m2.size(), m.size());
    for (auto &kv : m2) {
        FMA_UT_ASSERT(m[kv.first].size() == v.size());
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

    FieldData data;
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData());
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData((int8_t)10));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData((int16_t)10));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData((int32_t)10));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData((int64_t)10));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData((float)10.0));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData((double)10.0));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData(Date("2088-01-22")));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData(DateTime("2088-01-22 11:22:33")));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData("hello"));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData::Point(EWKB_Point));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData::LineString(EWKB_LineString));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData::Polygon(EWKB_Polygon));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(
        data, FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0, WKB_Point)));
    BinaryRead(infile, data);
    FMA_UT_CHECK_EQ(data, FieldData::FloatVector(vec));

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
