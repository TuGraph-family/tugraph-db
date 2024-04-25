//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/**
 * @file  lgraph_spatial.h
 * @brief Implemnets the Spatial, SpatialBase and SpatialDerive classes.
 * 
 * TODO(shw):
 *   1. add more tests for the conversion between string type and spatial type in
 *      funciton ParseStringToValueOfFieldType and TryFieldDataToValueOfFieldType. 
 *   2. support the compression of EWKB, which is now the format of spatial type storage.
 *   3. more comprehensive and readable comments.
 *   4. use FieldType2CType to simplify the realization of spatial data function;
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
typedef std::vector<boost::uint8_t> ByteVector;

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
 *          for more information about SRID, you can refer to: https://spatialreference.org/ref/
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
* @param       ewkb expected wkb/ewkb format string;
*
* @returns     true if little endian, false if big endian;
*/
bool Endian(const std::string& ewkb);

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
void EndianTransfer(std::string& input);

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
* @param [in, out] wkb  the wkb format data in big/little endian, out int little/big endian;
*/
void WkbEndianTransfer(std::string& wkb);

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
* @param ewkb  the EWKB format data in big/little endian;
*
* @returns the EWKB format data in little/big endian;
*/
std::string EwkbEndianTransfer(const std::string& ewkb);

/**
* @brief  extend the wkb format string to EWKB format string;
*         you can find the difference between WKB and EWKB format
*         at the comments of TryDecodeWKB and TryDecodeEWKB
*              
* @param wkb          the wkb format to be extended   
*
* @param srid_type    the srid type to be added in the format
* 
* @returns the EWKB foramt data extended from the input wkb format data;
*/
std::string SetExtension(const std::string& wkb, SRID srid_type);

// extract the srid from EWKB format;
SRID ExtractSRID(const std::string& ewkb);

// extract the spatial type from EWKB format;
SpatialType ExtractType(const std::string& ewkb);

/**
* @brief   the WKB(well known binary ) layout of spatial data(little endian):
*          WKB is the most widely used format to represent spatial data (in hex format)
*          for more information, you can refer to: 
*          https://postgis.net/docs/using_postgis_dbmanagement.html#WKB_WKT
*          Point: 01  01000000  000000000000F03F  0000000000000040 Point(1.0 2.0) 
*          01:       1byte  the way of encoding, 00 represents big-endian, 01 represents little-endian
*          01000000: 4bytes spatial data type, the little endian of 00000001, represents Point. 
*                    02 LineString 03 Polygon 
*          then each 4 bytes represent a datum, each 8 bytes represent a point;
*           
*          LineString: 01 02000000 03000000 0000000000000000 0000000000000000
*          0000000000000040 0000000000000040 0000000000000840 000000000000F03F
*          LineString(0 0,2 2,3 1)
*          01:       1byte  the way of encoding;
*          02000000: 4bytes spatial data type  02 represents LineString
*          03000000: 4bytes represents there are some point in LineString;
*          then every 8 bytes represents a point;
*
*          Polygon: 01 03000000 01000000 05000000 0000000000000000 000000000
*          0000000 0000000000000000 0000000000001C40 0000000000001040 0000000000000040
*          0000000000000040 0000000000000000 0000000000000000 0000000000000000
*          Polygon((0 0,0 1,1 1,1 0,0 0))
*          01:       1byte  the way of encoding;
*          03000000: 4bytes spatial data type  03 represents polygon
*          01000000: 4bytes there's a ring in polygon;
*          05000000: 4bytes there are 5 points in the ring;
*          then every 8 bytes represents a point;     
*          
*          big endian format:
*          Convert each part of the above WKB format, separated by spaces,
*          from little endian to big endian.
*          Point:
*          00 0001 2000 000010E6 3FF0000000000000 3FF0000000000000
*          
*          Implementation:
*          we use the read_wkb function in boost/geometry/extension to verify
*          whether the wkb format is correct;
*
* @param wkb           the string to be parse;
* @param type          the type of spatial data;
* 
* @returns true if WKB format is valid, false if wkb format is invalid;
*/
template<typename SRID_Type>
bool TryDecodeWKB(const std::string& wkb, SpatialType type);

/**
* @brief   the EWKB(extended well known binary) layout of spatial data:
*          EWKB is not an offical spatial data foramt, it is defined in postgis. The advantages of
*          EWKB is it contains the information of srid and dimension. For more information, you can
*          refer to: https://postgis.net/docs/using_postgis_dbmanagement.html#EWKB_EWKT
*          Point: 01 0100 0020 E6100000 000000000000F03F 0000000000000040 
*          SRID=4326 Point(1.0 2.0)
*          01:           the way of encoding, 00 represents big-endian, 01 represents little-endian
*          0100:         spatial data type, 0100 represents Point; (compared with WKB, 4bytes to 2bytes)
*          0020:         the dimension is 2;
*          E6010000:     4bytes SRID -- 4326；  little endian representation of 0x000010e6; 
*          then each 8 bytes represent a point;
*          
*          LineString: 01 0200 0020 E6100000 03000000 0000000000000000 0000000000000000
*          0000000000000040 0000000000000040 0000000000000840 000000000000F03F
*          SRID=4326 LineString(0 0,2 2,3 1)
*          01:   1byte  
*          0200: 2bytes spatial type
*          0020: 2bytes dimension
*          E6010000: 4bytes SRID -- 4326；  
*          03000000: 4bytes there're 3 points in linestring
*          
*          Polygon:
*          01 0300 0020 e6100000 01000000 05000000 0000000000000000 0000000000000000
*          0000000000000000 000000000000f03f 000000000000f03f 000000000000f03f 000000000000f03f
*          0000000000000000 0000000000000000 0000000000000000
*          SRID=4326;Polygon((0 0,0 1,1 1,1 0,0 0)) 
*
* @param ewkb          the string to be parse;
* @param type          the type of spatial data;
* 
* @returns true if WKB format is valid, false if WKB format is invalid;
*/
bool TryDecodeEWKB(const std::string& ewkb, SpatialType type);

/** @brief   Implements the Spatial template class. Spatial class now can hold one of
 *  Point, LineString or Polygon Pointer;
 */
template<typename SRID_Type>
class Spatial {
    std::unique_ptr<Point<SRID_Type>> point_;
    std::unique_ptr<LineString<SRID_Type>> line_;
    std::unique_ptr<Polygon<SRID_Type>> polygon_;

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
    explicit Spatial(const std::string& ewkb);

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
        return point_;
    }

    /** @brief return the line type Pointer */
    std::unique_ptr<LineString<SRID_Type>>& GetLine() {
        return line_;
    }

    /** @brief return the Polygon type Pointer */
    std::unique_ptr<Polygon<SRID_Type>>& GetPolygon() {
        return polygon_;
    }

    /** @brief return the type of spatial*/
    SpatialType GetType() {
        return type_;
    }

    /**
     *  @brief return the distance between two spatial data;
     * 
     *  @param other the other data of spatial type;
    */
    double Distance(Spatial<SRID_Type>& other);

    bool operator==(const Spatial<SRID_Type>& other);
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

    virtual std::string AsEWKB() const = 0;

    virtual std::string AsEWKT() const = 0;

    SpatialType GetType() const {
        return type_;
    }

    SRID GetSrid() const {
        return srid_;
    }
};

/**
 * @brief implements a Point spatial class which spatial type is Point; 
 */
template<typename SRID_Type>
class Point : public SpatialBase {
    std::string ewkb_;
    bg::model::point<double, 2, SRID_Type> point_;

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
    explicit Point(const std::string& ewkb);

    /**
     *  @brief construct Point from Coordinate pairs and srid;
     *  
     *  @param arg1 the first coordinate datum;
     * 
     *  @param arg2 the second coordinate datum; 
     * 
     *  @param srid the srid of the Point;
    */
    Point(double arg1, double arg2, SRID& srid);

    std::string AsEWKB() const override {
        return ewkb_;
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
        return point_;
    }

    /**
     *  @brief caculate the distance between data of point type;
     * 
     *  @param other the other data of point type;
    */
    double Distance(Point<SRID_Type>& other);

    /**
     *  @brief caculate the distance between point and linestring;
     *  
     *  @param other the other data of linestring type;
    */
    double Distance(LineString<SRID_Type>& other);

    /**
     *  @brief caculate the distance between point and polygon;
     *  
     *  @param other the other data of polygon type;
    */
    double Distance(Polygon<SRID_Type>& other);

    bool operator==(const Point<SRID_Type>& other);
};

/**
 * @brief implements a LineString spatial class which spatial data is Point; 
 */
template<typename SRID_Type>
class LineString : public SpatialBase {
    std::string ewkb_;
    typedef bg::model::point<double, 2, SRID_Type> Point_;
    bg::model::linestring<Point_> line_;

 public:
    LineString(SRID srid, SpatialType type, int construct_type, std::string& content);

    explicit LineString(const std::string& ewkb);

    std::string AsEWKB() const override {
        return ewkb_;
    }

    std::string AsEWKT() const override;

     /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;

    bg::model::linestring<bg::model::point<double, 2, SRID_Type>> GetSpatialData() {
        return line_;
    }

    /**
     *  @brief caculate the distance between linestring and point;
     * 
     *  @param other the other data of distance type;
    */
    double Distance(Point<SRID_Type>& other);

    /**
     *  @brief caculate the distance between linestring and linestring;
     *  
     *  @param other the other data of linestring type;
    */
    double Distance(LineString<SRID_Type>& other);

    /**
     *  @brief caculate the distance between linestring and polygon;
     *  
     *  @param other the other data of polygon type;
    */
    double Distance(Polygon<SRID_Type>& other);

    bool operator==(const LineString<SRID_Type>& other);
};

/**
 * @brief implements a Polygon spatial class which spatial data is Point; 
 */
template<typename SRID_Type>
class Polygon : public SpatialBase {
    std::string ewkb_;
    typedef bg::model::point<double, 2, SRID_Type> Point_;
    bg::model::polygon<Point_> polygon_;

 public:
    Polygon(SRID srid, SpatialType type, int construct_type, std::string& content);

    explicit Polygon(const std::string& ewkb);

    std::string AsEWKB() const override {
        return ewkb_;
    }

    std::string AsEWKT() const override;

     /**
     * @brief return EWKB format in default;
    */
    std::string ToString() const;

    bg::model::polygon<bg::model::point<double, 2, SRID_Type>> GetSpatialData() {
        return polygon_;
    }

    /**
     *  @brief caculate the distance between linestring and point;
     * 
     *  @param other the other data of point type;
    */
    double Distance(Point<SRID_Type>& other);

    /**
     *  @brief caculate the distance between linestring and linestring;
     *  
     *  @param other the other data of linestring type;
    */
    double Distance(LineString<SRID_Type>& other);

    /**
     *  @brief caculate the distance between linestring and polygon;
     *  
     *  @param other the other data of polygon type;
    */
    double Distance(Polygon<SRID_Type>& other);

    bool operator==(const Polygon<SRID_Type>& other);
};
}  //  namespace lgraph_api
