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

static inline std::string dec2hex(SRID srid_type, size_t width) {
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

static inline std::string set_extension(std::string EWKB, SRID srid_type) {
    EWKB[8] = '2';
    std::string s_hex = dec2hex(srid_type, 8);

    EWKB.insert(10, s_hex);
    return EWKB;
}

// return true if big endian;
static inline bool Endian(std::string EWKB) {
    std::string endian = EWKB.substr(0, 2);
    if(endian == "01") return true;  
    return false;
}

// transform the little endian to big endian;
static inline void little2big(const std::string& little, std::string& big) {
    size_t size = little.size();
    int i = size -2;
    while(i >= 0) {
        big += little.substr(i, 2);
        i -= 2;
    } 
}

static inline SRID ExtractSRID(std::string EWKB) {
    std::string srid = EWKB.substr(10, 8);
    if(Endian(EWKB)) {
        std::string rsrid;
        little2big(srid, rsrid);
        srid = rsrid;
    }

    std::stringstream ioss(srid); 
    int Srid;
    ioss >> std::hex >> Srid;
    switch(Srid) {
        case 0:
            return SRID::NUL;
        case 4326:
            return SRID::WSG84;
        case 7203:
            return SRID::CARTESIAN;
        default:
            throw std::runtime_error("wrong srid!");
    }
}

static inline SpatialType ExtractType(std::string EWKB) {
    std::string type = EWKB.substr(2, 4);
    if(Endian(EWKB)) {
        std::string rtype;
        little2big(type, rtype);
        type = rtype;
    }

    int Type = 0;
    std::stringstream ioss(type);
    ioss >> Type;

    switch(Type) {
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
class Spatial {
    std::unique_ptr<point<SRID_Type>> point_;
    std::unique_ptr<linestring<SRID_Type>> line_;
    std::unique_ptr<polygon<SRID_Type>> polygon_;
    
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

    // construct from EWKB
    Spatial(std::string EWKB);
    
    ~Spatial() {}

    // std::vector<std::string> ListTypes();
    // std::vector<int> ListSrids();
    std::string AsEWKB() const;
    std::string AsEWKT() const;  
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

    bool operator==(const Spatial<SRID_Type>& other);
    
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