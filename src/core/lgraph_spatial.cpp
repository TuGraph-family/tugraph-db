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
    std::string endian = EWKB.substr(0, 2);
    if (endian == "01") {
        return true;
    }
    return false;
}

void EndianTansfer(std::string& input) {
    size_t size = input.size();
    int i = size -2;
    std::string output;
    while (i >= 0) {
        output += input.substr(i, 2);
        i -= 2;
    }
    input = output;
}

std::string Srid2Hex(SRID srid_type, size_t width, bool endian) {
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

    if (endian)
        EndianTansfer(s_hex);

    return s_hex;
}

std::string SetExtension(const std::string& WKB, SRID srid_type) {
    bool endian = Endian(WKB);
    std::string EWKB = WKB;

    EWKB[endian ? 8 : 6] = '2';
    std::string s_hex = Srid2Hex(srid_type, 8, endian);
    EWKB.insert(10, s_hex);

    return EWKB;
}

SRID ExtractSRID(const std::string& EWKB) {
    if (EWKB.size() < 50)
        throw InputError("wrong EWKB type");

    std::string srid = EWKB.substr(10, 8);
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
    if (EWKB.size() < 50)
        throw InputError("wrong EWKB type");

    std::string type = EWKB.substr(2, 4);
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
    typedef bg::model::point<double, 2, SRID_Type> point;
    switch (type) {
        case SpatialType::NUL:
            return false;
        case SpatialType::POINT: {
            point point_;
            byte_vector wkb_;

            if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), point_)) {
                return false;
            }
            return true;
        }

        case SpatialType::LINESTRING: {
            bg::model::linestring<point> line_;
            byte_vector wkb_;

            if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
                return false;
            }
            return true;
        }

        case SpatialType::POLYGON: {
            bg::model::polygon<point> polygon_;
            byte_vector wkb_;

            if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
            !bg::read_wkb(wkb_.begin(), wkb_.end(), polygon_)) {
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
            point_.reset(new point<SRID_Type>(srid, type, construct_type, content));
            break;
        case SpatialType::LINESTRING:
            line_.reset(new linestring<SRID_Type>(srid, type, construct_type, content));
            break;
        case SpatialType::POLYGON:
            polygon_.reset(new polygon<SRID_Type>(srid, type, construct_type, content));
            break;
    }
}

template<typename SRID_Type>
Spatial<SRID_Type>::Spatial(const std::string& EWKB) {
    type_ = ExtractType(EWKB);
    SRID srid = ExtractSRID(EWKB);
    std::string WKB = EWKB.substr(0, 10) + EWKB.substr(18);
    WKB[8] = '0';
    WKB[9] = '0';

    switch (type_) {
        case SpatialType::NUL:
            throw InputError("invalid spatial type");
        case SpatialType::POINT:
            point_.reset(new point<SRID_Type>(srid, type_, 0, WKB));
            break;
        case SpatialType::LINESTRING:
            line_.reset(new linestring<SRID_Type>(srid, type_, 0, WKB));
            break;
        case SpatialType::POLYGON:
            polygon_.reset(new polygon<SRID_Type>(srid, type_, 0, WKB));
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
bool Spatial<SRID_Type>::operator==(const Spatial<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template<typename SRID_Type>
point<SRID_Type>::point(SRID srid, SpatialType type, int construct_type, std::string content)
: SpatialBase(srid, type) {
    if (construct_type == 0) {
        byte_vector wkb_;

        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), point_)) {
            throw InputError("wrong wkb format: " + content);
        }
        EWKB = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;

        // 如果WKT格式错误，这里会抛出异常; boost::exception_detail;
        try {
            bg::read_wkt(content, point_);
        } catch (const std::exception &ex) {
            throw InputError("wrong wkt format:" + std::string(ex.what()));
        }

        if (!bg::write_wkb(point_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }

        // 这里要不要判断异常?
        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = SetExtension(EWKB, GetSrid());
    }

    transform(EWKB.begin(), EWKB.end(), EWKB.begin(), ::toupper);
}

template<typename SRID_Type>
point<SRID_Type>::point(const std::string& EWKB_)
: SpatialBase(ExtractSRID(EWKB_), ExtractType(EWKB_)) {
    std::string WKB = EWKB_.substr(0, 10) + EWKB_.substr(18);
    EWKB = EWKB_;
    WKB[8] = WKB[9] = '0';

    byte_vector wkb_;

    if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), point_)) {
        throw InputError("wrong wkb format: " + WKB);
    }
}

template<typename SRID_Type>
std::string point<SRID_Type>::AsEWKT() const {
    std::string EWKT;
    std::stringstream ioss;
    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(point_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();

    return EWKT;
}

template<typename SRID_Type>
std::string point<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
bool point<SRID_Type>::operator==(const point<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template<typename SRID_Type>
linestring<SRID_Type>::linestring(SRID srid, SpatialType type,
int construct_type, std::string content)
: SpatialBase(srid, type) {
    if (construct_type == 0) {
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
linestring<SRID_Type>::linestring(const std::string& EWKB_)
: SpatialBase(ExtractSRID(EWKB_), ExtractType(EWKB_)) {
    std::string WKB = EWKB_.substr(0, 10) + EWKB_.substr(18);
    EWKB = EWKB_;
    WKB[8] = WKB[9] = '0';

    byte_vector wkb_;

    if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
        throw InputError("wrong wkb format: " + WKB);
    }
}

template<typename SRID_Type>
std::string linestring<SRID_Type>::AsEWKT() const {
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
std::string linestring<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
bool linestring<SRID_Type>::operator==(const linestring<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template<typename SRID_Type>
polygon<SRID_Type>::polygon(SRID srid, SpatialType type, int construct_type, std::string content)
: SpatialBase(srid, type) {
    if (construct_type == 0) {
        byte_vector wkb_;
        bg::hex2wkb(content, std::back_inserter(wkb_));
        if (!bg::hex2wkb(content, std::back_inserter(wkb_)) ||
        !bg::read_wkb(wkb_.begin(), wkb_.end(), polygon_)) {
            throw InputError("wrong wkb format: " + content);
        }

        EWKB = SetExtension(content, GetSrid());
    }

    if (construct_type == 1) {
        std::string wkb_out;

        try {
            bg::read_wkt(content, polygon_);
        } catch (const std::exception &ex) {
            throw InputError("wrong wkt format" + std::string(ex.what()));
        }

        if (!bg::write_wkb(polygon_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }

        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = SetExtension(EWKB, GetSrid());
    }
    transform(EWKB.begin(), EWKB.end(), EWKB.begin(), ::toupper);
}

template<typename SRID_Type>
polygon<SRID_Type>::polygon(const std::string& EWKB_)
: SpatialBase(ExtractSRID(EWKB_), ExtractType(EWKB_)) {
    std::string WKB = EWKB_.substr(0, 10) + EWKB_.substr(18);
    EWKB = EWKB_;
    WKB[8] = WKB[9] = '0';

    byte_vector wkb_;

    if (!bg::hex2wkb(WKB, std::back_inserter(wkb_)) ||
    !bg::read_wkb(wkb_.begin(), wkb_.end(), polygon_)) {
        throw InputError("wrong wkb format: " + WKB);
    }
}

template<typename SRID_Type>
std::string polygon<SRID_Type>::AsEWKT() const {
    std::string EWKT;
    std::stringstream ioss;

    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(polygon_) << std::endl;

    std::string tmp;
    while (ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();

    return EWKT;
}

template<typename SRID_Type>
std::string polygon<SRID_Type>::ToString() const {
    return AsEWKB();
}

template<typename SRID_Type>
bool polygon<SRID_Type>::operator==(const polygon<SRID_Type> &other) {
    return AsEWKB() == other.AsEWKB();
}

template class point<Wsg84>;
template class point<Cartesian>;
template class linestring<Wsg84>;
template class linestring<Cartesian>;
template class polygon<Wsg84>;
template class polygon<Cartesian>;
template class Spatial<Wsg84>;
template class Spatial<Cartesian>;

}  // namespace lgraph_api
