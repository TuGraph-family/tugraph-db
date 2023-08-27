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
        "person",
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false), FieldSpec("name", FieldType::STRING, false),
             FieldSpec("age", FieldType::FLOAT, true), FieldSpec("img", FieldType::BLOB, true),
             FieldSpec("desc", FieldType::STRING, true), FieldSpec("point", FieldType::POINT, true)}),
        true, vo));
    
    UT_EXPECT_TRUE(lg.AddLabel(
        "spatial",
        std::vector<FieldSpec>(
            {FieldSpec("id", FieldType::INT32, false), FieldSpec("point", FieldType::POINT, false),
             FieldSpec("linestring", FieldType::LINESTRING, true), FieldSpec("polygon", FieldType::POLYGON, true)}),
        true, vo));
        
    auto txn = lg.CreateWriteTxn();
        txn.AddVertex(std::string("person"),
                      std::vector<std::string>({"id", "name", "age", "img", "desc", "point"}),
                      std::vector<std::string>({"1", "p1", "11.5", "", "desc for p1",
                                                "0101000020E6100000000000000000F03F0000000000000040"}));
        txn.AddVertex(
        std::string("person"),
        std::vector<std::string>({"id", "name", "age", "img", "desc", "point"}),
        std::vector<std::string>(
            {"2", "p2", "12", lgraph_api::base64::Encode(std::string(8192, 'a')),
             std::string(4096, 'b'), "0101000020231C0000000000000000F03F000000000000F03F"}));
        
        txn.AddVertex(
        std::string("spatial"),
        std::vector<std::string>({"id", "point", "linestring", "polygon"}),
        std::vector<std::string>({"3", 
        "0101000020E6100000000000000000F03F0000000000000040", 
        "0102000020E610000003000000000000000000000000000000000000000"
        "00000000000004000000000000000400000000000000840000000000000F03F", 
        "0103000020231C0000010000000500000000000000000000000"
        "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
        "00000000040000000000000000000000000000000000000000000000000"})
        );


    txn.Commit();
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
    std::string dir = "./testdb";
    AutoCleanDir cleaner(dir);
    DBConfig conf;
    conf.dir = dir;

    UT_LOG() << "Testing spatial schema";
    CreateSampleDB(dir, GetParam());
    LightningGraph graph(conf);

    auto txn = graph.CreateReadTxn();
    UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("age")) == FieldData::Float(11.5));
    UT_EXPECT_TRUE(txn.GetVertexField(0, std::string("point")) == FieldData::Point(
        "0101000020E6100000000000000000F03F0000000000000040"));
    UT_EXPECT_TRUE(txn.GetVertexField(1, std::string("point")) == FieldData::Point(
        "0101000020231C0000000000000000F03F000000000000F03F") );
    UT_EXPECT_TRUE(txn.GetVertexField(2, std::string("linestring")) == FieldData::LineString
    ("0102000020E610000003000000000000000000000000000000000000000"
    "00000000000004000000000000000400000000000000840000000000000F03F"));
    UT_EXPECT_TRUE(txn.GetVertexField(2, std::string("polygon")) == FieldData::Polygon
    ("0103000020231C0000010000000500000000000000000000000"
     "00000000000000000000000000000000000000000001C400000000000001040000000000000004000000"
     "00000000040000000000000000000000000000000000000000000000000"));

}