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
#include "lgraph/lgraph_exceptions.h"
#include <boost/geometry/geometry.hpp>

namespace lgraph_api {

bool Endian(const std::string& ewkb) {
    if (ewkb.size() < 42)
        THROW_CODE(InputError, "wrong wkb/ewkb format");

    std::string endian = ewkb.substr(0, 2);
    if (endian == "01") {
        return true;
    }
    return false;
}

void EndianTransfer(std::string& input) {
    size_t size = input.size();
    if (size % 2 || size <= 2)
        THROW_CODE(InputError, "invalid endian transfer data!");
    int i = size - 2;
    std::string output;
    while (i >= 0) {
        output += input.substr(i, 2);
        i -= 2;
    }
    input = output;
}

std::string Srid2Hex(SRID srid_type, size_t width) {
    if (width < 4)
        THROW_CODE(InputError, "invalid width in Srid2Hex!");
    int srid = static_cast<int>(srid_type);
    std::stringstream ioss;
    std::string s_hex;
    ioss << std::hex << srid;
    ioss >> s_hex;
    if (width > s_hex.size()) {
        std::string s_0(width - s_hex.size(), '0');
        s_hex = s_0 + s_hex;
    }

    s_hex = s_hex.substr(s_hex.length() - width, s_hex.length());
    EndianTransfer(s_hex);   // little endian in default;

    return s_hex;
}

// there's some problem in big2little transform
void WkbEndianTransfer(std::string& wkb) {
    if (wkb.size() < 42)
        THROW_CODE(InputError, "wrong wkb type!");
    // transfer the first byte which represents the big/little endian;
    std::string output;
    std::string en = wkb.substr(0, 2);
    if (en == "01") en = "00";
    else if (en == "00") en = "01";
    output += en;

    // transfer the next 4 bytes which represents the type of spatial data;
    std::string t = wkb.substr(2, 8);
    EndianTransfer(t);
    output += t;
    size_t start = 10;
    SpatialType s = ExtractType(wkb);
    // for LineString/Polygon we need 4 bytes to represent the number of
    // Point pairs/rings;
    if (s == SpatialType::LINESTRING ||
        s == SpatialType::POLYGON) {
        std::string t = wkb.substr(start, 8);
        EndianTransfer(t);
        output += t;
        start += 8;
    }
    // for Polygon, we need extra 4 bytes to represent the number of Point
    if (s == SpatialType::POLYGON) {
        std::string t = wkb.substr(start, 8);
        EndianTransfer(t);
        output += t;
        start += 8;
    }

    // transfer the next n * 8 byte(each represent a Point data);
    while (start < wkb.size()) {
        std::string data = wkb.substr(start, 16);
        EndianTransfer(data);
        output += data;
        start += 16;
    }

    wkb = output;
}

std::string EwkbEndianTransfer(const std::string& ewkb) {
    if (ewkb.size() < 50)
        THROW_CODE(InputError, "wrong wkb type!");
    std::string output;
    std::string tmp = ewkb.substr(0, 2);  // transfer the first byte which represents the type;
    if (tmp == "01") tmp = "00";
    else if (tmp == "00") tmp = "01";
    output += tmp;

    tmp = ewkb.substr(2, 4);   // transfer the next 2 bytes which represents type;
    EndianTransfer(tmp);
    output += tmp;
    tmp = ewkb.substr(6, 4);  // transfer the next 2 bytes which represents dimension;
    EndianTransfer(tmp);
    output += tmp;
    tmp = ewkb.substr(10, 8);  // transfer the next 4 bytes which represents srid;
    EndianTransfer(tmp);
    output += tmp;

    SpatialType s = ExtractType(ewkb);
    size_t start = 18;
    // for LineString/Polygon we need 4 bytes to represent the number of
    // Point pairs/rings;
    if (s == SpatialType::LINESTRING ||
        s == SpatialType::POLYGON) {
        std::string t = ewkb.substr(start, 8);
        EndianTransfer(t);
        output += t;
        start += 8;
    }
    // for Polygon, we need extra 4 bytes to represent the number of Point
    if (s == SpatialType::POLYGON) {
        std::string t = ewkb.substr(start, 8);
        EndianTransfer(t);
        output += t;
        start += 8;
    }
    // transfer the next n * 8 byte(each represent a Point data);
    while (start < ewkb.size()) {
        std::string data = ewkb.substr(start, 16);
        EndianTransfer(data);
        output += data;
        start += 16;
    }

    return output;
}

std::string SetExtension(const std::string& wkb, SRID srid_type) {
    bool en = Endian(wkb);
    std::string ewkb = wkb;

    // set the dimension information;
    if (en)
        ewkb[8] = '2';
    else
        ewkb[6] = '2';

    // transform srid into hex format in little endian;
    std::string s_hex = Srid2Hex(srid_type, 8);
    transform(s_hex.begin(), s_hex.end(), s_hex.begin(), ::toupper);

    // if the wkb is in big endian, we need transform it;
    if (!en)
        EndianTransfer(s_hex);
    // insert the srid information into EWKB format;
    ewkb.insert(10, s_hex);

    return ewkb;
}

SRID ExtractSRID(const std::string& ewkb) {
    // only ewkb format have srid information;
    if (ewkb.size() < 50)
        THROW_CODE(InputError, "wrong EWKB type");

    std::string srid = ewkb.substr(10, 8);
    // transfer the little endian data into big endian for convenient;
    if (Endian(ewkb)) {
        EndianTransfer(srid);
    }

    std::stringstream ioss(srid);
    int Srid;
    ioss >> std::hex >> Srid;
    switch (Srid) {
        case 0:
            return SRID::NUL;
        case 4326:
            return SRID::WGS84;
        case 7203:
            return SRID::CARTESIAN;
        default:
            THROW_CODE(InputError, "Unsupported SRID!");
    }
}

SpatialType ExtractType(const std::string& ewkb) {
    std::string type = ewkb.substr(2, 8);
    // if the input format is EWKB type, we only need the first 2 bytes;
    if (type[4] != '0' || type[6] != '0')
        type = type.substr(0, 4);
    // transfer the little endian into big endian for convenient;
    if (Endian(ewkb)) {
        EndianTransfer(type);
    }

    int Type = 0;
    std::stringstream ioss(type);
    ioss >> Type;

    switch (Type) {
        case 0:
            return SpatialType::NUL;
        case 1:
            return SpatialType::POINT;
        case 2:
            return SpatialType::LINESTRING;
        case 3:
            return SpatialType::POLYGON;
        default:
            THROW_CODE(InputError, "Unkown Spatial Type!");
    }
}

template<typename SRID_Type>
bool TryDecodeWKB(const std::string& wkb, SpatialType type) {
    typedef bg::model::point<double, 2, SRID_Type> Point;
    switch (type) {
        case SpatialType::NUL:
            return false;
        case SpatialType::POINT: {
            Point point_;
            ByteVector wkb_;

            if (!bg::hex2wkb(wkb, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), point_)) {
                return false;
            }
            return true;
        }

        case SpatialType::LINESTRING: {
            bg::model::linestring<Point> line_;
            ByteVector wkb_;

            if (!bg::hex2wkb(wkb, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
                return false;
            }
            return true;
        }

        case SpatialType::POLYGON: {
            bg::model::polygon<Point> polygon_;
            ByteVector wkb_;

            if (!bg::hex2wkb(wkb, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), polygon_)) {
                return false;
            }
            return true;
        }

        default:
            return false;
    }
}

bool TryDecodeEWKB(const std::string& ewkb, SpatialType type) {
    // the EWKB format's is at least 50;
    if (ewkb.size() < 50)
        return false;

    char dim = ewkb[8] > ewkb[6] ? ewkb[8] : ewkb[6];
    if (dim != '2')
        return false;
    SRID s = ExtractSRID(ewkb);
    SpatialType t = ExtractType(ewkb);
    if (t != type)
        return false;

    std::string wkb = ewkb.substr(0, 10) + ewkb.substr(18);
    wkb[8] = '0';
    wkb[6] = '0';

    switch (s) {
        case SRID::NUL:
            return false;
        case SRID::WGS84:
            return TryDecodeWKB<Wgs84>(wkb, t);
        case SRID::CARTESIAN:
            return TryDecodeWKB<Cartesian>(wkb, t);
        default:
            return false;
    }
}


template<typename SRID_Type>
Spatial<SRID_Type>::Spatial(SRID srid, SpatialType type, int construct_type, std::string content) {
    type_ = type;
    switch (type) {
        case SpatialType::NUL:
            THROW_CODE(InputError, "Unknown Spatial Type");
        case SpatialType::POINT:
            point_.reset(new Point<SRID_Type>(srid, type, construct_type, content));
            break;
        case SpatialType::LINESTRING:
            line_.reset(new LineString<SRID_Type>(srid, type, construct_type, content));
            break;
        case SpatialType::POLYGON:
            polygon_.reset(new Polygon<SRID_Type>(srid, type, construct_type, content));
            break;
    }
}

template<typename SRID_Type>
Spatial<SRID_Type>::Spatial(const std::string& ewkb) {
    type_ = ExtractType(ewkb);

    switch (type_) {
        case SpatialType::NUL:
            THROW_CODE(InputError, "Unknown Spatial Type");
        case SpatialType::POINT:
            point_.reset(new Point<SRID_Type>(ewkb));
            break;
        case SpatialType::LINESTRING:
            line_.reset(new LineString<SRID_Type>(ewkb));
            break;
        case SpatialType::POLYGON:
            polygon_.reset(new Polygon<SRID_Type>(ewkb));
    }
}

template<typename SRID_Type>
std::string Spatial<SRID_Type>::AsEWKB() const {
    switch (type_) {
        case SpatialType::NUL:
            return "NUL";
        case SpatialType::POINT:
            return point_->AsEWKB();
        case SpatialType::LINESTRING:
            return line_->AsEWKB();
        case SpatialType::POLYGON:
            return polygon_->AsEWKB();
        default:
            throw std::runtime_error("Unknown SRID Type");
    }
}

template<typename SRID_Type>
std::string Spatial<SRID_Type>::AsEWKT() const {
    switch (type_) {
        case SpatialType::NUL:
            return "NUL";
        case SpatialType::POINT:
            return point_->AsEWKT();
        case SpatialType::LINESTRING:
            return line_->AsEWKT();
        case SpatialType::POLYGON:
            return polygon_->AsEWKT();
        default:
            throw std::runtime_error("Unknown SRID Type");
    }
}

template<typename SRID_Type>
std::string Spatial<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
double Spatial<SRID_Type>::Distance(Spatial<SRID_Type>& other) {
    switch (type_) {
        case SpatialType::POINT:
        {
            switch (other.GetType()) {
                case SpatialType::POINT:
                    return point_->Distance(*other.GetPoint().get());
                case SpatialType::LINESTRING:
                    return point_->Distance(*other.GetLine().get());
                case SpatialType::POLYGON:
                    return point_->Distance(*other.GetPolygon().get());
                default:
                    throw std::runtime_error("unsupported spatial type!");
            }
        }

        case SpatialType::LINESTRING:
        {
            switch (other.GetType()) {
                case SpatialType::POINT:
                    return line_->Distance(*other.GetPoint().get());
                case SpatialType::LINESTRING:
                    return line_->Distance(*other.GetLine().get());
                case SpatialType::POLYGON:
                    return line_->Distance(*other.GetPolygon().get());
                default:
                    throw std::runtime_error("unsupported spatial type!");
            }
        }

        case SpatialType::POLYGON:
        {
            switch (other.GetType()) {
                case SpatialType::POINT:
                    return polygon_->Distance(*other.GetPoint().get());
                case SpatialType::LINESTRING:
                    return polygon_->Distance(*other.GetLine().get());
                case SpatialType::POLYGON:
                    return polygon_->Distance(*other.GetPolygon().get());
                default:
                    throw std::runtime_error("unsupported spatial type!");
            }
        }

        default:
            THROW_CODE(InputError, "unsupported spatial type!");
    }
}

template<typename SRID_Type>
bool Spatial<SRID_Type>::operator==(const Spatial<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template<typename SRID_Type>
Point<SRID_Type>::Point(SRID srid, SpatialType type, int construct_type, std::string& content)
: SpatialBase(srid, type) {
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wgs84>::value && srid != SRID::WGS84)) {
        THROW_CODE(InputError, "template srid dismatch with input srid");
    }

    if (construct_type == 0) {
        // first, tranfer the big endian into little endian;
        if (!Endian(content))
            WkbEndianTransfer(content);
        ByteVector wkb_;

        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), point_)) {
            THROW_CODE(InputError, "wrong wkb format: " + content);
        }
        // extend wkb format to ewkb format
        ewkb_ = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;

        // exception will be thrown if the wkt format is wrong; boost::exception_detail;
        try {
            bg::read_wkt(content, point_);
        } catch (const std::exception &ex) {
            THROW_CODE(InputError, "wrong wkt format:" + std::string(ex.what()));
        }

        if (!bg::write_wkb(point_, std::back_inserter(wkb_out))) {
            THROW_CODE(InputError, "wrong wkt format: " + content);
        }

        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), ewkb_);
        ewkb_ = SetExtension(ewkb_, GetSrid());
    }

    transform(ewkb_.begin(), ewkb_.end(), ewkb_.begin(), ::toupper);
}

template<typename SRID_Type>
Point<SRID_Type>::Point(const std::string& ewkb)
: SpatialBase(ExtractSRID(ewkb), ExtractType(ewkb)) {
    // first, we need to transfer big endian to little endian;
    SRID srid = ExtractSRID(ewkb);
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wgs84>::value && srid != SRID::WGS84)) {
        THROW_CODE(InputError, "template srid dismatch with input srid");
    }
    if (!Endian(ewkb))
        ewkb_ = EwkbEndianTransfer(ewkb);
    else
        ewkb_ = ewkb;
    std::string wkb = ewkb_.substr(0, 10) + ewkb_.substr(18);
    wkb[8] = '0';
    ByteVector wkb_;

    if (!bg::hex2wkb(wkb, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), point_)) {
        THROW_CODE(InputError, "wrong wkb format: " + wkb);
    }

    transform(ewkb_.begin(), ewkb_.end(), ewkb_.begin(), ::toupper);
}

template<typename SRID_Type>
Point<SRID_Type>::Point(double arg1, double arg2, SRID& srid)
: SpatialBase(srid, SpatialType::POINT) {
    bg::set<0>(point_, arg1);
    bg::set<1>(point_, arg2);
    // write wkb;
    std::string wkb_out;
    bg::write_wkb(point_, std::back_inserter(wkb_out));
    std::string hex_out;
    if (!bg::wkb2hex(wkb_out.begin(), wkb_out.end(), hex_out))
        THROW_CODE(InputError, "wrong point data!");
    // set extension
    ewkb_ = SetExtension(hex_out, srid);
    transform(ewkb_.begin(), ewkb_.end(), ewkb_.begin(), ::toupper);
}

template<typename SRID_Type>
std::string Point<SRID_Type>::AsEWKT() const {
    std::string ewkt;
    std::stringstream ioss;
    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(point_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        ewkt += tmp;
        ewkt += ' ';
    }
    ewkt.pop_back();

    return ewkt;
}

template<typename SRID_Type>
std::string Point<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
double Point<SRID_Type>::Distance(Point<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(point_, other.GetSpatialData());
}

template<typename SRID_Type>
double Point<SRID_Type>::Distance(LineString<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(point_, other.GetSpatialData());
}

template<typename SRID_Type>
double Point<SRID_Type>::Distance(Polygon<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(point_, other.GetSpatialData());
}

template<typename SRID_Type>
bool Point<SRID_Type>::operator==(const Point<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template<typename SRID_Type>
LineString<SRID_Type>::LineString(SRID srid, SpatialType type,
int construct_type, std::string& content)
: SpatialBase(srid, type) {
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wgs84>::value && srid != SRID::WGS84)) {
        THROW_CODE(InputError, "template srid dismatch with input srid");
    }

    if (construct_type == 0) {
        if (!Endian(content))
            WkbEndianTransfer(content);
        ByteVector wkb_;
        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
            THROW_CODE(InputError, "wrong wkb format: " + content);
        }
        ewkb_ = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;
        try {
            bg::read_wkt(content, line_);
        } catch(const std::exception &ex) {
            THROW_CODE(InputError, "wrong wkt format" + std::string(ex.what()));
        }

        if (!bg::write_wkb(line_, std::back_inserter(wkb_out))) {
            THROW_CODE(InputError, "wrong wkt format: " + content);
        }
        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), ewkb_);
        ewkb_ = SetExtension(ewkb_, GetSrid());
    }
    transform(ewkb_.begin(), ewkb_.end(), ewkb_.begin(), ::toupper);
}

template<typename SRID_Type>
LineString<SRID_Type>::LineString(const std::string& ewkb)
: SpatialBase(ExtractSRID(ewkb), ExtractType(ewkb)) {
    SRID srid = ExtractSRID(ewkb);
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wgs84>::value && srid != SRID::WGS84)) {
        THROW_CODE(InputError, "template srid dismatch with input srid");
    }

    if (!Endian(ewkb))
        ewkb_ = EwkbEndianTransfer(ewkb);
    else
        ewkb_ = ewkb;
    std::string wkb = ewkb_.substr(0, 10) + ewkb_.substr(18);
    wkb[8] = '0';
    ByteVector wkb_;

    if (!bg::hex2wkb(wkb, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
        THROW_CODE(InputError, "wrong wkb format: " + wkb);
    }
}

template<typename SRID_Type>
std::string LineString<SRID_Type>::AsEWKT() const {
    std::string ewkt;
    std::stringstream ioss;

    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(line_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        ewkt += tmp;
        ewkt += ' ';
    }
    ewkt.pop_back();

    return ewkt;
}

template<typename SRID_Type>
std::string LineString<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
double LineString<SRID_Type>::Distance(Point<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(line_, other.GetSpatialData());
}

template<typename SRID_Type>
double LineString<SRID_Type>::Distance(LineString<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(line_, other.GetSpatialData());
}

template<typename SRID_Type>
double LineString<SRID_Type>::Distance(Polygon<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(line_, other.GetSpatialData());
}

template<typename SRID_Type>
bool LineString<SRID_Type>::operator==(const LineString<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template<typename SRID_Type>
Polygon<SRID_Type>::Polygon(SRID srid, SpatialType type, int construct_type, std::string& content)
: SpatialBase(srid, type) {
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wgs84>::value && srid != SRID::WGS84)) {
        THROW_CODE(InputError, "template srid dismatch with input srid");
    }

    if (construct_type == 0) {
        if (!Endian(content))
            WkbEndianTransfer(content);
        ByteVector wkb_;
        bg::hex2wkb(content, std::back_inserter(wkb_));
        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), polygon_)) {
            THROW_CODE(InputError, "wrong wkb format: " + content);
        }

        ewkb_ = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;

        try {
            bg::read_wkt(content, polygon_);
        } catch (const std::exception &ex) {
            THROW_CODE(InputError, "wrong wkt format" + std::string(ex.what()));
        }

        if (!bg::write_wkb(polygon_, std::back_inserter(wkb_out))) {
            THROW_CODE(InputError, "wrong wkt format: " + content);
        }

        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), ewkb_);
        ewkb_ = SetExtension(ewkb_, GetSrid());
    }
    transform(ewkb_.begin(), ewkb_.end(), ewkb_.begin(), ::toupper);
}

template<typename SRID_Type>
Polygon<SRID_Type>::Polygon(const std::string& ewkb)
: SpatialBase(ExtractSRID(ewkb), ExtractType(ewkb)) {
    SRID srid = ExtractSRID(ewkb);
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wgs84>::value && srid != SRID::WGS84)) {
        THROW_CODE(InputError, "template srid dismatch with input srid");
    }

    if (!Endian(ewkb))
        ewkb_ = EwkbEndianTransfer(ewkb);
    else
        ewkb_ = ewkb;
    std::string wkb = ewkb_.substr(0, 10) + ewkb_.substr(18);
    wkb[8] = '0';

    ByteVector wkb_;

    if (!bg::hex2wkb(wkb, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), polygon_)) {
        THROW_CODE(InputError, "wrong wkb format: " + wkb);
    }
}

template<typename SRID_Type>
std::string Polygon<SRID_Type>::AsEWKT() const {
    std::string ewkt;
    std::stringstream ioss;

    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(polygon_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        ewkt += tmp;
        ewkt += ' ';
    }
    ewkt.pop_back();

    return ewkt;
}

template<typename SRID_Type>
double Polygon<SRID_Type>::Distance(Point<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(polygon_, other.GetSpatialData());
}

template<typename SRID_Type>
double Polygon<SRID_Type>::Distance(LineString<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(polygon_, other.GetSpatialData());
}

template<typename SRID_Type>
double Polygon<SRID_Type>::Distance(Polygon<SRID_Type>& other) {
    if (other.GetSrid() != GetSrid())
        THROW_CODE(InputError, "distance srid missmatch!");
    return bg::distance(polygon_, other.GetSpatialData());
}

template<typename SRID_Type>
std::string Polygon<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
bool Polygon<SRID_Type>::operator==(const Polygon<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template class Point<Wgs84>;
template class Point<Cartesian>;
template class LineString<Wgs84>;
template class LineString<Cartesian>;
template class Polygon<Wgs84>;
template class Polygon<Cartesian>;
template class Spatial<Wgs84>;
template class Spatial<Cartesian>;

}  // namespace lgraph_api
