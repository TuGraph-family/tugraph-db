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
 *   3. 确认line与Polygon调用_SetVariableLengthValue实现正确。
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
typedef bg::cs::geographic<bg::degree> Wgs84;
typedef std::vector<boost::uint8_t> byte_vector;

/**         
 * @brief   now support three types of spatial data, they are all two-dimensional;
 *          for more detial, you can refer to
 *          https://www.boost.org/doc/libs/1_68_0/libs/geometry/doc/html/geometry/reference/models.html 
 */
enum class SpatialType {
    NUL = 0,
    POINT = 1,        // Point type, e.g. (1.0 2.0)
    LINESTRING = 2,   // LineString type, represent a LineString which is
                      // composed of different Points. e.g (1.0 2.0, 3.0 2.0, 5.0 4.0)
    POLYGON = 3       // Polygon type,
                      // A Polygon of Boost.Geometry is a Polygon with or without holes
                      // e.g (1.0 2.0, 3.0 2.0, 5.0 4.0, 1.0 2.0) for more detail, you can refer to
                      // https://www.boost.org/doc/libs/1_68_0/libs/geometry/doc/html/geometry/
                      // reference/concepts/concept_Polygon.html
};

/**
 * @brief   the full name of srid is Spatial Reference System Identifier, it's the reference system for
 *          spatial data, we now support two types of SRID. All spatial data in tugraph should have a 
 *          srid and the default srid is 4326. 
 *          这里暂时没有找到比较官方的参考资料
 */
enum class SRID {
    NUL = 0,
    WGS84 = 4326,      // it's the latest revision of the World Geodetic System standard
                       // (from 1984 CE), which defines a standard spheroidal reference system
                       // for mapping the Earth.
    CARTESIAN = 7203   // the cartesian srid, which is often used in Planar geometry.
};

template<typename SRID_Type>
class Point;

template<typename SRID_Type>
class LineString;

template<typename SRID_Type>
class Polygon;

/**
* @brief       return the given ewkb/wkb format data is little or big endian
*              (determined by the first byte)
*
* @exception   InputError  Thrown if the input size is less than 42(the minimum 
*                           wkb format length)
*
* @param       EWKB expected wkb/ewkb format string;
*
* @returns     true if little endian, false if big endian;
*/
bool Endian(const std::string& EWKB);

/**
* @brief transfer the given hex format string between little and big endian;
*        the hex data is 4 bit each, so every 2 hex data is 1 byte. We need 
*        to reverse each byte from the given input.
*
* @exception   InputError  Thrown if the input size is less than 42(the minimum 
*                          wkb format length)
*
* @param [in,out] input little/big endian hex format string;
*/
void EndianTansfer(std::string& input);

/**
* @brief  transfer the srid_type(enum class) into hex format(little endian in default);
* 
* @param srid_type      Spatial Reference System Identifier;
*
* @param width          the length of hex format to transform;   
* 
* @exception   InputError Thrown if the input width is less than 4;
*
* @returns the hex format srid_type in little endian;
*/
std::string Srid2Hex(SRID srid_type, size_t width);

/**
* @brief transfer the wkb format data between big and little endian;
*        first, transfer the first byte. Then transfer the next 4 bytes,
*        for LineString and Polygon, we need to transfer extra bytes(which 
*        represents the number of Points and rings).        
*        then transfer the next n * 8 bytes. you can refer to the wkb layout
*        from the conments of TryDecodeWKB;
*
* @exception   InputError  Thrown if the input size is less than 42(the minimum 
*                          wkb format length)
*
* @param [in, out] WKB  the wkb format data in big/little endian, out int little/big endian;
*/
void WkbEndianTransfer(std::string& WKB);

/**
* @brief transfer the EWKB format data from between big and little endian;
*        first, transfer the first byte. Then transfer the next 2 bytes(type) and
*        the following 2 bytes(dimension). Then transfer the next 4 bytes(srid).
*        for LineString and Polygon, we need to transfer extra bytes(which 
*        represents the number of Points and rings). Then transfer the next n * 8 bytes.
*        you can refer to the EWKB layout from the comments of TryDecodeEWKB. 
*       
* @exception   InputError  Thrown if the input size is less than 50(the minimum 
*                          ewkb format length)
*
* @param EWKB  the EWKB format data in big/little endian;
*
* @returns the EWKB format data in little/big endian;
*/
std::string EwkbEndianTransfer(const std::string& EWKB);

/**
* @brief  extend the wkb format string to EWKB format string;
*         you can find the difference between WKB and EWKB format
*         at the comments of TryDecodeWKB and TryDecodeEWKB
*              
* @param WKB          the wkb format to be extended   
*
* @param srid_type    the srid type to be added in the format
* 
* @returns the EWKB foramt data extended from the input wkb format data;
*/
std::string SetExtension(const std::string& WKB, SRID srid_type);

// extract the srid from EWKB format;
SRID ExtractSRID(const std::string& EWKB);

// extract the spatial type from EWKB format;
SpatialType ExtractType(const std::string& EWKB);

/**
* @brief  the WKB layout of spatial data(little endian):
*          Point: 01  01000000  000000000000F03F  0000000000000040 Point(1.0 2.0) 
*          01:       1byte  编码方式, 00表示big-endian, 01表示little-endian
*          01000000: 4bytes 数据类型, 00000001的little endian, 表示Point. 
*                    02 LineString 03 Polygon 
*          接下来每4个bytes代表一个数据, 每8个bytes代表一个Point坐标。
*           
*          LineString: 01 02000000 03000000 0000000000000000 0000000000000000
*          0000000000000040 0000000000000040 0000000000000840 000000000000F03F
*          LineString(0 0,2 2,3 1)
*          01:       1byte  编码方式
*          02000000: 4bytes 数据类型  表示LineString
*          03000000: 4bytes 表示在LineString中有三个Point;
*          接下来每8个bytes表示一个Point;
*
*          Polygon: 01 03000000 01000000 05000000 0000000000000000 000000000
*          0000000 0000000000000000 0000000000001C40 0000000000001040 0000000000000040
*          0000000000000040 0000000000000000 0000000000000000 0000000000000000
*          Polygon((0 0,0 1,1 1,1 0,0 0))
*          01:       1byte  编码方式
*          03000000: 4bytes 数据类型  表示LineString
*          01000000: 4bytes 表示在Polygon中有一个ring;
*          05000000: 4bytes 
*          接下来每8个bytes表示一个Point;             
*          
*          big endian表示:
*          将上述wkb format以空格隔开的每个部分都由little endian转为big endian;
*
*          这里主要利用boost/geometry/extensions 中的read_wkb检验WKB格式是否正确。
*
* @param WKB           the string to be parse;
* @param type          the type of spatial data;
* 
* @returns true if WKB format is valid, false if wkb format is invalid;
*/
template<typename SRID_Type>
bool TryDecodeWKB(const std::string& WKB, SpatialType type);

/**
* @brief   the EWKB layout of spatial data:
*          Point: 01 0100 0020 E6100000 000000000000F03F 0000000000000040 
*          SRID=4326 Point(1.0 2.0)
*          01:           编码方式, 00表示big-endian, 01表示little-endian
*          0100:         数据类型, 0100代表Point; (相比于WKB, 由4bytes表示变为2bytes)
*          0020:         表示维度为二维;
*          E6010000:     4bytes SRID -- 4326；  0x000010e6的小端表示; 
*          每8个byte代表一个Point。
*          
*          LineString: 01 0200 0020 E6100000 03000000 0000000000000000 0000000000000000
*          0000000000000040 0000000000000040 0000000000000840 000000000000F03F
*          SRID=4326 LineString(0 0,2 2,3 1)
*          01:   1byte  编码方式
*          0200: 2bytes 数据类型  表示LineString
*          0020: 2bytes dimension
*          E6010000: 4bytes SRID -- 4326；  0x000010e6的小端表示;
*          03000000: 4bytes 表示在LineString中有三个Point;
*          接下来每8个bytes表示一个Point;
*          
*          Polygon: 类似;
*          01 0300 0020 e6100000 01000000 05000000 0000000000000000 0000000000000000
*          0000000000000000 000000000000f03f 000000000000f03f 000000000000f03f 000000000000f03f
*          0000000000000000 0000000000000000 0000000000000000
*          SRID=4326;Polygon((0 0,0 1,1 1,1 0,0 0)) 
*
* @param EWKB          the string to be parse;
* @param type          the type of spatial data;
* 
* @returns true if WKB format is valid, false if WKB format is invalid;
*/
bool TryDecodeEWKB(const std::string& EWKB, SpatialType type);

/** @brief   Implements the Spatial template class. Spatial class now can hold one of
 *  Point, LineString or Polygon Pointer;
 */
template<typename SRID_Type>
class Spatial {
    std::unique_ptr<Point<SRID_Type>> Point_;
    std::unique_ptr<LineString<SRID_Type>> line_;
    std::unique_ptr<Polygon<SRID_Type>> Polygon_;

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

    /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;

    /** @brief  return the Point type Pointer */
    std::unique_ptr<Point<SRID_Type>>& GetPoint() {
        return Point_;
    }

    /** @brief return the line type Pointer */
    std::unique_ptr<LineString<SRID_Type>>& GetLine() {
        return line_;
    }

    /** @brief return the Polygon type Pointer */
    std::unique_ptr<Polygon<SRID_Type>>& GetPolygon() {
        return Polygon_;
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
 * @brief implements a Point spatial class which spatial data is Point; 
 */
template<typename SRID_Type>
class Point : public SpatialBase {
    std::string EWKB;
    bg::model::point<double, 2, SRID_Type> Point_;

 public:
    /**
    *  @brief construct Point class from wkb/wkt format;
    * 
    *  @param srid the srid of the Point;
    *  
    *  @param type type of the spatial data(Point)
    * 
    *  @param construct_type 0: WKB  1: wkt;
    * 
    *  @param content  the wkb/wkt format data;
    * 
    *  @exception InputError  Thrown if the WKB/WKT format is invalid or the input
    *                         srid dismatch the template srid;
    */
    Point(SRID srid, SpatialType type, int construct_type, std::string& content);

    /**
     *  @brief construct Point class from EWKB format;
     * 
     *  @param EWKB the EWKB format data
     * 
     *  @param InputError Thrown if the EWKB format is invalid or the input
     *                    srid dismatch the template srid;
    */
    explicit Point(const std::string& EWKB);

    std::string AsEWKB() const override {
        return EWKB;
    }

    std::string AsEWKT() const override;

    /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;
    /**
     *  @brief return Point type data;
    */
    bg::model::point<double, 2, SRID_Type> GetSpatialData() {
        return Point_;
    }

    bool operator==(const Point<SRID_Type>& other);
};

/**
 * @brief implements a LineString spatial class which spatial data is Point; 
 */
template<typename SRID_Type>
class LineString : public SpatialBase {
    std::string EWKB;
    typedef bg::model::point<double, 2, SRID_Type> Point_;
    bg::model::linestring<Point_> line_;

 public:
    LineString(SRID srid, SpatialType type, int construct_type, std::string& content);

    explicit LineString(const std::string& EWKB);

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

    bool operator==(const LineString<SRID_Type>& other);
};

/**
 * @brief implements a Polygon spatial class which spatial data is Point; 
 */
template<typename SRID_Type>
class Polygon : public SpatialBase {
    std::string EWKB;
    typedef bg::model::point<double, 2, SRID_Type> Point;
    bg::model::polygon<Point> Polygon_;

 public:
    Polygon(SRID srid, SpatialType type, int construct_type, std::string& content);

    explicit Polygon(const std::string& EWKB);

    std::string AsEWKB() const override {
        return EWKB;
    }

    std::string AsEWKT() const override;

     /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;

    bg::model::polygon<bg::model::point<double, 2, SRID_Type>> GetSpatialData() {
        return Polygon_;
    }

    bool operator==(const Polygon<SRID_Type>& other);
};
}  //  namespace lgraph_api
