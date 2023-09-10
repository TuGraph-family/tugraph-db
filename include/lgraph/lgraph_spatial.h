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

#include <boost/geometry/io/wkt/wkt.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>

namespace lgraph_api {

namespace bg = boost::geometry;

typedef bg::cs::cartesian Cartesian;
typedef bg::cs::geographic<bg::degree> Wsg84;
typedef std::vector<boost::uint8_t> byte_vector;

/**         
 * @brief   now support three types of spatial data, they are all two-dimensional;
 *          for more detial, you can refer to
 *          https://www.boost.org/doc/libs/1_68_0/libs/geometry/doc/html/geometry/reference/models.html 
 */
enum class SpatialType {
    NUL = 0,
    POINT = 1,        // point type, e.g. (1.0 2.0)
    LINESTRING = 2,   // linestring type, represent a linestring which is
                      // composed of different points. e.g (1.0 2.0, 3.0 2.0, 5.0 4.0)
    POLYGON = 3       // polygon type,
                      // A polygon of Boost.Geometry is a polygon with or without holes
                      // e.g (1.0 2.0, 3.0 2.0, 5.0 4.0, 1.0 2.0) for more detail, you can refer to
                      // https://www.boost.org/doc/libs/1_68_0/libs/geometry/doc/html/geometry/
                      // reference/concepts/concept_polygon.html
};

/**
 * @brief   the full name of srid is Spatial Reference System Identifier, it's the reference system for
 *          spatial data, we now support two types of SRID. All spatial data in tugraph should have a 
 *          srid and the default srid is 4326. 
 *          这里暂时没有找到比较官方的参考资料
 */
enum class SRID {
    NUL = 0,
    WSG84 = 4326,      // it's the latest revision of the World Geodetic System standard
                       // (from 1984 CE), which defines a standard spheroidal reference system
                       // for mapping the Earth.
    CARTESIAN = 7203   // the cartesian srid, which is often used in Planar geometry.
};

template<typename SRID_Type>
class point;

template<typename SRID_Type>
class linestring;

template<typename SRID_Type>
class polygon;

// true if little endian, false if big endian;
bool Endian(const std::string& EWKB);

/**
* @brief transfer the given hex format string between little and big endian;
*        the hex data is 4 bit each, so the size 2 hex data is 1 byte. We need 
*        to reverse every byte between the two endian. 
*
* @param [in,out] input little/big endian hex format string;
*/
void EndianTansfer(std::string& input);

/**
* @brief  transfer the srid_type into hex format;
* 
* @param srid_type      Spatial Reference System Identifier;
* @param width          the length of hex format to transform;   
* @param ebdian         true little endian, false big endian;
* 
* @returns the hex format srid_type;
*/
std::string Srid2Hex(SRID srid_type, size_t width, bool endian);

/**
* @brief  set the wkb format string to EWKB format string;
*         关于EWKB与WKB layout 见TryDecodeEWKB与TryDecodeWKB注释;
*         EWKB -> WKB   以little-endian为例:
*         WKB[8] = 2;   在WKB[10]之后插入SRID信息, E6100000(4326) 共8位
*              
* @param WKB          the wkb format to be extended   
* @param srid_type    the srid type to be added in the format
* 
* @returns the hex format srid_type;
*/
std::string SetExtension(const std::string& WKB, SRID srid_type);

// extract the srid from EWKB format;
SRID ExtractSRID(const std::string& EWKB);

// extract the spatial type from EWKB format;
SpatialType ExtractType(const std::string& EWKB);

/**
* @brief  the WKB layout of spatial data:
*          以point为例: 01  0100  0000  000000000000F03F  0000000000000040 Point(1.0 2.0) 
*          01:   编码方式, 00表示big-endian, 01表示little-endian
*          0100: 数据类型, 0100代表Point;
*          0000: 
*          每16个字节代表一个坐标对。
*          这里主要利用boost/geometry 中的read_wkb检验WKB格式是否正确。
*
* @param WKB           the string to be parse;
* @param type          the type of spatial data;
* 
* @returns true if WKB format is valid, false if wkb format is invalid;
*/
template<typename SRID_Type>
bool TryDecodeWKB(const std::string& WKB, SpatialType type);

/**
* @brief  the EWKB layout of spatial data:
*          以point为例: 01 0100 0020 E6100000 000000000000F03F 0000000000000040 Point(1.0 2.0)
*          01:           编码方式, 00表示big-endian, 01表示little-endian
*          0100:         数据类型, 0100代表Point;
*          0020:         表示维度为二维;(?)
*          E6010000:     SRID -- 4326；  0x000010e6的小端表示;
*          每16个字节代表一个坐标对。
*          这里首先将EWKB格式转换为WKB格式,再调用TryDecodeWKB;
*          EWKB -> WKB  WKB -> EWKB的逆变换, 修改EWKB[8] or EWKB[9], 去除SRID信息;(EWKB[10] - EWKB[17])
*
* @param EWKB          the string to be parse;
* @param type          the type of spatial data;
* 
* @returns true if EWKB format is valid, false if wkb format is invalid;
*/
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
    bg::model::point<double, 2, SRID_Type> GetSpatialData() {
        return point_;
    }

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
