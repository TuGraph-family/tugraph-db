#include "lgraph/lgraph_spatial.h"
#include "lgraph/lgraph_exceptions.h"

namespace lgraph_api {


template<typename SRID_Type>
Spatial<SRID_Type>::Spatial(SRID srid, SpatialType type, int construct_type, std::string content) {
    type_ = type;
    switch(type) {
        case SpatialType::NUL: 
            break;
        case SpatialType::POINT: {
            point_.reset(new point<SRID_Type>(srid, type, construct_type, content));
            break;            
        }

        case SpatialType::LINESTRING: {
            line_.reset(new linestring<SRID_Type>(srid, type, construct_type, content));
            break;    
        }

        case SpatialType::POLYGON: {
            polygon_.reset(new polygon<SRID_Type>(srid, type, construct_type, content));
            break;    
        }

        default:
            throw std::runtime_error("Unknown Spatial Type");
    }
}


template<typename SRID_Type>
std::string Spatial<SRID_Type>::AsEWKB() {
    switch(type_) {
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
std::string Spatial<SRID_Type>::AsEWKT() {
    switch(type_) {
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
point<SRID_Type>::point(SRID srid, SpatialType type, int construct_type, std::string content) 
: SpatialBase(srid, type) {
    if(construct_type == 0) {
        byte_vector wkb_;
        
        if(!bg::hex2wkb(content, std::back_inserter(wkb_)) || !bg::read_wkb(wkb_.begin(), wkb_.end(), point_)) {
            throw InputError("wrong wkb format: " + content);
        }
        EWKB = set_extension(content, GetSrid());
    }

    if(construct_type == 1) {
        std::string wkb_out;

        // 如果WKT格式错误，这里会抛出异常; boost::exception_detail;
        try {
            bg::read_wkt(content, point_);
        } catch (const std::exception &ex) {
            //std::cout << ex.what() << std::endl;
            throw InputError("wrong wkt format:" + std::string(ex.what()));
        }
            
        if(!bg::write_wkb(point_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }

        // 这里要不要判断异常?
        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = set_extension(EWKB, GetSrid()); 
    }
}

template<typename SRID_Type>
std::string point<SRID_Type>::AsEWKT() {
    std::string EWKT;
    std::stringstream ioss;
    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(point_) << std::endl;

    std::string tmp;
    while(ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();
    
    return EWKT;
}

template<typename SRID_Type>
linestring<SRID_Type>::linestring(SRID srid, SpatialType type, int construct_type, std::string content) 
: SpatialBase(srid, type) {
    if(construct_type == 0) {
        byte_vector wkb_;
        if(!bg::hex2wkb(content, std::back_inserter(wkb_)) || !bg::read_wkb(wkb_.begin(), wkb_.end(), line_)) {
            throw InputError("wrong wkb format: " + content);
        }
        //EWKB = content;
        EWKB = set_extension(content, GetSrid());
    }

    if(construct_type == 1) {
        std::string wkb_out;
        try {
            bg::read_wkt(content, line_);
        } catch(const std::exception &ex) {
            //std::cout << ex.what() << std::endl;
            throw InputError("wrong wkt format" + std::string(ex.what()));
        }
        
        if(!bg::write_wkb(line_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }
        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = set_extension(EWKB, GetSrid()); 
    }
}

template<typename SRID_Type>
std::string linestring<SRID_Type>::AsEWKT() {
    std::string EWKT;
    std::stringstream ioss;

    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(line_) << std::endl;

    std::string tmp;
    while(ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();
    
    return EWKT;
}

template<typename SRID_Type>
polygon<SRID_Type>::polygon(SRID srid, SpatialType type, int construct_type, std::string content) 
: SpatialBase(srid, type) {
    if(construct_type == 0) {
        byte_vector wkb_;
        bg::hex2wkb(content, std::back_inserter(wkb_));
        if(!bg::hex2wkb(content, std::back_inserter(wkb_)) || !bg::read_wkb(wkb_.begin(), wkb_.end(), polygon_)) {
            throw InputError("wrong wkb format: " + content);
        }

        EWKB = set_extension(content, GetSrid());
    }

    if(construct_type == 1) {
        std::string wkb_out;

        try {
            bg::read_wkt(content, polygon_);
        } catch(const std::exception &ex) {
            // std::cout << ex.what() << std::endl;
            throw InputError("wrong wkt format" + std::string(ex.what()));
        }
        
        if(!bg::write_wkb(polygon_, std::back_inserter(wkb_out))) {
            throw InputError("wrong wkt format: " + content);
        }

        bg::wkb2hex(wkb_out.begin(), wkb_out.end(), EWKB);
        EWKB = set_extension(EWKB, GetSrid()); 
    }
}

template<typename SRID_Type>
std::string polygon<SRID_Type>::AsEWKT() {
    std::string EWKT;
    std::stringstream ioss;

    ioss << "SRID=" << static_cast<int>(GetSrid()) << ";" << bg::wkt(polygon_) << std::endl;

    std::string tmp;
    while(ioss >> tmp) {
        EWKT += tmp;
        EWKT += ' ';
    }
    EWKT.pop_back();
    
    return EWKT;
}

template class point<Wsg84>;
template class point<Cartesian>;
template class linestring<Wsg84>;
template class linestring<Cartesian>;
template class polygon<Wsg84>;
template class polygon<Cartesian>;
template class Spatial<Wsg84>;
template class Spatial<Cartesian>;

}