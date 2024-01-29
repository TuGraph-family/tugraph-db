﻿/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include "gtest/gtest.h"

#include "core/data_type.h"
#include "core/field_data_helper.h"
#include "core/type_convert.h"
#include "./ut_utils.h"
using namespace lgraph;
using namespace lgraph::field_data_helper;
using namespace lgraph::_detail;

class TestTypeConvert : public TuGraphTest {};

template <typename T>
inline T MaxMinus2() {
    return std::numeric_limits<T>::max() - 2;
}

TEST_F(TestTypeConvert, TypeConvert) {
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(MaxMinus2<int8_t>()), FieldType::INT8).AsInt8(),
                 MaxMinus2<int8_t>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<int16_t>()), FieldType::INT16).AsInt16(),
        MaxMinus2<int16_t>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<int32_t>()), FieldType::INT32).AsInt32(),
        MaxMinus2<int32_t>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<int64_t>()), FieldType::INT64).AsInt64(),
        MaxMinus2<int64_t>());
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(MaxMinus2<float>()), FieldType::FLOAT).AsFloat(),
                 MaxMinus2<float>());
    UT_EXPECT_EQ(
        ValueToFieldData(Value::ConstRef(MaxMinus2<double>()), FieldType::DOUBLE).AsDouble(),
        MaxMinus2<double>());
    std::string blobs = "for blob data";
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(blobs), FieldType::STRING).AsString(), blobs);
    std::string s = "to lbr data";
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(s), FieldType::STRING).AsString(), s);
    // Testing spatial data;
    std::string point_wgs = "0101000020E6100000000000000000F03F000000000000F03F";
    std::string point_cartesian = "0101000020231C0000000000000000F03F000000000000F03F";
    std::string linestring_wgs = "0102000020E61000000300000000000000000000000000"
    "000000000000000000000000004000000000000000400000000000000840000000000000F03F";
    std::string linestring_cartesian = "0102000020231C00000300000000000000000000000000"
    "000000000000000000000000004000000000000000400000000000000840000000000000F03F";
    std::string polygon_wgs = "0103000020E6100000010000000500000000000000000000000"
    "00000000000000000000000000000000000000000001C400000000000001040000000000000004"
    "00000000000000040000000000000000000000000000000000000000000000000";
    std::string polygon_cartesian = "0103000020231C0000010000000500000000000000000000000"
    "00000000000000000000000000000000000000000001C400000000000001040000000000000004"
    "00000000000000040000000000000000000000000000000000000000000000000";

    UT_EXPECT_TRUE(ValueToFieldData(Value::ConstRef(point_wgs), FieldType::POINT).AsWgsPoint() ==
    ::lgraph_api::Point<::lgraph_api::Wgs84>(point_wgs));
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(point_wgs), FieldType::POINT).ToString(),
    point_wgs);
    UT_EXPECT_TRUE(ValueToFieldData(Value::ConstRef(point_cartesian),
    FieldType::POINT).AsCartesianPoint() ==
    ::lgraph_api::Point<::lgraph_api::Cartesian>(point_cartesian));
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(point_cartesian), FieldType::POINT).ToString(),
    point_cartesian);
    UT_EXPECT_TRUE(ValueToFieldData(Value::ConstRef(linestring_wgs),
    FieldType::LINESTRING).AsWgsLineString() ==
    ::lgraph_api::LineString<::lgraph_api::Wgs84>(linestring_wgs));
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(linestring_wgs),
    FieldType::LINESTRING).ToString(), linestring_wgs);
    UT_EXPECT_TRUE(ValueToFieldData(Value::ConstRef(linestring_cartesian),
    FieldType::LINESTRING).AsCartesianLineString() ==
    ::lgraph_api::LineString<::lgraph_api::Cartesian>(linestring_cartesian));
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(linestring_cartesian),
    FieldType::LINESTRING).ToString(), linestring_cartesian);
    UT_EXPECT_TRUE(ValueToFieldData(Value::ConstRef(polygon_wgs),
    FieldType::POLYGON).AsWgsPolygon() ==
    ::lgraph_api::Polygon<::lgraph_api::Wgs84>(polygon_wgs));
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(polygon_wgs),
    FieldType::POLYGON).ToString(), polygon_wgs);
    UT_EXPECT_TRUE(ValueToFieldData(Value::ConstRef(polygon_cartesian),
    FieldType::POLYGON).AsCartesianPolygon() ==
    ::lgraph_api::Polygon<::lgraph_api::Cartesian>(polygon_cartesian));
    UT_EXPECT_EQ(ValueToFieldData(Value::ConstRef(polygon_cartesian),
    FieldType::POLYGON).ToString(),
    polygon_cartesian);
}
