#pragma once

#include <boost/geometry.hpp>
#include <boost/geometry/multi/io/wkt/wkt.hpp> 
#include <boost/geometry/extensions/gis/io/wkb/write_wkb.hpp>
#include <boost/geometry/extensions/gis/io/wkb/read_wkb.hpp>
#include <boost/geometry/extensions/gis/io/wkb/utility.hpp>
#include <iostream>
#include <vector>

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

std::string dec2hex(SRID srid_type, size_t width) {
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

    std::string s_hex_;
    int i = 6;
    while(i >= 0) {
        s_hex_ += s_hex.substr(i, 2);
        i -= 2;
    }

    transform(s_hex_.begin(), s_hex_.end(), s_hex_.begin(), ::toupper);

    return s_hex_;
}

std::string set_extension(std::string EWKB, SRID srid_type) {
    EWKB[8] = '2';
    std::string s_hex = dec2hex(srid_type, 8);

    EWKB.insert(10, s_hex);
    return EWKB;
}


template<typename SRID_Type>
class Spatial {
    // std::unique_ptr<GeoBase> geo_;
    std::unique_ptr<point<SRID_Type>> point_;
    std::unique_ptr<linestring<SRID_Type>> line_;
    std::unique_ptr<polygon<SRID_Type>> polygon_;
    // std::unique_ptr<point_impl<bg::cs::cartesian>> point_impl_;
    
    SpatialType type_;

 public:
    /**
     * @brief   
     *
     * @param       srid 
     * @param [out] type   
     *
     * 
     */
    Spatial(SRID srid, SpatialType type, int construct_type, std::string content);
    ~Spatial() {}

    // std::vector<std::string> ListTypes();
    // std::vector<int> ListSrids();
    std::string AsEWKB();
    std::string AsEWKT(); 
    std::unique_ptr<point<SRID_Type>>& GetPoint() {
        return point_;
    }

    std::unique_ptr<linestring<SRID_Type>>& GetLine() {
        return line_;
    }

    std::unique_ptr<polygon<SRID_Type>>& GetPolygon() {
        return polygon_;
    }

    SpatialType GetType() {
        return type_;
    }
    
    // std::unique_ptr<GeoBase>& SelectGeo();
    // double Distance(geography<T>& other);
}; 

class SpatialBase {
    SRID srid_;
    SpatialType type_;

 public:
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

    bg::model::point<double, 2, SRID_Type> GetSpatialData() {
        return point_;
    }
};

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

    // double Distance(std::unique_ptr<point<T>>& other);
};

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
}