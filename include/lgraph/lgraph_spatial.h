//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/**
 * @file lgraph_spatial.h
 * @brief Implemnets the Spatial, SpatialBase and SpatialDerive classes.
 */

#pragma once

#include <string>
#include <vector>
#include <iostream>

#include <boost/geometry/algorithms/equals.hpp>
#include <boost/geometry/geometries/geometries.hpp>
#include <boost/geometry/io/wkt/read.hpp>
#include <boost/geometry/extensions/gis/io/wkb/write_wkb.hpp>
#include <boost/geometry/extensions/gis/io/wkb/utility.hpp>

#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>

namespace lgraph_api {

namespace bg = boost::geometry;

typedef bg::cs::cartesian Cartesian;
typedef bg::cs::geographic<bg::degree> Wsg84;
typedef std::vector<boost::uint8_t> byte_vector;

enum class SpatialType {
    NUL = 0,
    POINT = 1,
    LINESTRING = 2,
    POLYGON = 3
};

enum class SRID {
    NUL = 0,
    WSG84 = 4326,
    CARTESIAN = 7203
};

template<typename SRID_Type>
class point;

template<typename SRID_Type>
class linestring;

template<typename SRID_Type>
class polygon;

// true if little endian, false if big endian;
bool Endian(std::string EWKB);

// transfer endian between little and big;
void EndianTansfer(std::string& input);

/**
* @brief  transform the srid_type into hex format;
* 
* @param srid_type      Spatial Reference System Identifier;
* @param width          the length of hex format to transform;   
* @param ebdian         true little endian, false big endian;
* 
* @returns the hex format srid_type;
*/
std::string srid2hex(SRID srid_type, size_t width, bool endian);

/**
* @brief  set the wkb format string to EWKB format string;
* 
* @param content        
* @param width          the length of hex format to transform;   
* @param ebdian         true little endian, false big endian;
* 
* @returns the hex format srid_type;
*/
std::string set_extension(const std::string& WKB, SRID srid_type);

// extract the srid from EWKB format;
SRID ExtractSRID(std::string EWKB);

// extract the spatial type from EWKB format;
SpatialType ExtractType(std::string EWKB);

/** @brief   Implements the Spatial template class. Spatial class now can hold one of
 *  point, linestring or polygon pointer;
 */
template<typename SRID_Type>
class Spatial {
    std::unique_ptr<point<SRID_Type>> point_;
    std::unique_ptr<linestring<SRID_Type>> line_;
    std::unique_ptr<polygon<SRID_Type>> polygon_;

    SpatialType type_;

 public:
    /**
     * @brief  construct spatial type by four parameters;
     * 
     * @exception   runtime error thrown if the Spatial Type is NUL;
     * 
     * @param srid           Spatial Reference System Identifier
     * @param type           a specific Spatial Type 
     * @param construct_type 0: construct from WKB type, 1: construct from wkt type;
     * @param content        WKB format or WKT format;
     */

    Spatial(SRID srid, SpatialType type, int construct_type, std::string content);

    /**
     * @brief  construct spatial type from EWKB format;
     * 
     * @exception   runtime error thrown if the Spatial Type is NUL;
     * 
     * @param EWKB  the EWKB format string;
     */
    explicit Spatial(std::string EWKB);

    ~Spatial() {}

    /**
     * @brief   get EWKB format string of spatial type;
     *
     * @returns EWKB format string of spatial type(in capital);
     */
    std::string AsEWKB() const;

    /**
     * @brief   get EWKT format string of spatial type;
     *
     * @returns EWKT format string of spatial type(in capital);
     */
    std::string AsEWKT() const;

    /** @brief  return the point type pointer */
    std::unique_ptr<point<SRID_Type>>& GetPoint() {
        return point_;
    }

    /** @brief return the line type pointer */
    std::unique_ptr<linestring<SRID_Type>>& GetLine() {
        return line_;
    }

    /** @brief return the polygon type pointer */
    std::unique_ptr<polygon<SRID_Type>>& GetPolygon() {
        return polygon_;
    }

    /** @brief return the type of spatial*/
    SpatialType GetType() {
        return type_;
    }

    bool operator==(const Spatial<SRID_Type>& other);

    // double Distance(geography<T>& other);
};

/**
 * @brief   Implements a SpatialBase class which specific spatial type class 
 *          derive from;            
 */
class SpatialBase {
    SRID srid_;
    SpatialType type_;

 public:
    /**
     * @brief the constructor function of spatial base;
     */
    SpatialBase(SRID srid, SpatialType type) : srid_(srid), type_(type) {}

    // virtual ~SpatialBase();

    virtual std::string AsEWKB() = 0;

    virtual std::string AsEWKT() = 0;

    SpatialType GetType() {
        return type_;
    }

    SRID GetSrid() {
        return srid_;
    }
};

/**
 * @brief implements a point spatial class which spatial data is point; 
 */
template<typename SRID_Type>
class point : public SpatialBase {
    bg::model::point<double, 2, SRID_Type> point_;
    std::string EWKB;

 public:
    point(SRID srid, SpatialType type, int construct_type, std::string content);

    std::string AsEWKB() override {
        return EWKB;
    }

    std::string AsEWKT() override;
    /**
     *  @brief return point type data;
    */
    bg::model::point<double, 2, SRID_Type> GetSpatialData() {
        return point_;
    }
};

/**
 * @brief implements a linestring spatial class which spatial data is point; 
 */
template<typename SRID_Type>
class linestring : public SpatialBase {
    typedef bg::model::point<double, 2, SRID_Type> point_;
    bg::model::linestring<point_> line_;
    std::string EWKB;

 public:
    linestring(SRID srid, SpatialType type, int construct_type, std::string content);

    std::string AsEWKB() override {
        return EWKB;
    }

    std::string AsEWKT() override;

    bg::model::linestring<bg::model::point<double, 2, SRID_Type>> GetSpatialData() {
        return line_;
    }
};

/**
 * @brief implements a polygon spatial class which spatial data is point; 
 */
template<typename SRID_Type>
class polygon : public SpatialBase {
    typedef bg::model::point<double, 2, SRID_Type> point;
    bg::model::polygon<point> polygon_;
    std::string EWKB;

 public:
    polygon(SRID srid, SpatialType type, int construct_type, std::string content);

    std::string AsEWKB() override {
        return EWKB;
    }

    std::string AsEWKT() override;

    bg::model::polygon<bg::model::point<double, 2, SRID_Type>> GetSpatialData() {
        return polygon_;
    }
};
}  //  namespace lgraph_api
