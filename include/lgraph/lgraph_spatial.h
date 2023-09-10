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
 * 
 * TODO:
 *   1. 进一步通过测试样例检查ParseStringToValueOfFieldType与TryFieldDataToValueOfFieldType
 *      实现的正确性(空间数据与string数据的转换);
 *   2. FieldType2CType无法实现空间数据相关功能, 目前绕开实现。
 *   3. 确认line与polygon调用_SetVariableLengthValue实现正确。
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
#include <boost/geometry.hpp>
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
bool Endian(const std::string& EWKB);

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
SRID ExtractSRID(const std::string& EWKB);

// extract the spatial type from EWKB format;
SpatialType ExtractType(const std::string& EWKB);

// return whether a WKB format is valid;
template<typename SRID_Type>
bool TryDecodeWKB(const std::string& EWKB, SpatialType type);

// return whether a EWKB format is valid;
bool TryDecodeEWKB(const std::string& EWKB, SpatialType type);

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
    explicit Spatial(const std::string& EWKB);

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

    virtual std::string AsEWKB() const = 0;

    virtual std::string AsEWKT() const = 0;

    // virtual std::string ToString() const = 0;

    SpatialType GetType() const {
        return type_;
    }

    SRID GetSrid() const {
        return srid_;
    }
};

/**
 * @brief implements a point spatial class which spatial data is point; 
 */
template<typename SRID_Type>
class point : public SpatialBase {
    std::string EWKB;
    bg::model::point<double, 2, SRID_Type> point_;

 public:
    point(SRID srid, SpatialType type, int construct_type, std::string content);

    point(double arg1, double arg2, SRID& srid);

    explicit point(const std::string& EWKB);


    std::string AsEWKB() const override {
        return EWKB;
    }

    std::string AsEWKT() const override;

    /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;
    /**
     *  @brief return point type data;
    */
    bg::model::point<double, 2, SRID_Type> GetSpatialData();

    double Distance(point<SRID_Type>& other);

    bool operator==(const point<SRID_Type>& other);
};

/**
 * @brief implements a linestring spatial class which spatial data is point; 
 */
template<typename SRID_Type>
class linestring : public SpatialBase {
    std::string EWKB;
    typedef bg::model::point<double, 2, SRID_Type> point_;
    bg::model::linestring<point_> line_;

 public:
    linestring(SRID srid, SpatialType type, int construct_type, std::string content);

    explicit linestring(const std::string& EWKB);

    std::string AsEWKB() const override {
        return EWKB;
    }

    std::string AsEWKT() const override;

     /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;

    bg::model::linestring<bg::model::point<double, 2, SRID_Type>> GetSpatialData() {
        return line_;
    }

    bool operator==(const linestring<SRID_Type>& other);
};

/**
 * @brief implements a polygon spatial class which spatial data is point; 
 */
template<typename SRID_Type>
class polygon : public SpatialBase {
    std::string EWKB;
    typedef bg::model::point<double, 2, SRID_Type> point;
    bg::model::polygon<point> polygon_;

 public:
    polygon(SRID srid, SpatialType type, int construct_type, std::string content);

    explicit polygon(const std::string& EWKB);

    std::string AsEWKB() const override {
        return EWKB;
    }

    std::string AsEWKT() const override;

     /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;

    bg::model::polygon<bg::model::point<double, 2, SRID_Type>> GetSpatialData() {
        return polygon_;
    }

    bool operator==(const polygon<SRID_Type>& other);
};
}  //  namespace lgraph_api
