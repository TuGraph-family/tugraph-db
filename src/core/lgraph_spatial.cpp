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

namespace lgraph_api {

bool Endian(const std::string& EWKB) {
    if (EWKB.size() < 42)
        throw InputError("wrong wkb/ewkb format");

    std::string endian = EWKB.substr(0, 2);
    if (endian == "01") {
        return true;
    }
    return false;
}

void EndianTansfer(std::string& input) {
    size_t size = input.size();
    if (size % 2 || size <= 2)
        throw InputError("invalid endian transfer data!");
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
        throw InputError("invalid width in Srid2Hex!");
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
    EndianTansfer(s_hex);   // little endian in default;

    return s_hex;
}

// there's some problem in big2little transform
void WkbEndianTransfer(std::string& WKB) {
    if (WKB.size() < 42)
        throw InputError("wrong wkb type!");
    // transfer the first byte which represents the big/little endian;
    std::string output;
    std::string en = WKB.substr(0, 2);
    if (en == "01") en = "00";
    else if (en == "00") en = "01";
    output += en;

    // transfer the next 4 bytes which represents the type of spatial data;
    std::string t = WKB.substr(2, 8);
    EndianTansfer(t);
    output += t;
    size_t start = 10;
    // for LineString/Polygon we need 4 bytes to represent the number of
    // Point pairs/rings;
    if (ExtractType(WKB) == SpatialType::LINESTRING ||
        ExtractType(WKB) == SpatialType::POLYGON) {
        std::string t = WKB.substr(start, 8);
        EndianTansfer(t);
        output += t;
        start += 8;
    }
    // for Polygon, we need extra 4 bytes to represent the number of Point
    if (ExtractType(WKB) == SpatialType::POLYGON) {
        std::string t = WKB.substr(start, 8);
        EndianTansfer(t);
        output += t;
        start += 8;
    }

    // transfer the next n * 8 byte(each represent a Point data);
    while (start < WKB.size()) {
        std::string data = WKB.substr(start, 16);
        EndianTansfer(data);
        output += data;
        start += 16;
    }

    WKB = output;
}

std::string EwkbEndianTransfer(const std::string& EWKB) {
    if (EWKB.size() < 50)
        throw InputError("wrong wkb type!");
    std::string output;
    std::string tmp = EWKB.substr(0, 2);  // transfer the first byte which represents the type;
    if (tmp == "01") tmp = "00";
    else if (tmp == "00") tmp = "01";
    output += tmp;

    tmp = EWKB.substr(2, 4);   // transfer the next 2 bytes which represents type;
    EndianTansfer(tmp);
    output += tmp;
    tmp = EWKB.substr(6, 4);  // transfer the next 2 bytes which represents dimension;
    EndianTansfer(tmp);
    output += tmp;
    tmp = EWKB.substr(10, 8);  // transfer the next 4 bytes which represents srid;
    EndianTansfer(tmp);
    output += tmp;

    size_t start = 18;
    // for LineString/Polygon we need 4 bytes to represent the number of
    // Point pairs/rings;
    if (ExtractType(EWKB) == SpatialType::LINESTRING ||
        ExtractType(EWKB) == SpatialType::POLYGON) {
        std::string t = EWKB.substr(start, 8);
        EndianTansfer(t);
        output += t;
        start += 8;
    }
    // for Polygon, we need extra 4 bytes to represent the number of Point
    if (ExtractType(EWKB) == SpatialType::POLYGON) {
        std::string t = EWKB.substr(start, 8);
        EndianTansfer(t);
        output += t;
        start += 8;
    }
    // transfer the next n * 8 byte(each represent a Point data);
    while (start < EWKB.size()) {
        std::string data = EWKB.substr(start, 16);
        EndianTansfer(data);
        output += data;
        start += 16;
    }

    return output;
}

std::string SetExtension(const std::string& WKB, SRID srid_type) {
    bool en = Endian(WKB);
    std::string EWKB = WKB;

    // set the dimension information;
    if (en)
        EWKB[8] = '2';
    else
        EWKB[6] = '2';

    // transform srid into hex format in little endian;
    std::string s_hex = Srid2Hex(srid_type, 8);
    transform(s_hex.begin(), s_hex.end(), s_hex.begin(), ::toupper);

    // if the wkb is in big endian, we need transform it;
    if (!en)
        EndianTansfer(s_hex);
    // insert the srid information into EWKB format;
    EWKB.insert(10, s_hex);

    return EWKB;
}

SRID ExtractSRID(const std::string& EWKB) {
    // only ewkb format have srid information;
    if (EWKB.size() < 50)
        throw InputError("wrong EWKB type");

    std::string srid = EWKB.substr(10, 8);
    // transfer the little endian data into big endian for convenient;
    if (Endian(EWKB)) {
        EndianTansfer(srid);
    }

    std::stringstream ioss(srid);
    int Srid;
    ioss >> std::hex >> Srid;
    switch (Srid) {
        case 0:
            return SRID::NUL;
        case 4326:
            return SRID::WSG84;
        case 7203:
            return SRID::CARTESIAN;
        default:
            throw InputError("Unsupported SRID!");
    }
}

SpatialType ExtractType(const std::string& EWKB) {
    // if (EWKB.size() < 50)
    //    throw InputError("wrong EWKB type");

    std::string type = EWKB.substr(2, 8);
    // if the input format is EWKB type, we only need the first 2 bytes;
    if (type[4] != '0' || type[6] != '0')
        type = type.substr(0, 4);
    // transfer the little endian into big endian for convenient;
    if (Endian(EWKB)) {
        EndianTansfer(type);
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
            throw std::runtime_error("wrong SpatialType!");
    }
}

template<typename SRID_Type>
bool TryDecodeWKB(const std::string& WKB, SpatialType type) {
    typedef bg::model::point<double, 2, SRID_Type> Point;
    switch (type) {
        case SpatialType::NUL:
            return false;
        case SpatialType::POINT: {
            Point Point_;
            byte_vector wkb_;

            if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), Point_)) {
                return false;
            }
            return true;
        }

        case SpatialType::LINESTRING: {
            bg::model::linestring<Point> line_;
            byte_vector wkb_;

            if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
                return false;
            }
            return true;
        }

        case SpatialType::POLYGON: {
            bg::model::polygon<Point> Polygon_;
            byte_vector wkb_;

            if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), Polygon_)) {
                return false;
            }
            return true;
        }

        default:
            return false;
    }
}

bool TryDecodeEWKB(const std::string& EWKB, SpatialType type) {
    // the EWKB format's is at least 50;
    if (EWKB.size() < 50)
        return false;

    char dim = EWKB[8] > EWKB[6] ? EWKB[8] : EWKB[6];
    if (dim != '2')
        return false;
    SRID s = ExtractSRID(EWKB);
    SpatialType t = ExtractType(EWKB);
    if (t != type)
        return false;

    std::string WKB = EWKB.substr(0, 10) + EWKB.substr(18);
    WKB[8] = '0';
    WKB[6] = '0';

    switch (s) {
        case SRID::NUL:
            return false;
        case SRID::WSG84:
            return TryDecodeWKB<Wsg84>(WKB, t);
        case SRID::CARTESIAN:
            return TryDecodeWKB<Cartesian>(WKB, t);
        default:
            return false;
    }
}


template<typename SRID_Type>
Spatial<SRID_Type>::Spatial(SRID srid, SpatialType type, int construct_type, std::string content) {
    type_ = type;
    switch (type) {
        case SpatialType::NUL:
            throw std::runtime_error("Unknown Spatial Type");
        case SpatialType::POINT:
            Point_.reset(new Point<SRID_Type>(srid, type, construct_type, content));
            break;
        case SpatialType::LINESTRING:
            line_.reset(new LineString<SRID_Type>(srid, type, construct_type, content));
            break;
        case SpatialType::POLYGON:
            Polygon_.reset(new Polygon<SRID_Type>(srid, type, construct_type, content));
            break;
    }
}

template<typename SRID_Type>
Spatial<SRID_Type>::Spatial(const std::string& EWKB) {
    type_ = ExtractType(EWKB);
    SRID srid = ExtractSRID(EWKB);
    std::string WKB = EWKB.substr(0, 10) + EWKB.substr(18);
    WKB[8] = '0';
    WKB[6] = '0';

    switch (type_) {
        case SpatialType::NUL:
            throw InputError("invalid spatial type");
        case SpatialType::POINT:
            Point_.reset(new Point<SRID_Type>(srid, type_, 0, WKB));
            break;
        case SpatialType::LINESTRING:
            line_.reset(new LineString<SRID_Type>(srid, type_, 0, WKB));
            break;
        case SpatialType::POLYGON:
            Polygon_.reset(new Polygon<SRID_Type>(srid, type_, 0, WKB));
    }
}

template<typename SRID_Type>
std::string Spatial<SRID_Type>::AsEWKB() const {
    switch (type_) {
        case SpatialType::NUL:
            return "NUL";
        case SpatialType::POINT:
            return Point_->AsEWKB();
        case SpatialType::LINESTRING:
            return line_->AsEWKB();
        case SpatialType::POLYGON:
            return Polygon_->AsEWKB();
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
            return Point_->AsEWKT();
        case SpatialType::LINESTRING:
            return line_->AsEWKT();
        case SpatialType::POLYGON:
            return Polygon_->AsEWKT();
        default:
            throw std::runtime_error("Unknown SRID Type");
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
    || (std::is_same<SRID_Type, Wsg84>::value && srid != SRID::WSG84)) {
        throw InputError("template srid dismatch with input srid");
    }

    if (construct_type == 0) {
        // first, tranfer the big endian into little endian;
        if (!Endian(content))
            WkbEndianTransfer(content);
        byte_vector wkb_;

        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), Point_)) {
            throw InputError("wrong wkb format: " + content);
        }
        // extend wkb format to ewkb format
        EWKB = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;

        // 如果WKT格式错误，这里会抛出异常; boost::exception_detail;
        try {
            bg::read_wkt(content, Point_);
        } catch (const std::exception &ex) {
            throw InputError("wrong wkt format:" + std::string(ex.what()));
        }

        if (!bg::write_wkb(Point_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }

        // 这里要不要判断异常?
        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = SetExtension(EWKB, GetSrid());
    }

    transform(EWKB.begin(), EWKB.end(), EWKB.begin(), ::toupper);
}

template<typename SRID_Type>
Point<SRID_Type>::Point(const std::string& EWKB_)
: SpatialBase(ExtractSRID(EWKB_), ExtractType(EWKB_)) {
    // first, we need to transfer big endian to little endian;
    SRID srid = ExtractSRID(EWKB_);
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wsg84>::value && srid != SRID::WSG84)) {
        throw InputError("template srid dismatch with input srid");
    }
    if (!Endian(EWKB_))
        EWKB = EwkbEndianTransfer(EWKB_);
    else
        EWKB = EWKB_;
    std::string WKB = EWKB.substr(0, 10) + EWKB.substr(18);
    WKB[8] = '0';
    byte_vector wkb_;

    if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), Point_)) {
        throw InputError("wrong wkb format: " + WKB);
    }

    transform(EWKB.begin(), EWKB.end(), EWKB.begin(), ::toupper);
}

template<typename SRID_Type>
std::string Point<SRID_Type>::AsEWKT() const {
    std::string EWKT;
    std::stringstream ioss;
    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(Point_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();

    return EWKT;
}

template<typename SRID_Type>
std::string Point<SRID_Type>::ToString() const {
    return AsEWKB();
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
    || (std::is_same<SRID_Type, Wsg84>::value && srid != SRID::WSG84)) {
        throw InputError("template srid dismatch with input srid");
    }

    if (construct_type == 0) {
        if (!Endian(content))
            WkbEndianTransfer(content);
        byte_vector wkb_;
        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
            throw InputError("wrong wkb format: " + content);
        }
        EWKB = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;
        try {
            bg::read_wkt(content, line_);
        } catch(const std::exception &ex) {
            throw InputError("wrong wkt format" + std::string(ex.what()));
        }

        if (!bg::write_wkb(line_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }
        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = SetExtension(EWKB, GetSrid());
    }
    transform(EWKB.begin(), EWKB.end(), EWKB.begin(), ::toupper);
}

template<typename SRID_Type>
LineString<SRID_Type>::LineString(const std::string& EWKB_)
: SpatialBase(ExtractSRID(EWKB_), ExtractType(EWKB_)) {
    SRID srid = ExtractSRID(EWKB_);
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wsg84>::value && srid != SRID::WSG84)) {
        throw InputError("template srid dismatch with input srid");
    }

    if (!Endian(EWKB_))
        EWKB = EwkbEndianTransfer(EWKB_);
    else
        EWKB = EWKB_;
    std::string WKB = EWKB.substr(0, 10) + EWKB.substr(18);
    WKB[8] = '0';
    byte_vector wkb_;

    if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
        throw InputError("wrong wkb format: " + WKB);
    }
}

template<typename SRID_Type>
std::string LineString<SRID_Type>::AsEWKT() const {
    std::string EWKT;
    std::stringstream ioss;

    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(line_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();

    return EWKT;
}

template<typename SRID_Type>
std::string LineString<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
bool LineString<SRID_Type>::operator==(const LineString<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template<typename SRID_Type>
Polygon<SRID_Type>::Polygon(SRID srid, SpatialType type, int construct_type, std::string& content)
: SpatialBase(srid, type) {
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wsg84>::value && srid != SRID::WSG84)) {
        throw InputError("template srid dismatch with input srid");
    }

    if (construct_type == 0) {
        if (!Endian(content))
            WkbEndianTransfer(content);
        byte_vector wkb_;
        bg::hex2wkb(content, std::back_inserter(wkb_));
        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), Polygon_)) {
            throw InputError("wrong wkb format: " + content);
        }

        EWKB = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;

        try {
            bg::read_wkt(content, Polygon_);
        } catch (const std::exception &ex) {
            throw InputError("wrong wkt format" + std::string(ex.what()));
        }

        if (!bg::write_wkb(Polygon_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }

        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = SetExtension(EWKB, GetSrid());
    }
    transform(EWKB.begin(), EWKB.end(), EWKB.begin(), ::toupper);
}

template<typename SRID_Type>
Polygon<SRID_Type>::Polygon(const std::string& EWKB_)
: SpatialBase(ExtractSRID(EWKB_), ExtractType(EWKB_)) {
    SRID srid = ExtractSRID(EWKB_);
    if ((std::is_same<SRID_Type, Cartesian>::value && srid != SRID::CARTESIAN)
    || (std::is_same<SRID_Type, Wsg84>::value && srid != SRID::WSG84)) {
        throw InputError("template srid dismatch with input srid");
    }

    if (!Endian(EWKB_))
        EWKB = EwkbEndianTransfer(EWKB_);
    else
        EWKB = EWKB_;
    std::string WKB = EWKB.substr(0, 10) + EWKB.substr(18);
    WKB[8] = '0';

    byte_vector wkb_;

    if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), Polygon_)) {
        throw InputError("wrong wkb format: " + WKB);
    }
}

template<typename SRID_Type>
std::string Polygon<SRID_Type>::AsEWKT() const {
    std::string EWKT;
    std::stringstream ioss;

    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(Polygon_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();

    return EWKT;
}

template<typename SRID_Type>
std::string Polygon<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
bool Polygon<SRID_Type>::operator==(const Polygon<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template class Point<Wsg84>;
template class Point<Cartesian>;
template class LineString<Wsg84>;
template class LineString<Cartesian>;
template class Polygon<Wsg84>;
template class Polygon<Cartesian>;
template class Spatial<Wsg84>;
template class Spatial<Cartesian>;

}  // namespace lgraph_api
