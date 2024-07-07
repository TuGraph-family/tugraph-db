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

//
// Created by hct on 18-12-29.
//

#include "fma-common/utils.h"
#include "server/proto_convert.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"

class TestProtoConvert : public TuGraphTest {};

TEST_F(TestProtoConvert, ProtoConvert) {
    using namespace lgraph;
    using namespace fma_common;
    // test field data convert
    {
        ProtoFieldData pfd;
        {
            FieldData fd((int64_t)123);
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(fd.AsInt64(), pfd.int64_());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_EQ(fd.AsInt64(), fd2.AsInt64());
        }
        {
            FieldData fd((double)3.14);
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(fd.AsDouble(), pfd.dp());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_EQ(fd.AsDouble(), fd2.AsDouble());
        }
        {
            FieldData fd("hello");
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(fd.AsString(), pfd.str());
            UT_EXPECT_TRUE(pfd.has_str());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_EQ(fd.AsString(), fd2.AsString());
        }

        // testing spatial data;
        {
            FieldData fd(PointWgs84(std::string("0101000020E6100000000000000000"
                                                "F03F0000000000000040")));
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_TRUE(fd.AsWgsPoint() == PointWgs84(pfd.point()));
            UT_EXPECT_TRUE(pfd.has_point());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_TRUE(fd.AsWgsPoint() == fd2.AsWgsPoint());
        }

        {
            FieldData fd(LineStringWgs84(std::string("0102000020E610000003000000000000000000000"
            "00000000000000000000000000000004000000000000000400000000000000840000000000000F03F")));
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_TRUE(fd.AsWgsLineString() == LineStringWgs84(pfd.linestring()));
            UT_EXPECT_TRUE(pfd.has_linestring());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_TRUE(fd.AsWgsLineString() == fd2.AsWgsLineString());
        }

        {
            FieldData fd(PolygonWgs84(std::string("0103000020E610000001000000050000000000000000"
            "000000000000000000000000000000000000000000000000001C400000000000001040000000000000"
            "00400000000000000040000000000000000000000000000000000000000000000000")));
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_TRUE(fd.AsWgsPolygon() == PolygonWgs84(pfd.polygon()));
            UT_EXPECT_TRUE(pfd.has_polygon());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_TRUE(fd.AsWgsPolygon() == fd2.AsWgsPolygon());
        }

        {
            FieldData fd(SpatialWgs84(std::string("0103000020E610000001000000050000000000000000"
            "000000000000000000000000000000000000000000000000001C400000000000001040000000000000"
            "00400000000000000040000000000000000000000000000000000000000000000000")));
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_TRUE(fd.AsWgsSpatial() == SpatialWgs84(pfd.spatial()));
            UT_EXPECT_TRUE(pfd.has_spatial());
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_TRUE(fd.AsWgsSpatial() == fd2.AsWgsSpatial());
        }

        // testing float vector data
        {
            std::vector<std::vector<float>> test_vector = {
                {1.111111, 2.111111, 3.111111, 4.111111, 5.111111},
                {1111111, 2111111, 3111111, 4111111, 5111111},
                {0.111111, 0.222222, 0.3333333},
                {111.1111, 222.2222, 333.3333},
                {111111.0, 222222.0, 333333.0},
                {1111, 2222, 3333}
            };
            for (size_t i = 0; i < test_vector.size(); i ++) {
                FieldData fd(test_vector[i]);
                FieldDataConvert::FromLGraphT(fd, &pfd);
                std::vector<float> vec2;
                auto size = pfd.fvector().fv_size();
                for (int i = 0; i < size; i++) {
                    vec2.push_back(pfd.fvector().fv(i));
                }
                UT_EXPECT_TRUE(fd.AsFloatVector() == vec2);
                UT_EXPECT_TRUE(pfd.has_fvector());
                auto fd2 = FieldDataConvert::ToLGraphT(pfd);
                UT_EXPECT_TRUE(fd.AsFloatVector() == fd2.AsFloatVector());
            }
        }

        {
            FieldData fd;
            FieldDataConvert::FromLGraphT(fd, &pfd);
            UT_EXPECT_EQ(pfd.Data_case(), pfd.DATA_NOT_SET);
            auto fd2 = FieldDataConvert::ToLGraphT(pfd);
            UT_EXPECT_TRUE(fd2.is_null());
        }
        {
            std::vector<FieldData> fds;
            fds.emplace_back((int64_t)333);
            fds.emplace_back((double)277.1);
            fds.emplace_back("test convert string");
            fds.emplace_back();
            ::google::protobuf::RepeatedPtrField<ProtoFieldData> pfds, pfds2;
            FieldDataConvert::FromLGraphT(fds, &pfds);
            auto fds2 = FieldDataConvert::ToLGraphT(pfds);
            UT_EXPECT_TRUE(fds.size() == fds2.size());
            for (size_t i = 0; i < fds.size(); i++) UT_EXPECT_TRUE(fds[i] == fds2[i]);
            FieldDataConvert::FromLGraphT(std::move(fds2), &pfds2);
            UT_EXPECT_TRUE(fds.size() == pfds2.size());
            for (size_t i = 0; i < fds.size(); i++)
                UT_EXPECT_TRUE(fds[i] ==
                               FieldDataConvert::ToLGraphT(pfds2.Get(static_cast<int>(i))));
        }
    }

    // test field spec convert
    {
        std::vector<FieldSpec> fss;
        fss.emplace_back("int8", FieldType::INT8, true);
        fss.emplace_back("int8", FieldType::INT16, false);
        fss.emplace_back("int8", FieldType::INT32, true);
        fss.emplace_back("int8", FieldType::INT64, true);
        fss.emplace_back("int8", FieldType::FLOAT, false);
        fss.emplace_back("int8", FieldType::DOUBLE, false);
        fss.emplace_back("int8", FieldType::STRING, true);
        fss.emplace_back("int8", FieldType::POINT, false);
        fss.emplace_back("int8", FieldType::LINESTRING, false);
        fss.emplace_back("int8", FieldType::POLYGON, false);
        fss.emplace_back("int8", FieldType::SPATIAL, false);
        fss.emplace_back("int8", FieldType::FLOAT_VECTOR, false);
        ::google::protobuf::RepeatedPtrField<ProtoFieldSpec> pfss, pfss2;
        FieldSpecConvert::FromLGraphT(fss, &pfss);
        UT_EXPECT_EQ(pfss.size(), fss.size());
        for (size_t i = 0; i < fss.size(); i++)
            UT_EXPECT_TRUE(fss[i] == FieldSpecConvert::ToLGraphT(pfss.Get(static_cast<int>(i))));
        auto fss2 = FieldSpecConvert::ToLGraphT(pfss);
        UT_EXPECT_EQ(fss.size(), fss2.size());
        for (size_t i = 0; i < fss.size(); i++) UT_EXPECT_TRUE(fss[i] == fss2[i]);
        FieldSpecConvert::FromLGraphT(std::move(fss2), &pfss2);
        UT_EXPECT_EQ(pfss2.size(), fss.size());
        for (size_t i = 0; i < fss.size(); i++)
            UT_EXPECT_TRUE(fss[i] == FieldSpecConvert::ToLGraphT(pfss2.Get(static_cast<int>(i))));
    }
}
