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

#include "fma-common/string_formatter.h"
#include "gtest/gtest.h"
#include "./ut_utils.h"
#include "core/data_type.h"
#include "./test_tools.h"

class TestDataType : public TuGraphTest {};

template <size_t N>
void TestNBytesIdSetAndLoad(int64_t value) {
    using namespace fma_common;
    using namespace lgraph;
    using namespace ::lgraph::_detail;
    Value v(64);
    SetNByteIdToBuf<N>(v.Data(), value);
    UT_EXPECT_EQ(GetNByteIdFromBuf<N>(v.Data()), value);
}

TEST_F(TestDataType, DataType) {
    using namespace fma_common;
    using namespace lgraph;
    using namespace lgraph::_detail;
    using namespace lgraph_api;
    UT_LOG() << "Testing FieldData";
    {
        // data prepared for spatial type;
        std::string WKB_Point = "0101000000000000000000f03f0000000000000040";
        std::string EWKB_Point = "0101000020E6100000000000000000F03F0000000000000040";
        std::string WKB_LineString = "0102000000030000000000000000000000000000000"
        "0000000000000000000004000000000000000400000000000000840000000000000F03F";
        std::string EWKB_LineString = "0102000020E61000000300000000000000000000000000"
        "000000000000000000000000004000000000000000400000000000000840000000000000F03F";
        std::string WKB_Polygon = "01030000000100000005000000000000000000000000000000"
        "0000000000000000000000000000000000001C4000000000000010400000000000000040"
        "0000000000000040000000000000000000000000000000000000000000000000";
        std::string EWKB_Polygon = "0103000020E6100000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004"
        "00000000000000040000000000000000000000000000000000000000000000000";

        // data prepared for vector
        std::vector<float> vec1 = {1.111, 2.111, 3.111, 4.111, 5.111, 6.111};
        std::vector<float> vec2 = {1111111, 2111111, 3111111, 4111111, 5111111, 6111111};
        std::vector<float> vec3 = {1.111111, 2.111111, 3.111111, 4.111111, 5.111111};
        std::vector<float> vec4 = {1111.000, 2222.000, 3333.000};
        std::vector<float> vec5 = {111111.0, 222222.0, 333333.0};
        std::vector<float> vec6 = {1111, 2222, 3333};

        // is_null and is_buf
        UT_EXPECT_TRUE(FieldData().is_null());
        UT_EXPECT_TRUE(!FieldData(123).is_null());
        UT_EXPECT_TRUE(!FieldData().is_buf());
        UT_EXPECT_TRUE(FieldData("str").is_buf());
        UT_EXPECT_TRUE(FieldData::Blob("str").is_buf());
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0,
        WKB_Point)).is_buf());
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(EWKB_Point)).is_buf());
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 0,
        WKB_LineString)).is_buf());
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(EWKB_LineString)).is_buf());
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POLYGON, 0,
        WKB_Polygon)).is_buf());
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(EWKB_Polygon)).is_buf());
        UT_EXPECT_FALSE(FieldData::FloatVector(vec1).is_buf());
        UT_EXPECT_FALSE(FieldData::FloatVector(vec2).is_buf());
        UT_EXPECT_FALSE(FieldData::FloatVector(vec3).is_buf());
        UT_EXPECT_FALSE(FieldData::FloatVector(vec4).is_buf());
        UT_EXPECT_FALSE(FieldData::FloatVector(vec5).is_buf());
        UT_EXPECT_FALSE(FieldData::FloatVector(vec6).is_buf());

        // Constructor and AsXXX()
        UT_EXPECT_EQ(FieldData(true).AsBool(), true);
        UT_EXPECT_EQ(FieldData(false).AsBool(), false);
        UT_EXPECT_ANY_THROW(FieldData(false).AsInt8());
        UT_EXPECT_EQ(FieldData::Int8(12).AsInt8(), 12);
        UT_EXPECT_ANY_THROW(FieldData::Int8(12).AsBool());
        UT_EXPECT_EQ(FieldData::Int16(32767).AsInt16(), 32767);
        UT_EXPECT_ANY_THROW(FieldData::Int16(12).AsInt32());
        UT_EXPECT_EQ(FieldData::Int32(1 << 30).AsInt32(), 1 << 30);
        UT_EXPECT_ANY_THROW(FieldData::Int32(12).AsInt16());
        UT_EXPECT_EQ(FieldData::Int64((int64_t)1 << 40).AsInt64(), (int64_t)1 << 40);
        UT_EXPECT_ANY_THROW(FieldData::Int64(12).AsFloat());
        UT_EXPECT_EQ(FieldData::Float((float)3.14).AsFloat(), (float)3.14);
        UT_EXPECT_ANY_THROW(FieldData::Float((float)12.12).AsInt16());
        UT_EXPECT_EQ(FieldData::Double(32.14).AsDouble(), 32.14);
        UT_EXPECT_ANY_THROW(FieldData::Double(12.12).AsFloat());
        UT_EXPECT_TRUE(FieldData::Date("2088-01-22").AsDate() == Date("2088-01-22"));
        UT_EXPECT_ANY_THROW(FieldData::Date("2088-01-22").AsDateTime());
        UT_EXPECT_TRUE(FieldData::DateTime("2088-01-22 11:22:33").AsDateTime() ==
                       DateTime("2088-01-22 11:22:33"));
        UT_EXPECT_ANY_THROW(FieldData::DateTime("2088-01-22 11:22:33").AsString());
        UT_EXPECT_EQ(FieldData::String("str").AsString(), "str");
        UT_EXPECT_ANY_THROW(FieldData::String("str").AsBlob());
        UT_EXPECT_EQ(FieldData::Blob(std::string(12, -1)).AsBlob(), std::string(12, -1));
        UT_EXPECT_EQ(FieldData::Blob(std::string(12, -1)).AsBase64Blob(),
                     ::lgraph_api::base64::Encode(std::string(12, -1)));
        UT_EXPECT_ANY_THROW(FieldData::Blob(std::string(12, -1)).AsDate());
        std::string orig = "orig string";
        UT_EXPECT_EQ(FieldData::BlobFromBase64(::lgraph_api::base64::Encode(orig)).AsBlob(), orig);


        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0,
        WKB_Point)).AsWgsSpatial() == Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0,
        WKB_Point));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 0,
        WKB_LineString)).AsWgsSpatial() == Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 0,
        WKB_LineString));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POLYGON, 0,
        WKB_Polygon)).AsWgsSpatial() == Spatial<Wgs84>(SRID::WGS84, SpatialType::POLYGON, 0,
        WKB_Polygon));
        UT_EXPECT_TRUE(FieldData::Point(EWKB_Point).AsWgsPoint() == Point<Wgs84>(EWKB_Point));
        UT_EXPECT_TRUE(FieldData::LineString(EWKB_LineString).AsWgsLineString() ==
        LineString<Wgs84>(EWKB_LineString));
        UT_EXPECT_TRUE(FieldData::Polygon(EWKB_Polygon).AsWgsPolygon() ==
        Polygon<Wgs84>(EWKB_Polygon));
        UT_EXPECT_EQ(FieldData::FloatVector(vec1).AsFloatVector(), vec1);
        UT_EXPECT_EQ(FieldData::FloatVector(vec2).AsFloatVector(), vec2);
        UT_EXPECT_EQ(FieldData::FloatVector(vec3).AsFloatVector(), vec3);
        UT_EXPECT_EQ(FieldData::FloatVector(vec4).AsFloatVector(), vec4);
        UT_EXPECT_EQ(FieldData::FloatVector(vec5).AsFloatVector(), vec5);
        UT_EXPECT_EQ(FieldData::FloatVector(vec6).AsFloatVector(), vec6);

        // ToString
        UT_EXPECT_EQ(FieldData::Bool(true).ToString(), "true");
        UT_EXPECT_EQ(FieldData::Bool(false).ToString(), "false");
        UT_EXPECT_EQ(FieldData::Int8(127).ToString(), "127");
        UT_EXPECT_EQ(FieldData::Int16(256).ToString(), "256");
        UT_EXPECT_EQ(FieldData::Int32(1000000).ToString(), "1000000");
        UT_EXPECT_EQ(FieldData::Int64(10000000000L).ToString(), "10000000000");
        UT_EXPECT_EQ(FieldData::Float((float)127.25).ToString(), std::to_string((float)127.25));
        UT_EXPECT_EQ(FieldData::Double(-127.25).ToString(), std::to_string(-127.25));
        UT_EXPECT_EQ(FieldData::Date("2088-01-22").ToString(), "2088-01-22");
        UT_EXPECT_EQ(FieldData::DateTime("2088-01-22 11:22:33").ToString(), "2088-01-22 11:22:33");
        UT_EXPECT_EQ(FieldData::String("str").ToString(), "str");
        UT_EXPECT_EQ(FieldData::Blob(std::string(12, -11)).ToString(),
                     ::lgraph_api::base64::Encode(std::string(12, -11)));
        UT_EXPECT_EQ(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0,
        WKB_Point)).ToString(), EWKB_Point);
        UT_EXPECT_EQ(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 0,
        WKB_LineString)).ToString(), EWKB_LineString);
        UT_EXPECT_EQ(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POLYGON, 0,
        WKB_Polygon)).ToString(), EWKB_Polygon);
        UT_EXPECT_EQ(FieldData::Point(Point<Wgs84>(EWKB_Point)).ToString(), EWKB_Point);
        UT_EXPECT_EQ(FieldData::LineString(LineString<Wgs84>
        (EWKB_LineString)).ToString(), EWKB_LineString);
        UT_EXPECT_EQ(FieldData::Polygon(Polygon<Wgs84>(EWKB_Polygon)).ToString(), EWKB_Polygon);
        UT_EXPECT_EQ(FieldData::FloatVector(vec1).ToString(),
                     "1.111000,2.111000,3.111000,4.111000,5.111000,6.111000");
        UT_EXPECT_EQ(FieldData::FloatVector(vec2).ToString(),
                     "1111111,2111111,3111111,4111111,5111111,6111111");
        UT_EXPECT_EQ(FieldData::FloatVector(vec3).ToString(),
                     "1.111111,2.111111,3.111111,4.111111,5.111111");
        UT_EXPECT_EQ(FieldData::FloatVector(vec4).ToString(),
                     "1111.000,2222.000,3333.000");
        UT_EXPECT_EQ(FieldData::FloatVector(vec5).ToString(),
                     "111111.0,222222.0,333333.0");
        UT_EXPECT_EQ(FieldData::FloatVector(vec6).ToString(),
                     "1111.000,2222.000,3333.000");

        // compare operators
        UT_EXPECT_ANY_THROW(FieldData::Int8(1) == FieldData::Bool(false));
        UT_EXPECT_TRUE(FieldData() == FieldData());
        UT_EXPECT_TRUE(!(FieldData(12) == FieldData()));
        UT_EXPECT_TRUE(FieldData(12) != FieldData());
        UT_EXPECT_TRUE(FieldData::Bool(true) == FieldData::Bool(true));
        UT_EXPECT_TRUE(!(FieldData::Bool(true) == FieldData::Bool(false)));
        UT_EXPECT_TRUE(FieldData::Int8(12) == FieldData::Int8(12));
        UT_EXPECT_TRUE(!(FieldData::Int8(11) == FieldData::Int8(12)));
        UT_EXPECT_TRUE(FieldData::Int16(128) == FieldData::Int16(128));
        UT_EXPECT_TRUE(!(FieldData::Int16(11) == FieldData::Int16(12)));
        UT_EXPECT_TRUE(FieldData::Int32(65536) == FieldData::Int32(65536));
        UT_EXPECT_TRUE(!(FieldData::Int32(11) == FieldData::Int32(12)));
        UT_EXPECT_TRUE(FieldData::Int64(1000000000L) == FieldData::Int64(1000000000L));
        UT_EXPECT_TRUE(!(FieldData::Int64(11) == FieldData::Int64(12)));
        UT_EXPECT_TRUE(FieldData::Float((float)12.11) == FieldData::Float((float)12.11));
        UT_EXPECT_TRUE(!(FieldData::Float(11) == FieldData::Float(12)));
        UT_EXPECT_TRUE(FieldData::Double(111.12) == FieldData::Double(111.12));
        UT_EXPECT_TRUE(!(FieldData::Double(11) == FieldData::Double(12)));
        UT_EXPECT_TRUE(FieldData::Date("2222-12-12") == FieldData::Date("2222-12-12"));
        UT_EXPECT_TRUE(FieldData::Date("2222-12-12") != FieldData::Date("2222-12-11"));
        UT_EXPECT_TRUE(FieldData::DateTime("2222-12-12 11:22:33") ==
                       FieldData::DateTime("2222-12-12 11:22:33"));
        UT_EXPECT_TRUE(FieldData::DateTime("2222-12-12 11:22:33") !=
                       FieldData::DateTime("2222-12-12 11:22:34"));
        UT_EXPECT_TRUE(FieldData::String("123") == FieldData::String("123"));
        UT_EXPECT_TRUE(FieldData::Blob("123") == FieldData::Blob("123"));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0,
        WKB_Point)) ==
        FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0, WKB_Point)));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(EWKB_Point))
        == FieldData::Spatial(Spatial<Wgs84>(EWKB_Point)));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 0,
        WKB_LineString)) == FieldData::Spatial
        (Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 0, WKB_LineString)));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(EWKB_LineString))
        == FieldData::Spatial(Spatial<Wgs84>(EWKB_LineString)));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(SRID::WGS84, SpatialType::POLYGON, 0,
        WKB_Polygon)) == FieldData::Spatial
        (Spatial<Wgs84>(SRID::WGS84, SpatialType::POLYGON, 0, WKB_Polygon)));
        UT_EXPECT_TRUE(FieldData::Spatial(Spatial<Wgs84>(EWKB_Polygon))
        == FieldData::Spatial(Spatial<Wgs84>(EWKB_Polygon)));

        UT_EXPECT_FALSE(FieldData::FloatVector(vec1) == FieldData::FloatVector(vec1));
        UT_EXPECT_FALSE(FieldData::FloatVector(vec2) == FieldData::FloatVector(vec2));
        UT_EXPECT_FALSE(FieldData::FloatVector(vec3) == FieldData::FloatVector(vec3));
        UT_EXPECT_FALSE(FieldData::FloatVector(vec4) == FieldData::FloatVector(vec4));
        UT_EXPECT_FALSE(FieldData::FloatVector(vec5) == FieldData::FloatVector(vec5));
        UT_EXPECT_FALSE(FieldData::FloatVector(vec6) == FieldData::FloatVector(vec6));

        UT_EXPECT_ANY_THROW(FieldData::Int8(1) > FieldData::Bool(false));
        UT_EXPECT_TRUE(FieldData::Int8(1) > FieldData::Int8(-10));
        UT_EXPECT_TRUE(FieldData::Int16(1000) > FieldData::Int16(-10000));
        UT_EXPECT_TRUE(FieldData::Int32(1000000) > FieldData::Int32(-10000000));
        UT_EXPECT_TRUE(FieldData::Int64(10000000000L) > FieldData::Int64(-100000));
        UT_EXPECT_TRUE(FieldData::Float((float)1.2) > FieldData::Float((float)0.123));
        UT_EXPECT_TRUE(FieldData::Double(1.2) > FieldData::Double(-0.123));
        UT_EXPECT_TRUE(FieldData::Date("2222-12-12") > FieldData::Date("2222-12-11"));
        UT_EXPECT_TRUE(FieldData::DateTime("2222-12-12 11:22:33") >
                       FieldData::DateTime("2222-12-12 11:22:30"));
        UT_EXPECT_TRUE(FieldData::String("123") > FieldData::String("12"));
        UT_EXPECT_TRUE(FieldData::Blob("123") > FieldData::Blob("12"));

        // test operator on date
        Date date1 = Date::Now();
        std::string date_t = date1.ToString();
        auto date2 = date1;
        UT_EXPECT_TRUE(date2 == Date(date_t));
        UT_EXPECT_TRUE((date2 + 1) > Date(date_t));
        UT_EXPECT_TRUE((date2 + 1) >= Date(date_t));
        UT_EXPECT_TRUE((date2 - 1) < Date(date_t));
        UT_EXPECT_TRUE((date2 - 1) <= Date(date_t));
        date2 -= 1;
        UT_EXPECT_TRUE(date2 == (Date(date_t) - 1));
        UT_EXPECT_TRUE(date2 != Date(date_t));
        date2 += 1;
        UT_EXPECT_TRUE(date2 == Date(date_t));
        UT_EXPECT_TRUE(Date(date2.GetStorage()).ToString() == date_t);
        UT_EXPECT_TRUE(Date(date2.GetYearMonthDay()).ToString() == date_t);
        // test datetime exception (Erroneous formated and Over range data)
        date_t = "2019-02-29";
        date1 = Date(date_t);
        date_t = "2019-00-29";
        UT_EXPECT_ANY_THROW(date1 = Date(date_t));
        date_t = "2019-13-29";
        UT_EXPECT_ANY_THROW(date1 = Date(date_t));
        date_t = "2019-12-32";
        UT_EXPECT_ANY_THROW(date1 = Date(date_t));
        UT_EXPECT_ANY_THROW(Date("1970-01-32"));
        UT_EXPECT_ANY_THROW(Date("1970-13-00"));
        UT_EXPECT_ANY_THROW(Date("1970-00-00"));
        UT_EXPECT_ANY_THROW(Date("1970-01-00"));
        Date("9999-01-01");
        Date("0000-01-01");
        // test datetime exception (Erroneous formated and Over range data)
        UT_EXPECT_ANY_THROW(DateTime("1970-01-01 00:00:60"));
        DateTime("0000-01-01 00:00:00");
        UT_EXPECT_ANY_THROW(DateTime("1970-01-01 00:60:00"));
        UT_EXPECT_ANY_THROW(DateTime("1970-01-01 24:00:00"));
        UT_EXPECT_ANY_THROW(DateTime("1970-01-32 00:00:00"));
        UT_EXPECT_ANY_THROW(DateTime("1970-13-01 00:00:00"));
        UT_EXPECT_ANY_THROW(DateTime("-192-13-01 00:00:00"));
        UT_EXPECT_ANY_THROW(DateTime("1970-00-01 00:00:00"));
        UT_EXPECT_ANY_THROW(DateTime("1970--3-01 00:00:00"));
        UT_EXPECT_ANY_THROW(DateTime("1970-10-0100:00:00"));
        DateTime("9999-12-01 00:00:00");
    }

    UT_LOG() << "Testing EdgeUid";
    {
        EdgeUid::OutEdgeSortOrder oes;
        EdgeUid::InEdgeSortOrder ies;
        UT_EXPECT_TRUE(oes(EdgeUid(), EdgeUid(0, 0, 0, 0, 1)));
        UT_EXPECT_TRUE(oes(EdgeUid(), EdgeUid(0, 0, 0, 1, 0)));
        UT_EXPECT_TRUE(oes(EdgeUid(), EdgeUid(0, 0, 1, 0, 0)));
        UT_EXPECT_TRUE(oes(EdgeUid(), EdgeUid(0, 1, 0, 0, 0)));
        UT_EXPECT_TRUE(oes(EdgeUid(), EdgeUid(1, 0, 0, 0, 0)));
        UT_EXPECT_TRUE(oes(EdgeUid(0, 0, 1, 1, 1), EdgeUid(1, 0, 0, 0, 0)));
        UT_EXPECT_TRUE(oes(EdgeUid(1, 0, 0, 0, 1), EdgeUid(1, 0, 1, 0, 0)));
        UT_EXPECT_TRUE(oes(EdgeUid(1, 0, 0, 0, 1), EdgeUid(1, 0, 0, 1, 0)));
        UT_EXPECT_TRUE(ies(EdgeUid(), EdgeUid(0, 0, 0, 0, 1)));
        UT_EXPECT_TRUE(ies(EdgeUid(), EdgeUid(0, 0, 0, 1, 0)));
        UT_EXPECT_TRUE(ies(EdgeUid(), EdgeUid(0, 0, 1, 0, 0)));
        UT_EXPECT_TRUE(ies(EdgeUid(), EdgeUid(0, 1, 0, 0, 0)));
        UT_EXPECT_TRUE(ies(EdgeUid(), EdgeUid(1, 0, 0, 0, 0)));
        UT_EXPECT_TRUE(ies(EdgeUid(1, 0, 0, 1, 1), EdgeUid(0, 1, 0, 0, 0)));
        UT_EXPECT_TRUE(ies(EdgeUid(0, 1, 0, 0, 1), EdgeUid(1, 1, 0, 0, 0)));
        UT_EXPECT_TRUE(ies(EdgeUid(1, 1, 0, 0, 1), EdgeUid(1, 1, 0, 1, 0)));
    }

    UT_LOG() << "Testing TestNBytesIdSetAndLoad";
    {
#define TESTNBSAL(n) TestNBytesIdSetAndLoad<n>((int64_t)((((uint64_t)1) << (n * 8 - 1)) - 2));

        TESTNBSAL(1);
        TESTNBSAL(2);
        TESTNBSAL(3);
        TESTNBSAL(4);
        TESTNBSAL(5);
        TESTNBSAL(6);
        TESTNBSAL(7);
        TESTNBSAL(8);
    }

    UT_LOG() << "Testing WriteVid, GetSrcVid, GetEid, GetOffset, SetOffset";
    {
        char buf[8];
        int64_t large_vid = (int64_t)(((uint64_t)1 << (VID_SIZE * 8 - 1)) - 2);
        WriteVid(buf, large_vid);
        UT_EXPECT_EQ(GetVid(buf), large_vid);
        int64_t large_eid = (int64_t)(((uint64_t)1 << (EID_SIZE * 8 - 1)) - 2);
        WriteEid(buf, large_eid);
        UT_EXPECT_EQ(GetEid(buf), large_eid);
        PackDataOffset large_offset =
            (PackDataOffset)(((uint64_t)1 << (sizeof(PackDataOffset) * 8 - 1)) - 2);
        SetOffset(buf, 0, large_offset);
        UT_EXPECT_EQ(GetOffset(buf, 0), large_offset);

        UT_EXPECT_ANY_THROW(CheckVid((VertexId)-1));
        UT_EXPECT_ANY_THROW(CheckEid((EdgeId)-1));
        UT_EXPECT_ANY_THROW(CheckLid((LabelId)-1));
    }

    // exception with stacktrace
    {
        std::runtime_error e("my exception");
        std::string ret = PrintNestedException(e, 1);
        UT_EXPECT_EQ(ret, "  my exception");
    }
}
