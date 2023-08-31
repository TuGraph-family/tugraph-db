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
#include "fma-common/logger.h"
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
             FieldSpec("string2point", FieldType::STRING, false),
             FieldSpec("point", FieldType::POINT, false),
             FieldSpec("string2line", FieldType::STRING, false),
             FieldSpec("linestring", FieldType::LINESTRING, true),
             FieldSpec("polygon", FieldType::POLYGON, true)}),
        true, vo));

    EdgeOptions options;
    options.temporal_field = "distance";
    options.detach_property = detach_property;
    UT_EXPECT_TRUE(lg.AddLabel(
                    "near",
                    std::vector<FieldSpec>({FieldSpec("distance", FieldType::INT64, true)}),
                    false, options));

    auto txn = lg.CreateWriteTxn();

    VertexId v0 = txn.AddVertex(
    std::string("spatial"),
    std::vector<std::string>({"id", "string2point", "point",
    "string2line", "linestring", "polygon"}),
    std::vector<std::string>({"1",
    "0101000020E6100000000000000000F03F0000000000000040",
    "0101000020E6100000000000000000F03F0000000000000040",
    "0102000020E610000003000000000000000000000000000000000000000"
    "00000000000004000000000000000400000000000000840000000000000F03F",
    "0102000020E610000003000000000000000000000000000000000000000"
    "00000000000004000000000000000400000000000000840000000000000F03F",
    "0103000020231C0000010000000500000000000000000000000"
    "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
    "00000000040000000000000000000000000000000000000000000000000"}));

    // 这里的primary_key不能相同;
    VertexId v1 = txn.AddVertex(
    std::string("spatial"),
    std::vector<std::string>({"id", "string2point", "point",
    "string2line", "linestring", "polygon"}),
    std::vector<std::string>({"2",
    "0101000020E6100000000000000000F03F0000000000000040",
    "0101000020E6100000000000000000F03F0000000000000040",
    "0102000020E610000003000000000000000000000000000000000000000"
    "00000000000004000000000000000400000000000000840000000000000F03F",
    "0102000020E610000003000000000000000000000000000000000000000"
    "00000000000004000000000000000400000000000000840000000000000F03F",
    "0103000020231C0000010000000500000000000000000000000"
    "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
    "00000000040000000000000000000000000000000000000000000000000"}));

    txn.AddEdge(v0, v1, std::string("near"), std::vector<std::string>({"distance"}),
                std::vector<std::string>({"0"}));
    txn.AddEdge(v0, v1, std::string("near"), std::vector<std::string>({"distance"}),
                std::vector<std::string>({"1"}));

    txn.Commit();
}

static void TestSampleDB(const std::string& dir, bool detach_property) {
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
             FieldSpec("point", FieldType::POINT, true),
             FieldSpec("line", FieldType::LINESTRING, true),
             FieldSpec("polygon", FieldType::POLYGON, true)}),
        true, vo));
    {
        auto txn = lg.CreateWriteTxn();
        UT_EXPECT_ANY_THROW(txn.AddVertex(
            std::string("spatial"),
            std::vector<std::string>({"id", "point", "line", "polygon"}),
            std::vector<std::string>({"1",
            "aabbcsadsafasda",
            "01020000201234000003000000000000000000000000000000000000000"
            "00000000000004000000000000000400000000000000840000000000000F03F",
            "0103000020231C00000100000005000000000000000000000000000"
            "0000000000000000000000000000000000000001C40000000000000104000000000000"
            "000400000000000000040000000000000000000000000000000000000000000000000"})));
    }

    {
        UT_EXPECT_TRUE(lg.AddLabel(
        "spatial_transform",
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false),
            FieldSpec("point2string", FieldType::STRING, true),
             FieldSpec("line2string", FieldType::STRING, true),
             FieldSpec("polygon2string", FieldType::STRING, true)}),
        true, vo));

        auto txn = lg.CreateWriteTxn();

        txn.AddVertex(
            std::string("spatial_transform"),
            std::vector<std::string>({"id", "point2string", "line2string", "polygon2string"}),
            std::vector<std::string>({"1",
            "aabbcsadsafasda",
            "01020000201234000003000000000000000000000000000000000000000"
            "00000000000004000000000000000400000000000000840000000000000F03F",
            "0103000020231C00000100000005000000000000000000000000000"
            "0000000000000000000000000000000000000001C40000000000000104000000000000"
            "000400000000000000040000000000000000000000000000000000000000000000000"}));

        size_t n_changed = 0;
        UT_EXPECT_ANY_THROW(lg.AlterLabelModFields(
            "spatial_transform",
            std::vector<FieldSpec>({FieldSpec("point2string", FieldType::POINT, true),
                                    FieldSpec("linestring2string", FieldType::LINESTRING, true),
                                    FieldSpec("polygon2string", FieldType::POLYGON, true)}),
                                    true, &n_changed));
    }
}

TEST_P(TestSpatial, Spatial) {
    using namespace lgraph_api;

    {
        UT_LOG() << "Testing encode and decode";

        Spatial<Wsg84> point_w(SRID::WSG84, SpatialType::POINT, 0,
        "0101000000000000000000F03F0000000000000040");
        // 输出全部为大写字母
        UT_EXPECT_EQ(point_w.AsEWKB(), "0101000020E6100000000000000000F03F0000000000000040");
        UT_EXPECT_EQ(point_w.AsEWKT(), "SRID=4326;POINT(1 2)");

        // testing wrong wkb format;
        UT_EXPECT_ANY_THROW(Spatial<Wsg84>(SRID::WSG84, SpatialType::POINT, 0, "1111111"));


        Spatial<Cartesian> point_c(SRID::CARTESIAN, SpatialType::POINT, 1, "POINT(1.0 1.0)");
        UT_EXPECT_EQ(point_c.AsEWKB(), "0101000020231C0000000000000000F03F000000000000F03F");
        UT_EXPECT_EQ(point_c.AsEWKT(), "SRID=7203;POINT(1 1)");

        // testing wrong wkt format;
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::POINT, 1,
        "POINT(1.0 2.0 3.0)"));
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::POINT, 1, "POINT(a)"));

        Spatial<Wsg84> line_w(SRID::WSG84, SpatialType::LINESTRING, 0, "01020000000300000000000000"
        "000000000000000000000000000000000000004000000000000000400000000000000840000000000000F03F");

        UT_EXPECT_EQ(line_w.AsEWKB(), "0102000020E610000003000000000000000000000000000000000000000"
        "00000000000004000000000000000400000000000000840000000000000F03F");

        UT_EXPECT_EQ(line_w.AsEWKT(), "SRID=4326;LINESTRING(0 0,2 2,3 1)");

        UT_EXPECT_ANY_THROW(Spatial<Wsg84>(SRID::WSG84, SpatialType::LINESTRING, 0, "1111111"));

        Spatial<Cartesian> line_c(SRID::CARTESIAN, SpatialType::LINESTRING, 1,
        "LINESTRING(0 0,2 2,3 1)");
        UT_EXPECT_EQ(line_c.AsEWKB(), "0102000020231C00000300000000000000000000000000000000000"
        "000000000000000004000000000000000400000000000000840000000000000F03F");
        UT_EXPECT_EQ(line_c.AsEWKT(), "SRID=7203;LINESTRING(0 0,2 2,3 1)");

        UT_EXPECT_ANY_THROW(Spatial<Wsg84>(SRID::WSG84, SpatialType::LINESTRING, 1,
        "LINESTRING(1.0 2.0 a)"));
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::LINESTRING, 1,
        "LINE(0 0,2 2,3 1)"));

        Spatial<Wsg84> polygon_w(SRID::WSG84, SpatialType::POLYGON, 0,
        "0103000000010000000500000000000000000000000000000000000000000"
        "00000000000000000000000001C4000000000000010400000000000000040"
        "0000000000000040000000000000000000000000000000000000000000000000");

        UT_EXPECT_EQ(polygon_w.AsEWKB(), "0103000020E6100000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
        "00000000040000000000000000000000000000000000000000000000000");

        UT_EXPECT_EQ(polygon_w.AsEWKT(), "SRID=4326;POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_ANY_THROW(Spatial<Wsg84>(SRID::WSG84, SpatialType::POLYGON, 1, "abcde"));

        Spatial<Cartesian> polygon_c(SRID::CARTESIAN, SpatialType::POLYGON, 1,
        "POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_EQ(polygon_c.AsEWKB(), "0103000020231C0000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
        "00000000040000000000000000000000000000000000000000000000000");

        UT_EXPECT_EQ(polygon_c.AsEWKT(), "SRID=7203;POLYGON((0 0,0 7,4 2,2 0,0 0))");
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::WSG84, SpatialType::POLYGON, 1,
        "POLYGON(122)"));
        UT_EXPECT_ANY_THROW(Spatial<Cartesian>(SRID::CARTESIAN, SpatialType::POLYGON, 1,
        "POL((0 0,0 7,4 2,2 0,0 0))"));
    }

    {
        UT_LOG() << "Testing construct from EWKB";

        std::string EWKB1 = "0101000020E6100000000000000000F03F000000000000F03F";
        Spatial<Wsg84> p(EWKB1);
        UT_EXPECT_EQ(p.AsEWKB(), EWKB1);
        UT_EXPECT_EQ(p.AsEWKT(), "SRID=4326;POINT(1 1)");

        std::string EWKB2 = "0102000020231C00000300000000000000000000000000000000000000"
        "000000000000004000000000000000400000000000000840000000000000F03F";

        Spatial<Cartesian> l(EWKB2);
        UT_EXPECT_EQ(l.AsEWKB(), EWKB2);
        UT_EXPECT_EQ(l.AsEWKT(), "SRID=7203;LINESTRING(0 0,2 2,3 1)");

        std::string EWKB3 = "0103000020E610000001000000050000000000000000000000"
        "00000000000000000000000000000000000000000000F03F000000000000F03F000000000000F03F"
        "000000000000F03F000000000000000000000000000000000000000000000000";

        Spatial<Wsg84> p_(EWKB3);
        UT_EXPECT_EQ(p_.AsEWKB(), EWKB3);
        UT_EXPECT_EQ(p_.AsEWKT(), "SRID=4326;POLYGON((0 0,0 1,1 1,1 0,0 0))");
    }

    {
        UT_LOG() << "Testing point constructor";
        std::string point_EWKB = "0101000020E6100000000000000000F03F000000000000F03F";
        point<Wsg84> p(point_EWKB);
        UT_EXPECT_EQ(p.AsEWKB(), point_EWKB);
        UT_EXPECT_EQ(p.AsEWKT(), "SRID=4326;POINT(1 1)");
    }
}

TEST_P(TestSpatial, Spatial_Schema) {
    using namespace lgraph;

    UT_LOG() << "Test Schema::AddspatialFields, DelspatialFields and ModspatialFields";
    Schema s1;
    s1.SetSchema(
        true,
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false),
             FieldSpec("point", FieldType::POINT, false),
             FieldSpec("linestring", FieldType::LINESTRING, true),
             FieldSpec("polygon", FieldType::POLYGON, true),
             FieldSpec("string2point", FieldType::STRING, true),
             FieldSpec("string2line", FieldType::STRING, false),
             FieldSpec("string2polygon", FieldType::STRING, true)}),
        "id", {});
    std::map<std::string, FieldSpec> fields = s1.GetFieldSpecsAsMap();
    // add fields;
    {
        Schema s2(s1);
        s2.AddFields(std::vector<FieldSpec>({FieldSpec("point2", FieldType::POINT, false)}));
        UT_EXPECT_TRUE(s2.GetFieldExtractor("point2")->GetFieldSpec() ==
                       FieldSpec("point2", FieldType::POINT, false));
        auto fmap = s2.GetFieldSpecsAsMap();
        UT_EXPECT_EQ(fmap.size(), fields.size() + 1);
        fmap.erase("point2");
        UT_EXPECT_TRUE(fmap == fields);
    }

    // mod fields;
    {
        Schema s2(s1);
        std::vector<FieldSpec> mod = {FieldSpec("string2point", FieldType::POINT, false),
                                      FieldSpec("string2line", FieldType::LINESTRING, false),
                                      FieldSpec("string2polygon", FieldType::POLYGON, false)};
        s2.ModFields(mod);
        UT_EXPECT_TRUE(!s2.HasBlob());
        auto fmap = s2.GetFieldSpecsAsMap();
        auto old_fields = fields;
        for (auto& f : mod) old_fields[f.name] = f;
        UT_EXPECT_TRUE(fmap == old_fields);
        UT_EXPECT_THROW(s2.ModFields(std::vector<FieldSpec>(
                            {FieldSpec("no_such_field", FieldType::BLOB, true)})),
                        FieldNotFoundException);
    }

    // del fields;
    {
        Schema s2(s1);
        std::vector<std::string> to_del = {"point", "linestring", "polygon"};
        s2.DelFields(to_del);
        UT_EXPECT_TRUE(!s2.HasBlob());
        auto fmap = s2.GetFieldSpecsAsMap();
        auto old_fields = fields;
        for (auto& f : to_del) old_fields.erase(f);
        UT_EXPECT_TRUE(fmap == old_fields);
    }

    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);
    DBConfig conf;
    conf.dir = dir;
    CreateSampleDB(dir, GetParam());
    LightningGraph graph(conf);
    UT_LOG() << "Testing set spatial schema and add vertex";
    {
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("point")) == FieldData::Point
        ("0101000020E6100000000000000000F03F0000000000000040"));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("linestring")) == FieldData::LineString
        ("0102000020E610000003000000000000000000000000000000000000000"
        "00000000000004000000000000000400000000000000840000000000000F03F"));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("polygon")) == FieldData::Polygon
        ("0103000020231C0000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
        "00000000040000000000000000000000000000000000000000000000000"));
    }

    UT_LOG() << "testing add";
    {
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelAddFields(
            "spatial",
            std::vector<FieldSpec>({FieldSpec("point2", FieldType::POINT, true),
                                    FieldSpec("linestring2", FieldType::LINESTRING, true),
                                    FieldSpec("polygon2", FieldType::POLYGON, true)}),
            std::vector<FieldData>({FieldData::Point
            ("0101000020231C0000000000000000F03F0000000000000040"),
            FieldData(), FieldData()}), true, &n_changed));
        UT_EXPECT_EQ(n_changed, 2);
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("point2")) == FieldData::Point
        ("0101000020231C0000000000000000F03F0000000000000040"));
    }

    UT_LOG() << "Testing modify";
    {
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelModFields(
            "spatial",
            std::vector<FieldSpec>({FieldSpec("string2point", FieldType::POINT, false),
                                    FieldSpec("string2line", FieldType::LINESTRING, false)}),
            true, &n_changed));
        UT_EXPECT_EQ(n_changed, 2);
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("string2point")) == FieldData::Point
        ("0101000020E6100000000000000000F03F0000000000000040"));
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("string2line")) == FieldData::LineString
        ("0102000020E610000003000000000000000000000000000000000000000"
        "00000000000004000000000000000400000000000000840000000000000F03F"));
    }

    UT_LOG() << "Testing Del";
    {
        size_t n_changed = 0;
        UT_EXPECT_TRUE(graph.AlterLabelDelFields("spatial", std::vector<std::string>
                       ({"point", "linestring", "polygon"}), true, &n_changed));
        UT_EXPECT_EQ(n_changed, 2);
        auto txn = graph.CreateReadTxn();
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("point")) == FieldData());
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("linestring")) == FieldData());
        UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("polygon")) == FieldData());
    }
}

TEST_P(TestSpatial, error_handle) {
    using namespace lgraph;
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);
    DBConfig conf;
    conf.dir = dir;
    TestSampleDB(dir, GetParam());
}