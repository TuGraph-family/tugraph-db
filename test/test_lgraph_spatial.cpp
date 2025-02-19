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

#include "lgraph/lgraph_spatial.h"

#include "fma-common/configuration.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "./graph_factory.h"
#include "./test_tools.h"
#include "./random_port.h"
#include "./ut_utils.h"

// class TestSpatial : public TuGraphTest {};
class TestSpatial : public TuGraphTestWithParam<bool> {};

INSTANTIATE_TEST_CASE_P(TestSpatial, TestSpatial, testing::Values(false, true));

static void CreateSampleDB(const std::string& dir, bool detach_property) {
    using namespace lgraph;
    lgraph::DBConfig conf;
    conf.dir = dir;
    lgraph::LightningGraph lg(conf);
    VertexOptions vo;
    vo.primary_field = "id";
    vo.detach_property = detach_property;

    UT_EXPECT_TRUE(lg.AddLabel(
        "spatial",
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false),
             FieldSpec("string2Point", FieldType::STRING, false),
             FieldSpec("Point", FieldType::POINT, false),
             FieldSpec("string2line", FieldType::STRING, false),
             FieldSpec("LineString", FieldType::LINESTRING, true),
             FieldSpec("string2Polygon", FieldType::STRING, true),
             FieldSpec("Polygon", FieldType::POLYGON, true),
             FieldSpec("string2Spatial", FieldType::STRING, true),
             FieldSpec("Spatial", FieldType::SPATIAL, true)}),
        true, vo));

    EdgeOptions options;
    options.temporal_field = "distance";
    options.detach_property = detach_property;
    UT_EXPECT_TRUE(lg.AddLabel(
                    "near",
                    std::vector<FieldSpec>({FieldSpec("distance", FieldType::INT64, true)}),
                    false, options));

    auto txn = lg.CreateWriteTxn();
    std::string Point_EWKB = "0101000020E6100000000000000000F03F0000000000000040";
    std::string Line_EWKB = "0102000020E61000000300000000000000000000000000000000"
    "000000000000000000004000000000000000400000000000000840000000000000F03F";
    std::string Polygon_EWKB = "0103000020231C0000010000000500000000000000000000000"
    "00000000000000000000000000000000000000000001C400000000000001040000000000000004"
    "00000000000000040000000000000000000000000000000000000000000000000";
    std::string Spatial_EWKB = "0102000020E61000000300000000000000000000000000000000"
    "000000000000000000004000000000000000400000000000000840000000000000F03F";
    VertexId v0 = txn.AddVertex(
    std::string("spatial"),
    std::vector<std::string>({"id", "string2Point", "Point",
    "string2line", "LineString", "string2Polygon", "Polygon", "string2Spatial", "Spatial"}),
    std::vector<std::string>({"1",
    Point_EWKB, Point_EWKB, Line_EWKB, Line_EWKB, Polygon_EWKB, Polygon_EWKB,
    Spatial_EWKB, Spatial_EWKB}));

    VertexId v1 = txn.AddVertex(
    std::string("spatial"),
    std::vector<std::string>({"id", "string2Point", "Point",
    "string2line", "LineString", "string2Polygon", "Polygon", "string2Spatial", "Spatial"}),
    std::vector<std::string>({"2",
    Point_EWKB, Point_EWKB, Line_EWKB, Line_EWKB, Polygon_EWKB, Polygon_EWKB,
    Spatial_EWKB, Spatial_EWKB}));

    // testting parse and set fielddata;
    VertexId v2 = txn.AddVertex(
    std::string("spatial"),
    std::vector<std::string>({"id", "string2Point", "Point",
    "string2line", "LineString", "string2Polygon", "Polygon", "string2Spatial", "Spatial"}),
    std::vector<FieldData>({FieldData::Int32(3), FieldData::String(Point_EWKB),
    FieldData::Point(Point_EWKB), FieldData::String(Line_EWKB),
    FieldData::LineString(Line_EWKB), FieldData::String(Polygon_EWKB),
    FieldData::Polygon(Polygon_EWKB), FieldData::String(Spatial_EWKB),
    FieldData::Spatial(Spatial_EWKB)}));

    // testing edge;
    txn.AddEdge(v0, v1, std::string("near"), std::vector<std::string>({"distance"}),
                std::vector<std::string>({"0"}));
    txn.AddEdge(v2, v1, std::string("near"), std::vector<std::string>({"distance"}),
                std::vector<std::string>({"1"}));

    txn.Commit();
}

// test whether spatial data error(wrong ewkb format)
// can be handled at schema level;
static void TestWrongDB(const std::string& dir, bool detach_property) {
    using namespace lgraph;
    lgraph::DBConfig conf;
    conf.dir = dir;
    lgraph::LightningGraph lg(conf);
    VertexOptions vo;
    vo.primary_field = "id";
    vo.detach_property = detach_property;

    UT_EXPECT_TRUE(lg.AddLabel(
        "spatial",
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false),
             FieldSpec("Point", FieldType::POINT, true),
             FieldSpec("line", FieldType::LINESTRING, true),
             FieldSpec("Polygon", FieldType::POLYGON, true),
             FieldSpec("Spatial", FieldType::SPATIAL, true)}),
        true, vo));
    {
        auto txn = lg.CreateWriteTxn();
        UT_EXPECT_ANY_THROW(txn.AddVertex(
            std::string("spatial"),
            std::vector<std::string>({"id", "Point", "line", "Polygon", "Spatial"}),
            std::vector<std::string>({"1",
            "aabbcsadsafasda",
            "01020000201234000003000000000000000000000000000000000000000"
            "00000000000004000000000000000400000000000000840000000000000F03F",
            "0103000020231C00000100000005000000000000000000000000000"
            "0000000000000000000000000000000000000001C40000000000000104000000000000"
            "000400000000000000040000000000000000000000000000000000000000000000000",
            "asdq348320#000!@"})));
        UT_EXPECT_ANY_THROW(
            txn.AddVertex(
                std::string("spatial"),
                std::vector<std::string>({"id", "Point", "line", "Polygon"}),
                std::vector<FieldData>({FieldData::Int32(1), FieldData::Point("asdasjcabkdsv"),
                FieldData::LineString("01020000201234000003000000000000000000000000000000000000000"
                "00000000000004000000000000000400000000000000840000000000000F03F"),
                FieldData::Polygon("0000000000000000000000000000000000000001C40000"
                "000000000104000"),
                FieldData::Spatial("jdasdu9038r1dai0iq")})));
    }

    {
        UT_EXPECT_TRUE(lg.AddLabel(
        "spatial_transform",
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false),
            FieldSpec("Point2string", FieldType::STRING, true),
             FieldSpec("line2string", FieldType::STRING, true),
             FieldSpec("Polygon2string", FieldType::STRING, true),
             FieldSpec("Spatial2string", FieldType::STRING, true)}),
        true, vo));

        auto txn = lg.CreateWriteTxn();

        txn.AddVertex(
            std::string("spatial_transform"),
            std::vector<std::string>({"id", "Point2string", "line2string", "Polygon2string",
            "Spatial2string"}),
            std::vector<std::string>({"1",
            "aabbcsadsafasda",
            "01020000201234000003000000000000000000000000000000000000000"
            "00000000000004000000000000000400000000000000840000000000000F03F",
            "0103000020231C00000100000005000000000000000000000000000"
            "0000000000000000000000000000000000000001C40000000000000104000000000000"
            "000400000000000000040000000000000000000000000000000000000000000000000",
            "01020000201234000003000000000000000000000000000000000000000"
            "00000000000004000000000000000400000000000000840000000000000F03F"}));

        size_t n_changed = 0;
        UT_EXPECT_ANY_THROW(lg.AlterLabelModFields(
            "spatial_transform",
            std::vector<FieldSpec>({FieldSpec("Point2string", FieldType::POINT, true),
                                    FieldSpec("LineString2string", FieldType::LINESTRING, true),
                                    FieldSpec("Polygon2string", FieldType::POLYGON, true),
                                    FieldSpec("Spatial2string", FieldType::SPATIAL, true)}),
                                    true, &n_changed));
    }
}

TEST_P(TestSpatial, Spatial) {
    using namespace lgraph_api;

    {
        UT_LOG() << "Testing encode and decode";

        // Testing point wkb format
        Spatial<Wgs84> Point_w(SRID::WGS84, SpatialType::POINT, 0,
        "0101000000000000000000F03F0000000000000040");
        // Output ewkb format are in capital
        UT_EXPECT_EQ(Point_w.AsEWKB(), "0101000020E6100000000000000000F03F0000000000000040");
        UT_EXPECT_EQ(Point_w.AsEWKT(), "SRID=4326;POINT(1 2)");
        UT_EXPECT_ANY_THROW(Spatial<Wgs84>(SRID::WGS84, SpatialType::POINT, 0, "1111111"));

        // Testing point wkt format
        Spatial<Cartesian> Point_c(SRID::CARTESIAN, SpatialType::POINT, 1, "POINT(1.0 1.0)");
        UT_EXPECT_EQ(Point_c.AsEWKB(), "0101000020231C0000000000000000F03F000000000000F03F");
        UT_EXPECT_EQ(Point_c.AsEWKT(), "SRID=7203;POINT(1 1)");
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::POINT, 1,
        "POINT(1.0 2.0 3.0)"));
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::POINT, 1, "POINT(a)"));

        // Testing linestring wkb format
        UT_EXPECT_ANY_THROW(Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 0, "1111111"));
        Spatial<Wgs84> line_w(SRID::WGS84, SpatialType::LINESTRING, 0, "01020000000300000000000000"
        "000000000000000000000000000000000000004000000000000000400000000000000840000000000000F03F");
        UT_EXPECT_EQ(line_w.AsEWKB(), "0102000020E610000003000000000000000000000000000000000000000"
        "00000000000004000000000000000400000000000000840000000000000F03F");
        UT_EXPECT_EQ(line_w.AsEWKT(), "SRID=4326;LINESTRING(0 0,2 2,3 1)");

        // Testing linestring wkt format
        Spatial<Cartesian> line_c(SRID::CARTESIAN, SpatialType::LINESTRING, 1,
        "LINESTRING(0 0,2 2,3 1)");
        UT_EXPECT_EQ(line_c.AsEWKB(), "0102000020231C00000300000000000000000000000000000000000"
        "000000000000000004000000000000000400000000000000840000000000000F03F");
        UT_EXPECT_EQ(line_c.AsEWKT(), "SRID=7203;LINESTRING(0 0,2 2,3 1)");
        UT_EXPECT_ANY_THROW(Spatial<Wgs84>(SRID::WGS84, SpatialType::LINESTRING, 1,
        "LINESTRING(1.0 2.0 a)"));
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::LINESTRING, 1,
        "LINE(0 0,2 2,3 1)"));

        // Testing polygon wkb format
        Spatial<Wgs84> Polygon_w(SRID::WGS84, SpatialType::POLYGON, 0,
        "0103000000010000000500000000000000000000000000000000000000000"
        "00000000000000000000000001C4000000000000010400000000000000040"
        "0000000000000040000000000000000000000000000000000000000000000000");
        UT_EXPECT_EQ(Polygon_w.AsEWKB(), "0103000020E6100000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
        "00000000040000000000000000000000000000000000000000000000000");
        UT_EXPECT_EQ(Polygon_w.AsEWKT(), "SRID=4326;POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_ANY_THROW(Spatial<Wgs84>(SRID::WGS84, SpatialType::POLYGON, 0, "abcde"));

        // Testing polygon wkt format;
        Spatial<Cartesian> Polygon_c(SRID::CARTESIAN, SpatialType::POLYGON, 1,
        "POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_EQ(Polygon_c.AsEWKB(), "0103000020231C0000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
        "00000000040000000000000000000000000000000000000000000000000");
        UT_EXPECT_EQ(Polygon_c.AsEWKT(), "SRID=7203;POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::WGS84, SpatialType::POLYGON, 1,
        "POLYGON(122)"));
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::POLYGON, 1,
        "POL((0 0,0 7,4 2,2 0,0 0))"));
    }

    {
        UT_LOG() << "Testing construct from EWKB";
        // Testing point ewkb format;
        std::string point_wgs84 = "0101000020E6100000000000000000F03F000000000000F03F";
        std::string point_cartesian = "0101000020231C0000000000000000F03F000000000000F03F";
        Spatial<Wgs84> p1(point_wgs84);
        Spatial<Cartesian> p2(point_cartesian);
        UT_EXPECT_EQ(p1.AsEWKB(), point_wgs84);
        UT_EXPECT_EQ(p1.AsEWKT(), "SRID=4326;POINT(1 1)");
        UT_EXPECT_EQ(p2.AsEWKB(), point_cartesian);
        UT_EXPECT_EQ(p2.AsEWKT(), "SRID=7203;POINT(1 1)");

        // Testing line ewkb format;
        std::string line_wgs84 = "0102000020E61000000300000000000000000000000000000000000000"
        "000000000000004000000000000000400000000000000840000000000000F03F";
        std::string line_cartesian = "0102000020231C00000300000000000000000000000000000000000000"
        "000000000000004000000000000000400000000000000840000000000000F03F";
        Spatial<Wgs84> l1(line_wgs84);
        Spatial<Cartesian> l2(line_cartesian);
        UT_EXPECT_EQ(l1.AsEWKB(), line_wgs84);
        UT_EXPECT_EQ(l1.AsEWKT(), "SRID=4326;LINESTRING(0 0,2 2,3 1)");
        UT_EXPECT_EQ(l2.AsEWKB(), line_cartesian);
        UT_EXPECT_EQ(l2.AsEWKT(), "SRID=7203;LINESTRING(0 0,2 2,3 1)");

        // Testing polygon ewkb format;
        std::string polygon_wgs84 = "0103000020E610000001000000050000000000000000000000"
        "00000000000000000000000000000000000000000000F03F000000000000F03F000000000000F03F"
        "000000000000F03F000000000000000000000000000000000000000000000000";
        std::string polygon_cartesian = "0103000020231C000001000000050000000000000000000000"
        "00000000000000000000000000000000000000000000F03F000000000000F03F000000000000F03F"
        "000000000000F03F000000000000000000000000000000000000000000000000";
        Spatial<Wgs84> po1(polygon_wgs84);
        Spatial<Cartesian> po2(polygon_cartesian);

        UT_EXPECT_EQ(po1.AsEWKB(), polygon_wgs84);
        UT_EXPECT_EQ(po1.AsEWKT(), "SRID=4326;POLYGON((0 0,0 1,1 1,1 0,0 0))");
        UT_EXPECT_EQ(po2.AsEWKB(), polygon_cartesian);
        UT_EXPECT_EQ(po2.AsEWKT(), "SRID=7203;POLYGON((0 0,0 1,1 1,1 0,0 0))");
    }

    {
        UT_LOG() << "Testing srid template and parameter dismatch";
        std::string Point_EWKB = "0101000020E6100000000000000000F03F000000000000F03F";
        std::string line_WKT = "LINESTRING(0 0,2 2,3 1)";
        Point<Wgs84> p(Point_EWKB);
        UT_EXPECT_ANY_THROW(Point<Cartesian> p_(Point_EWKB));
        UT_EXPECT_ANY_THROW(LineString<Wgs84> l_(SRID::CARTESIAN, SpatialType::LINESTRING, 1,
        line_WKT));
        UT_EXPECT_EQ(p.AsEWKB(), Point_EWKB);
        UT_EXPECT_EQ(p.AsEWKT(), "SRID=4326;POINT(1 1)");
    }

    {
        UT_LOG() << "Testing Point big endian";
        std::string big_Point_wkb = "00000000013FF00000000000003FF0000000000000";
        std::string big_Point_EWKB = "0000012000000010E63FF00000000000003FF0000000000000";
        std::string little_Point_EWKB = "0101000020E6100000000000000000F03F000000000000F03F";
        std::string little_Point_wkb = "0101000000000000000000F03F000000000000F03F";
        Point<Wgs84> p(SRID::WGS84, SpatialType::POINT, 0, big_Point_wkb);
        Point<Wgs84> p_(big_Point_EWKB);
        UT_EXPECT_EQ(p.AsEWKB(), little_Point_EWKB);
        UT_EXPECT_EQ(p.AsEWKT(), "SRID=4326;POINT(1 1)");
        UT_EXPECT_EQ(p_.AsEWKB(), little_Point_EWKB);
        UT_EXPECT_EQ(p_.AsEWKT(), "SRID=4326;POINT(1 1)");

        UT_LOG() << "Testing LineString big endian";
        std::string big_LineString_wkb = "0102000000030000000000000000000000000000"
        "0000000000000000000000004000000000000000400000000000000840000000000000F03F";
        std::string big_LineString_EWKB = "0102000020E61000000300000000000000000000000"
        "000000000000000000000000000004000000000000000400000000000000840000000000000F03F";
        std::string little_LineString_EWKB = big_LineString_EWKB;
        WkbEndianTransfer(big_LineString_wkb);
        big_LineString_EWKB = EwkbEndianTransfer(big_LineString_EWKB);
        LineString<Wgs84> l(SRID::WGS84, SpatialType::LINESTRING, 0, big_LineString_wkb);
        LineString<Wgs84> l_(big_LineString_EWKB);
        UT_EXPECT_EQ(l.AsEWKB(), little_LineString_EWKB);
        UT_EXPECT_EQ(l.AsEWKT(), "SRID=4326;LINESTRING(0 0,2 2,3 1)");
        UT_EXPECT_EQ(l_.AsEWKB(), little_LineString_EWKB);
        UT_EXPECT_EQ(l_.AsEWKT(), "SRID=4326;LINESTRING(0 0,2 2,3 1)");

        UT_LOG() << "Testing Polygon big endian";
        std::string big_Polygon_wkb = "010300000001000000050000000000000000000000000000000"
        "000000000000000000000000000000000001C4000000000000010400000000000000040"
        "0000000000000040000000000000000000000000000000000000000000000000";
        std::string big_Polygon_ewkb = "0103000020E6100000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C4000000000000010400000000000000040000"
        "0000000000040000000000000000000000000000000000000000000000000";
        std::string little_Polygon_ewkb = big_Polygon_ewkb;
        WkbEndianTransfer(big_Polygon_wkb);
        big_Polygon_ewkb = EwkbEndianTransfer(big_Polygon_ewkb);
        Polygon<Wgs84> po(SRID::WGS84, SpatialType::POLYGON, 0, big_Polygon_wkb);
        Polygon<Wgs84> po_(big_Polygon_ewkb);
        UT_EXPECT_EQ(po.AsEWKB(), little_Polygon_ewkb);
        UT_EXPECT_EQ(po.AsEWKT(), "SRID=4326;POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_EQ(po_.AsEWKB(), little_Polygon_ewkb);
        UT_EXPECT_EQ(po_.AsEWKT(), "SRID=4326;POLYGON((0 0,0 7,4 2,2 0,0 0))");

        UT_LOG() << "Testing Spatial big endian";
        Spatial<Wgs84> s(SRID::WGS84, SpatialType::POLYGON, 0, big_Polygon_wkb);
        Spatial<Wgs84> s_(big_Polygon_ewkb);
        UT_EXPECT_EQ(s.AsEWKB(), little_Polygon_ewkb);
        UT_EXPECT_EQ(s.AsEWKT(), "SRID=4326;POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_EQ(s_.AsEWKB(), little_Polygon_ewkb);
        UT_EXPECT_EQ(s_.AsEWKT(), "SRID=4326;POLYGON((0 0,0 7,4 2,2 0,0 0))");

        Spatial<Wgs84> sl(SRID::WGS84, SpatialType::LINESTRING, 0, big_LineString_wkb);
        Spatial<Wgs84> sl_(big_LineString_EWKB);
        UT_EXPECT_EQ(sl.AsEWKB(), little_LineString_EWKB);
        UT_EXPECT_EQ(sl.AsEWKT(), "SRID=4326;LINESTRING(0 0,2 2,3 1)");
        UT_EXPECT_EQ(sl_.AsEWKB(), little_LineString_EWKB);
        UT_EXPECT_EQ(sl_.AsEWKT(), "SRID=4326;LINESTRING(0 0,2 2,3 1)");
    }
}

TEST_P(TestSpatial, Spatial_Schema) {
    using namespace lgraph;
    bool enable_fast_alter = false;
    UT_LOG() << "Test Schema::AddspatialFields, DelspatialFields and ModspatialFields";
    Schema s1;
    s1.SetSchema(
        true,
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false),
             FieldSpec("Point", FieldType::POINT, false),
             FieldSpec("LineString", FieldType::LINESTRING, true),
             FieldSpec("Polygon", FieldType::POLYGON, true),
             FieldSpec("Spatial", FieldType::SPATIAL, true),
             FieldSpec("string2Point", FieldType::STRING, true),
             FieldSpec("string2line", FieldType::STRING, false),
             FieldSpec("string2Polygon", FieldType::STRING, true),
             FieldSpec("string2Spatial", FieldType::SPATIAL, true)}),
        "id", "", {}, {});
    std::map<std::string, FieldSpec> fields = s1.GetFiledSpecsAsMapIgnoreFieldId();
    // Add fields;
    {
        Schema s2(s1);
        s2.AddFields(std::vector<FieldSpec>({FieldSpec("Point2", FieldType::POINT, false)}));
        UT_EXPECT_TRUE(s2.GetFieldExtractor("Point2")->GetFieldSpec() ==
                       FieldSpec("Point2", FieldType::POINT, false, enable_fast_alter ? 9 : 0));
        auto fmap = s2.GetFiledSpecsAsMapIgnoreFieldId();
        UT_EXPECT_EQ(fmap.size(), fields.size() + 1);
        fmap.erase("Point2");
        UT_EXPECT_TRUE(fmap == fields);
    }

    // Mod fields;
    {
        Schema s2(s1);
        std::vector<FieldSpec> mod = {FieldSpec("string2Point", FieldType::POINT, false),
                                      FieldSpec("string2line", FieldType::LINESTRING, false),
                                      FieldSpec("string2Polygon", FieldType::POLYGON, false),
                                      FieldSpec("string2Spatial", FieldType::SPATIAL, false)};
        s2.ModFields(mod);
        UT_EXPECT_TRUE(!s2.HasBlob());
        auto fmap = s2.GetFiledSpecsAsMapIgnoreFieldId();
        auto old_fields = fields;
        for (auto& f : mod) {
            if (enable_fast_alter) {
                FieldId id = old_fields[f.name].id;
                old_fields[f.name].id = id;
            }
            old_fields[f.name] = f;
        }


        UT_EXPECT_TRUE(fmap == old_fields);
        UT_EXPECT_THROW_CODE(s2.ModFields(std::vector<FieldSpec>(
                            {FieldSpec("no_such_field", FieldType::BLOB, true)})),
                        FieldNotFound);
    }

    // Del fields;
    {
        Schema s2(s1);
        std::vector<std::string> to_del = {"Point", "LineString", "Polygon", "Spatial"};
        s2.DelFields(to_del);
        UT_EXPECT_TRUE(!s2.HasBlob());
        auto fmap = s2.GetFiledSpecsAsMapIgnoreFieldId();
        auto old_fields = fields;
        for (auto& f : to_del) old_fields.erase(f);
        UT_EXPECT_TRUE(fmap == old_fields);
    }

    UT_LOG() << "Testing set spatial schema and add vertex";
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);
    DBConfig conf;
    conf.dir = dir;
    CreateSampleDB(dir, GetParam());
    LightningGraph graph(conf);
    {
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Point")) == FieldData::Point
        ("0101000020E6100000000000000000F03F0000000000000040"));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("LineString")) == FieldData::LineString
        ("0102000020E610000003000000000000000000000000000000000000000"
        "00000000000004000000000000000400000000000000840000000000000F03F"));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Polygon")) == FieldData::Polygon
        ("0103000020231C0000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
        "00000000040000000000000000000000000000000000000000000000000"));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Spatial")) == FieldData::Spatial
        ("0102000020E610000003000000000000000000000000000000000000000"
        "00000000000004000000000000000400000000000000840000000000000F03F"));
    }

    UT_LOG() << "Testing add";
    {
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelAddFields(
            "spatial",
            std::vector<FieldSpec>({FieldSpec("Point2", FieldType::POINT, true),
                                    FieldSpec("LineString2", FieldType::LINESTRING, true),
                                    FieldSpec("Polygon2", FieldType::POLYGON, true),
                                    FieldSpec("Spatial2", FieldType::SPATIAL, true)}),
            std::vector<FieldData>({FieldData::Point
            ("0101000020231C0000000000000000F03F0000000000000040"),
            FieldData(), FieldData(), FieldData()}), true, &n_changed));
        UT_EXPECT_EQ(n_changed, 3);
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Point2")) == FieldData::Point
        ("0101000020231C0000000000000000F03F0000000000000040"));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Spatial2")) == FieldData());
    }

    if (!enable_fast_alter) {
        UT_LOG() << "Testing modify";
        {
            size_t n_changed = 0;
            UT_EXPECT_TRUE(graph.AlterLabelModFields(
                "spatial",
                std::vector<FieldSpec>({FieldSpec("string2Point", FieldType::POINT, false),
                                        FieldSpec("string2line", FieldType::LINESTRING, false),
                                        FieldSpec("string2Polygon", FieldType::POLYGON, false)}),
                true, &n_changed));
            UT_EXPECT_EQ(n_changed, 3);
            auto txn = graph.CreateReadTxn();
            UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("string2Point")) ==
                           FieldData::Point("0101000020E6100000000000000000F03F0000000000000040"));
            UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("string2line")) ==
                           FieldData::LineString(
                               "0102000020E610000003000000000000000000000000000000000000000"
                               "00000000000004000000000000000400000000000000840000000000000F03F"));
        }
    }

    UT_LOG() << "Testing Del";
    {
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelDelFields(
            "spatial", std::vector<std::string>({"Point", "LineString", "Polygon", "Spatial"}),
            true, &n_changed));
        UT_EXPECT_EQ(n_changed, enable_fast_alter ? 0 : n_changed);
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Point")) == FieldData());
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("LineString")) == FieldData());
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Polygon")) == FieldData());
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("Spatial")) == FieldData());
    }
}

TEST_P(TestSpatial, error_handle) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);
    DBConfig conf;
    conf.dir = dir;
    TestWrongDB(dir, GetParam());
}
