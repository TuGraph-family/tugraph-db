//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <typeinfo>
#include <cassert>
#include <map>
#include <set>
#include <any>
#include <stdexcept>
#include <cstdint>
#include <string>
#include <utility>
#include <vector>

#include "lgraph/base64_encode.h"
#include "lgraph/lgraph_date_time.h"
#include "lgraph/lgraph_spatial.h"

namespace lgraph_api {

/**
 * @brief   Access level a user or role has on a graph. NONE: no permission. READ: read-only, no
 *          write access. WRITE: can read and write vertex and edge, but cannot change meta data
 *          such as schema or access. FULL: full access, can modify schema, grant access to other
 *          users, or even delete this graph.
 */
enum class AccessLevel {
    // DO NOT change the order of the levels.
    // They are used to compare levels.
    NONE = 0,
    READ = 1,
    WRITE = 2,
    FULL = 3
};

[[maybe_unused]]
inline static std::string to_string(const AccessLevel& v) {
    switch (v) {
        case AccessLevel::NONE:    return "NONE";
        case AccessLevel::READ:    return "READ";
        case AccessLevel::WRITE:   return "WRITE";
        case AccessLevel::FULL:    return "FULL";
        default:   throw std::runtime_error("Unknown AccessLevel");
    }
}

enum class FieldAccessLevel {
    // DO NOT change the order of the levels.
    // They are used to compare different levels.
    NONE = 0,
    READ = 1,
    WRITE = 2
};

[[maybe_unused]]
inline static std::string to_string(const FieldAccessLevel& v) {
    switch (v) {
        case FieldAccessLevel::NONE:    return "NONE";
        case FieldAccessLevel::READ:    return "READ";
        case FieldAccessLevel::WRITE:   return "WRITE";
        default:   throw std::runtime_error("Unknown AccessLevel");
    }
}

enum class GraphQueryType {
    CYPHER = 0,
    GQL = 1
};

[[maybe_unused]]
inline static std::string to_string(const GraphQueryType& v) {
    switch (v) {
        case GraphQueryType::CYPHER:    return "CYPHER";
        case GraphQueryType::GQL:       return "GQL";
        default:   throw std::runtime_error("Unknown GraphQueryType");
    }
}

/**
 * @brief  Edge constraints type define
 */
typedef std::vector<std::pair<std::string, std::string>> EdgeConstraints;

/**
 * @brief  Label options, base class, define some common fields and methods
 */
struct LabelOptions {
    // store property data in detached model
    // Default: false
    bool detach_property = false;
    // use fast alter schema format
    // Default: false
    bool fast_alter_schema = false;
    virtual std::string to_string() const = 0;
    virtual void clear() = 0;
    virtual ~LabelOptions() {}
};

/**
 * @brief  Edge label options, contain fields only edge have
 */
struct EdgeOptions : LabelOptions {
    // edge constraints, specify the start and end vertex label in edge direction
    // Default: empty, means no constraints
    EdgeConstraints edge_constraints;
    // edge temporal field, edge will be stored in the order of this field
    // Default: empty
    std::string temporal_field;
    // order of edge temporal field
    // Default: ASC
    enum class TemporalFieldOrder {
        ASC = 0,
        DESC = 1,
    } temporal_field_order = TemporalFieldOrder::ASC;

    inline static std::string to_string(const TemporalFieldOrder& v) {
        switch (v) {
        case TemporalFieldOrder::ASC:
            return "ASC";
        case TemporalFieldOrder::DESC:
            return "DESC";
        default:
            throw std::runtime_error("Unknown TemporalFieldOrder");
        }
    }

    EdgeOptions() = default;
    explicit EdgeOptions(const EdgeConstraints& edge_constraints)
        : edge_constraints(edge_constraints) {}

    std::string to_string() const {
        std::string ret;
        std::string constraints;
        for (size_t i = 0; i < edge_constraints.size(); i++) {
            constraints += edge_constraints[i].first + " -> " + edge_constraints[i].second;
            if (i != edge_constraints.size()-1) {
                constraints += ", ";
            }
        }
        constraints = "[" + constraints + "]";

        return "detach_property: " + std::to_string(detach_property) +
               ", edge_constraints: " + constraints + ", temporal_field: " + temporal_field +
               ", temporal_field_order: " + to_string(temporal_field_order);
    }
    void clear() {
        detach_property = false;
        edge_constraints.clear();
        temporal_field.clear();
    }
};

/**
 * @brief  Vertex label options, contain fields only vertex have
 */
struct VertexOptions : public LabelOptions {
    // vertex primary field, must be set
    std::string primary_field;

    VertexOptions() = default;
    explicit VertexOptions(const std::string& primary_field) : primary_field(primary_field) {}
    std::string to_string() const {
        return "detach_property: " + std::to_string(detach_property) +
               ", primary_field: " + primary_field;
    }
    void clear() {
        detach_property = false;
        primary_field.clear();
    }
};

/** @brief   Field and value types. */
enum FieldType {
    NUL = 0,          // NULL value, used to represent an empty value, can not be used as field type
    BOOL = 1,         // Boolean value, TRUE or FALSE
    INT8 = 2,         // 8-bit signed integer
    INT16 = 3,        // 16-bit signed integer
    INT32 = 4,        // 32-bit signed integer
    INT64 = 5,        // 64-bit signed integer
    FLOAT = 6,        // 32-bit IEEE 754 floating Point number
    DOUBLE = 7,       // 64-bit IEEE 754 floating Point number
    DATE = 8,         // Date ranging from 0/1/1 to 9999/12/31
    DATETIME = 9,     // DateTime ranging from 0000-01-01 00:00:00.000000 to
                      // 9999-12-31 23:59:59.999999
    STRING = 10,      // string type, in unicode encoding
    BLOB = 11,        // binary byte array
    POINT = 12,       // Point type of spatial data
    LINESTRING = 13,  // LineString type of spatial data
    POLYGON = 14,     // Polygon type of spatial data
    SPATIAL = 15,     // spatial data, it's now unused but may be used in the future.
    FLOAT_VECTOR = 16  // float vector
};

inline bool const is_integer_type(FieldType type) {
    switch (type) {
    case INT8:
    case INT16:
    case INT32:
    case INT64:
        return true;
    default:
        return false;
    }
}

inline bool const is_float_type(FieldType type) {
    switch (type) {
    case FLOAT:
    case DOUBLE:
        return true;
    default:
        return false;
    }
}

/**
 * @brief   Get the name of the given FieldType.
 *
 * @exception   std::runtime_error  when an unrecognizable FieldType is given.
 *
 * @param   v   A FieldType.
 *
 * @returns Name of the given FieldType.
 */
inline const std::string to_string(FieldType v) {
    switch (v) {
    case NUL:
        return "NUL";
    case BOOL:
        return "BOOL";
    case INT8:
        return "INT8";
    case INT16:
        return "INT16";
    case INT32:
        return "INT32";
    case INT64:
        return "INT64";
    case FLOAT:
        return "FLOAT";
    case DOUBLE:
        return "DOUBLE";
    case DATE:
        return "DATE";
    case DATETIME:
        return "DATETIME";
    case STRING:
        return "STRING";
    case BLOB:
        return "BLOB";
    case POINT:
        return "POINT";
    case LINESTRING:
        return "LINESTRING";
    case POLYGON:
        return "POLYGON";
    case SPATIAL:
        return "SPATIAL";
    case FLOAT_VECTOR:
        return "FLOAT_VECTOR";
    default:
        throw std::runtime_error("Unknown Field Type");
    }
}

/**
   * @brief a type of value used in result entry and parameter in procedure or plugin signature
   * @param INTEGER
   * @param FLOAT
   * @param DOUBLE
   * @param BOOLEAN
   * @param STRING
   * @param MAP <string, FieldData>
   * @param NODE VertexIterator, VertexId
   * @param RELATIONSHIP InEdgeIterator || OutEdgeIterator, EdgeUid
   * @param PATH lgraph_api::Path
   * @param LIST <string, FieldData>
   * @param ANY like Object in Java,
   * its procedure author's responsibility to check the underlying concrete type
   * whether valid in runtime.
 */
enum class LGraphType : uint16_t {
    NUL = 0x0,
    INTEGER = 0x11,
    FLOAT = 0x12,
    DOUBLE = 0x13,
    BOOLEAN = 0x14,
    STRING = 0x15,
    NODE = 0x21,
    RELATIONSHIP = 0x22,
    PATH = 0x23,
    LIST = 0x41,
    MAP = 0x42,
    ANY = 0x80
};

inline auto LGraphTypeIsField(LGraphType type) -> bool {
    return (uint16_t(type) & 0x10) != 0;
}

inline auto LGraphTypeIsGraphElement(LGraphType type) -> bool {
    return (uint16_t(type) & 0x20) != 0;
}

inline auto LGraphTypeIsCollection(LGraphType type) -> bool {
    return (uint16_t(type) & 0x40) != 0;
}

inline auto LGraphTypeIsAny(LGraphType type) -> bool {
    return type == LGraphType::ANY;
}

inline const std::string to_string(LGraphType type) {
    switch (type) {
    case LGraphType::NUL:
        return "NUL";
    case LGraphType::INTEGER:
        return "INTEGER";
    case LGraphType::FLOAT:
        return "FLOAT";
    case LGraphType::DOUBLE:
        return "DOUBLE";
    case LGraphType::BOOLEAN:
        return "BOOLEAN";
    case LGraphType::STRING:
        return "STRING";
    case LGraphType::NODE:
        return "NODE";
    case LGraphType::RELATIONSHIP:
        return "RELATIONSHIP";
    case LGraphType::PATH:
        return "PATH";
    case LGraphType::LIST:
        return "LIST";
    case LGraphType::MAP:
        return "MAP";
    case LGraphType::ANY:
        return "ANY";
    default:
        throw std::runtime_error("Unknown LGraph Type");
    }
}

/**
 * @brief The parameter of procedure/plugin
 */
struct Parameter {
    std::string name;  ///> name of the parameter
    int index;         ///> index of the parameter list in which the parameter stay
    LGraphType type;   ///> type of the parameter
};

/**
 * @brief The Specification of procedure/plugin signature
 *
 * @example
 *  plugin.cpp.customShortestPath(a, b) -> length, nodeIds
 *  input_list: a, b
 *  results: length, nodeIds
 */
struct SigSpec {
    std::vector<Parameter> input_list;   ///> input parameter list
    std::vector<Parameter> result_list;  ///> return parameter list
};

/** @brief   Type of code given when loading a new plugin. */
enum PluginCodeType {
    PY = 1,   // .py for python source code
    SO = 2,   // compiled library for cpp plugin
    CPP = 3,  // single C++ source file
    ZIP = 4   // zip package of source file
};

/** @brief   Get the name of plugin code types. */
inline std::string PluginCodeTypeStr(PluginCodeType code_type) {
    switch (code_type) {
    case PluginCodeType::PY:
        return "py";
    case PluginCodeType::SO:
        return "so";
    case PluginCodeType::CPP:
        return "cpp";
    case PluginCodeType::ZIP:
        return "zip";
    default:
        return "unknown";
    }
}

/** @brief   A class that represents variant type. */
struct FieldData {
    FieldData() {
        type = FieldType::NUL;
        data.int64 = 0;
    }

    explicit FieldData(bool b) {
        type = FieldType::BOOL;
        data.boolean = b;
    }

    explicit FieldData(int8_t integer) {
        type = FieldType::INT8;
        data.int8 = integer;
    }

    explicit FieldData(int16_t integer) {
        type = FieldType::INT16;
        data.int16 = integer;
    }

    explicit FieldData(int32_t integer) {
        type = FieldType::INT32;
        data.int32 = integer;
    }

    explicit FieldData(int64_t integer) {
        type = FieldType::INT64;
        data.int64 = integer;
    }

    explicit FieldData(float real) {
        type = FieldType::FLOAT;
        data.sp = real;
    }

    explicit FieldData(double real) {
        type = FieldType::DOUBLE;
        data.dp = real;
    }

    explicit FieldData(const Date& d) {
        type = FieldType::DATE;
        data.int32 = d.DaysSinceEpoch();
    }

    explicit FieldData(const DateTime& d) {
        type = FieldType::DATETIME;
        data.int64 = d.MicroSecondsSinceEpoch();
    }

    explicit FieldData(const std::string& buf) {
        type = FieldType::STRING;
        data.buf = new std::string(buf);
    }

    explicit FieldData(std::string&& str) {
        type = FieldType::STRING;
        data.buf = new std::string(std::move(str));
    }

    explicit FieldData(const char* buf) {
        type = FieldType::STRING;
        data.buf = new std::string(buf);
    }

    explicit FieldData(const char* buf, size_t s) {
        type = FieldType::STRING;
        data.buf = new std::string(buf, buf + s);
    }

    explicit FieldData(const Point<Cartesian>& p) {
        type = FieldType::POINT;
        data.buf = new std::string(p.AsEWKB());
    }

    explicit FieldData(const Point<Wgs84>& p) {
        type = FieldType::POINT;
        data.buf = new std::string(p.AsEWKB());
    }

    explicit FieldData(const LineString<Cartesian>& l) {
        type = FieldType::LINESTRING;
        data.buf = new std::string(l.AsEWKB());
    }

    explicit FieldData(const LineString<Wgs84>& l) {
        type = FieldType::LINESTRING;
        data.buf = new std::string(l.AsEWKB());
    }

    explicit FieldData(const Polygon<Cartesian>& p) {
        type = FieldType::POLYGON;
        data.buf = new std::string(p.AsEWKB());
    }

    explicit FieldData(const Polygon<Wgs84>& p) {
        type = FieldType::POLYGON;
        data.buf = new std::string(p.AsEWKB());
    }

    explicit FieldData(const Spatial<Cartesian>& s) {
        type = FieldType::SPATIAL;
        data.buf = new std::string(s.AsEWKB());
    }

    explicit FieldData(const Spatial<Wgs84>& s) {
        type = FieldType::SPATIAL;
        data.buf = new std::string(s.AsEWKB());
    }

    explicit FieldData(const std::vector<float>& fv) {
        type = FieldType::FLOAT_VECTOR;
        data.vp = new std::vector<float>(fv);
    }

    explicit FieldData(std::vector<float>&& fv) {
        type = FieldType::FLOAT_VECTOR;
        data.vp = new std::vector<float>(std::move(fv));
    }

    ~FieldData() {
        if (IsBufType(type)) delete data.buf;
        if (type == FieldType::FLOAT_VECTOR) delete data.vp;
    }

    FieldData(const FieldData& rhs) {
        type = rhs.type;
        if (IsBufType(rhs.type)) {
            data.buf = new std::string(*rhs.data.buf);
        } else if (rhs.type != FieldType::FLOAT_VECTOR) {
            // the integer type must have the biggest size, see static_assertion below
            data.int64 = rhs.data.int64;
        } else {
            data.vp = new std::vector<float>(*rhs.data.vp);
        }
    }

    FieldData(FieldData&& rhs) {
        type = rhs.type;
        // the integer type must have the biggest size, see static_assertion below
        data.int64 = rhs.data.int64;
        rhs.type = FieldType::NUL;
    }

    inline FieldData& operator=(const FieldData& rhs) {
        if (this == &rhs) return *this;
        if (IsBufType(type)) delete data.buf;
        type = rhs.type;
        if (IsBufType(rhs.type)) {
            data.buf = new std::string(*rhs.data.buf);
        } else if (rhs.type != FieldType::FLOAT_VECTOR) {
            // the integer type must have the biggest size, see static_assertion below
            data.int64 = rhs.data.int64;
        } else {
            data.vp = new std::vector<float>(*rhs.data.vp);
        }
        return *this;
    }

    inline FieldData& operator=(FieldData&& rhs) {
        if (this == &rhs) return *this;
        if (IsBufType(type)) delete data.buf;
        type = rhs.type;
        // the integer type must have the biggest size, see static_assertion below
        data.int64 = rhs.data.int64;
        rhs.type = FieldType::NUL;
        return *this;
    }

    // =================================
    // static constructors

    static inline FieldData Bool(bool b) { return FieldData(b); }
    static inline FieldData Int8(int8_t i) { return FieldData(i); }
    static inline FieldData Int16(int16_t i) { return FieldData(i); }
    static inline FieldData Int32(int32_t i) { return FieldData(i); }
    static inline FieldData Int64(int64_t i) { return FieldData(i); }
    static inline FieldData Float(float d) { return FieldData(d); }
    static inline FieldData Double(double d) { return FieldData(d); }
    static inline FieldData Date(const std::string& str) {
        return FieldData(::lgraph_api::Date(str));
    }
    static inline FieldData Date(const ::lgraph_api::Date& d) { return FieldData(d); }
    static inline FieldData DateTime(const std::string& str) {
        return FieldData(::lgraph_api::DateTime(str));
    }
    static inline FieldData DateTime(const ::lgraph_api::DateTime& d) { return FieldData(d); }
    static inline FieldData String(const std::string& str) { return FieldData(str); }
    static inline FieldData String(std::string&& str) { return FieldData(std::move(str)); }
    static inline FieldData String(const char* str) { return FieldData(str); }
    static inline FieldData String(const char* p, size_t s) { return FieldData(p, s); }

    static inline FieldData Point(const ::lgraph_api::Point<Cartesian>& p) {
    return FieldData(p); }
    static inline FieldData Point(const ::lgraph_api::Point<Wgs84>& p) {return FieldData(p); }
    static inline FieldData Point(const std::string& str) {
        switch (::lgraph_api::ExtractSRID(str)) {
            case ::lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "Unsupported SRID!");
            case ::lgraph_api::SRID::CARTESIAN:
                return FieldData(::lgraph_api::Point<Cartesian>(str));
            case ::lgraph_api::SRID::WGS84:
                return FieldData(::lgraph_api::Point<Wgs84>(str));
            default:
                THROW_CODE(InputError, "Unsupported SRID!");
        }
    }

    static inline FieldData LineString(const ::lgraph_api::LineString<Cartesian>& l) {
        return FieldData(l); }
    static inline FieldData LineString(const ::lgraph_api::LineString<Wgs84>& l) {
        return FieldData(l); }
    static inline FieldData LineString(const std::string& str) {
        switch (::lgraph_api::ExtractSRID(str)) {
            case ::lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "Unsupported SRID!");
            case ::lgraph_api::SRID::CARTESIAN:
                return FieldData(::lgraph_api::LineString<Cartesian>(str));
            case ::lgraph_api::SRID::WGS84:
                return FieldData(::lgraph_api::LineString<Wgs84>(str));
            default:
                THROW_CODE(InputError, "Unsupported SRID!");
        }
    }

    static inline FieldData Polygon(const ::lgraph_api::Polygon<Cartesian>& p) {
    return FieldData(p); }
    static inline FieldData Polygon(const ::lgraph_api::Polygon<Wgs84>& p) {return FieldData(p); }
    static inline FieldData Polygon(const std::string& str) {
        switch (::lgraph_api::ExtractSRID(str)) {
            case ::lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "Unsupported SRID!");
            case ::lgraph_api::SRID::CARTESIAN:
                return FieldData(::lgraph_api::Polygon<Cartesian>(str));
            case ::lgraph_api::SRID::WGS84:
                return FieldData(::lgraph_api::Polygon<Wgs84>(str));
            default:
                THROW_CODE(InputError, "Unsupported SRID!");
        }
    }

    static inline FieldData Spatial(const ::lgraph_api::Spatial<Cartesian>& s) {
    return FieldData(s); }
    static inline FieldData Spatial(const ::lgraph_api::Spatial<Wgs84>& s) {return FieldData(s); }
    static inline FieldData Spatial(const std::string& str) {
        switch (::lgraph_api::ExtractSRID(str)) {
            case ::lgraph_api::SRID::NUL:
                THROW_CODE(InputError, "Unsupported SRID!");
            case ::lgraph_api::SRID::CARTESIAN:
                return FieldData(::lgraph_api::Spatial<Cartesian>(str));
            case ::lgraph_api::SRID::WGS84:
                return FieldData(::lgraph_api::Spatial<Wgs84>(str));
            default:
                THROW_CODE(InputError, "Unsupported SRID!");
        }
    }

    static inline FieldData FloatVector(const std::vector<float>& fv) { return FieldData(fv); }

    //-------------------------
    // Constructs blobs.
    // A blob is a byte array. It can be used to store binary data such as images, html
    // files, ect.
    static inline FieldData Blob(const std::string& str) {
        FieldData d;
        d.type = FieldType::BLOB;
        d.data.buf = new std::string(str);
        return d;
    }

    static inline FieldData Blob(std::string&& str) {
        FieldData d;
        d.type = FieldType::BLOB;
        d.data.buf = new std::string(std::move(str));
        return d;
    }

    /** @brief   Constructs a Blob from vector of uint8_t, treated as byte array. */
    static inline FieldData Blob(const std::vector<uint8_t>& str) {
        FieldData d;
        d.type = FieldType::BLOB;
        d.data.buf = new std::string(str.begin(), str.end());
        return d;
    }

    /** @brief   Constructs a BLOB from Base64 encoded string. */
    static inline FieldData BlobFromBase64(const std::string& base64_encoded) {
        return Blob(::lgraph_api::base64::Decode(base64_encoded));
    }

    //=================================
    // accessors

    /**
     * @brief   Access the FieldData as int64. Valid only when the FieldData is of INT8, INT16,
     *          INT32, or INT64 types.
     *
     * @exception   std::bad_cast   Thrown when the FieldData is not of int types.
     *
     * @returns An int64_t.
     */
    inline int64_t integer() const {
        switch (type) {
        case FieldType::NUL:
        case FieldType::BOOL:
            throw std::bad_cast();
        case FieldType::INT8:
            return static_cast<int64_t>(data.int8);
        case FieldType::INT16:
            return static_cast<int64_t>(data.int16);
        case FieldType::INT32:
            return static_cast<int64_t>(data.int32);
        case FieldType::INT64:
            return data.int64;
        case FieldType::FLOAT:
        case FieldType::DOUBLE:
        case FieldType::DATE:
        case FieldType::DATETIME:
        case FieldType::STRING:
        case FieldType::BLOB:
        case FieldType::POINT:
        case FieldType::LINESTRING:
        case FieldType::POLYGON:
        case FieldType::SPATIAL:
        case FieldType::FLOAT_VECTOR:
            throw std::bad_cast();
        }
        return 0;
    }

    /**
     * @brief   Access the FieldData as a double. The FieldData must be of FLOAT or DOUBLE types.
     *
     * @exception   std::bad_cast   Thrown if the FieldData is not of FLOAT or DOUBLE types.
     *
     * @returns A double.
     */
    inline double real() const {
        switch (type) {
        case FieldType::NUL:
        case FieldType::BOOL:
        case FieldType::INT8:
        case FieldType::INT16:
        case FieldType::INT32:
        case FieldType::INT64:
            throw std::bad_cast();
        case FieldType::FLOAT:
            return static_cast<double>(data.sp);
        case FieldType::DOUBLE:
            return data.dp;
        case FieldType::DATE:
        case FieldType::DATETIME:
        case FieldType::STRING:
        case FieldType::BLOB:
        case FieldType::POINT:
        case FieldType::LINESTRING:
        case FieldType::POLYGON:
        case FieldType::SPATIAL:
        case FieldType::FLOAT_VECTOR:
            throw std::bad_cast();
        }
        return 0.;
    }

    /**
     * @brief   Access the FieldData as std::string. Valid only for STRING, BLOB and SPATIAL. BLOB data is
     *          returned as-is, since std::string can also hold byte array.
     *
     * @exception   std::bad_cast   Thrown when a bad cast error condition occurs.
     *
     * @returns A reference to a const std::string.
     */
    inline const std::string& string() const {
        switch (type) {
        case FieldType::STRING:
        case FieldType::BLOB:
        case FieldType::POINT:
        case FieldType::LINESTRING:
        case FieldType::POLYGON:
        case FieldType::SPATIAL:
            return *data.buf;
        default:
            {
                throw std::bad_cast();
                static const std::string _;
                return _;
            }
        }
    }

    // ------------------------------
    // Accessors that do not convert. std::bad_cast is thrown if type does not match exactly.

    inline bool AsBool() const {
        if (type == FieldType::BOOL) return data.boolean;
        throw std::bad_cast();
    }

    inline int8_t AsInt8() const {
        if (type == FieldType::INT8) return data.int8;
        throw std::bad_cast();
    }

    inline int16_t AsInt16() const {
        if (type == FieldType::INT16) return data.int16;
        throw std::bad_cast();
    }

    inline int32_t AsInt32() const {
        if (type == FieldType::INT32) return data.int32;
        throw std::bad_cast();
    }

    inline int64_t AsInt64() const {
        if (type == FieldType::INT64) return data.int64;
        throw std::bad_cast();
    }

    inline float AsFloat() const {
        if (type == FieldType::FLOAT) return data.sp;
        throw std::bad_cast();
    }

    inline double AsDouble() const {
        if (type == FieldType::DOUBLE) return data.dp;
        throw std::bad_cast();
    }

    inline ::lgraph_api::Date AsDate() const {
        if (type == FieldType::DATE) return ::lgraph_api::Date(data.int32);
        throw std::bad_cast();
    }

    inline ::lgraph_api::DateTime AsDateTime() const {
        if (type == FieldType::DATETIME) return ::lgraph_api::DateTime(data.int64);
        throw std::bad_cast();
    }

    inline std::string AsString() const {
        if (type == FieldType::STRING) return *data.buf;
        if (type == FieldType::NUL) return "";
        throw std::bad_cast();
    }

    inline std::string AsBlob() const {
        if (type == FieldType::BLOB) return *data.buf;
        if (type == FieldType::NUL) return "";
        throw std::bad_cast();
    }

    inline std::string AsBase64Blob() const {
        if (type == FieldType::BLOB) return ::lgraph_api::base64::Encode(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::SRID GetSRID() const {
        if (type >= FieldType::POINT && type <= FieldType::SPATIAL)
            return ::lgraph_api::ExtractSRID(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::Point<::lgraph_api::Wgs84> AsWgsPoint() const {
        if (type == FieldType::POINT) return ::lgraph_api::Point
        <::lgraph_api::Wgs84>(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::Point<::lgraph_api::Cartesian> AsCartesianPoint() const {
        if (type == FieldType::POINT) return ::lgraph_api::Point
        <::lgraph_api::Cartesian>(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::LineString<::lgraph_api::Wgs84> AsWgsLineString()
    const {
        if (type == FieldType::LINESTRING) return ::lgraph_api::LineString
        <::lgraph_api::Wgs84>(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::LineString<::lgraph_api::Cartesian> AsCartesianLineString()
    const {
        if (type == FieldType::LINESTRING) return ::lgraph_api::LineString
        <::lgraph_api::Cartesian>(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::Polygon<::lgraph_api::Wgs84> AsWgsPolygon() const {
        if (type == FieldType::POLYGON) return ::lgraph_api::Polygon
        <::lgraph_api::Wgs84>(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::Polygon<::lgraph_api::Cartesian> AsCartesianPolygon() const {
        if (type == FieldType::POLYGON) return ::lgraph_api::Polygon
        <::lgraph_api::Cartesian>(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::Spatial<::lgraph_api::Wgs84> AsWgsSpatial()
    const {
        if (IsSpatial()) return ::lgraph_api::Spatial
        <::lgraph_api::Wgs84>(*data.buf);
        throw std::bad_cast();
    }

    inline ::lgraph_api::Spatial<::lgraph_api::Cartesian> AsCartesianSpatial()
    const {
        if (IsSpatial()) return ::lgraph_api::Spatial
        <::lgraph_api::Cartesian>(*data.buf);
        throw std::bad_cast();
    }

    inline std::vector<float> AsFloatVector() const {
        if (type == FieldType::FLOAT_VECTOR) return *data.vp;
        throw std::bad_cast();
    }

    std::any ToBolt() const;

    /** @brief   Get string representation of this FieldData. */
    inline std::string ToString(const std::string& null_value = "NUL") const {
        switch (type) {
        case FieldType::NUL:
            return null_value;
        case FieldType::BOOL:
            return data.boolean ? "true" : "false";
        case FieldType::INT8:
            return std::to_string(data.int8);
        case FieldType::INT16:
            return std::to_string(data.int16);
        case FieldType::INT32:
            return std::to_string(data.int32);
        case FieldType::INT64:
            return std::to_string(data.int64);
        case FieldType::FLOAT:
            return std::to_string(data.sp);
        case FieldType::DOUBLE:
            return std::to_string(data.dp);
        case FieldType::DATE:
            return ::lgraph_api::Date(data.int32).ToString();
        case FieldType::DATETIME:
            return ::lgraph_api::DateTime(data.int64).ToString();
        case FieldType::STRING:
            return *data.buf;
        case FieldType::BLOB:
            return ::lgraph_api::base64::Encode(*data.buf);
        case FieldType::POINT:
        case FieldType::LINESTRING:
        case FieldType::POLYGON:
        case FieldType::SPATIAL:
            return *data.buf;
        case FieldType::FLOAT_VECTOR:
            {
                std::string vec_str;
                for (float num : *data.vp) {
                    if (num > 999999) {
                        vec_str += std::to_string(num).substr(0, 7);
                    } else {
                        vec_str += std::to_string(num).substr(0, 8);
                    }
                    vec_str += ',';
                }
                if (!vec_str.empty()) {
                    vec_str.pop_back();
                }
                return vec_str;
            }
        }
        throw std::runtime_error("Unhandled data type, probably corrupted data.");
        return "";
    }

    /**************************************************************************
     *           Comparators. Two FieldData objects can be compared iff
     *           1. They are of the same type, or
     *           2. they are both numeric types
     *           In case 1, the underlying values are compared directly. In case 2, integer
     *           values are converted to int64_t if compared with another integer, or to double
     *           if compared with a floating-Point number.
     */

    inline bool operator==(const FieldData& rhs) const {
        if (type == FieldType::NUL && rhs.type == FieldType::NUL) return true;
        if (type == FieldType::NUL || rhs.type == FieldType::NUL) return false;
        if (type == FieldType::FLOAT_VECTOR || rhs.type == FieldType::FLOAT_VECTOR) return false;
        if (type == rhs.type) {
            switch (type) {
            case FieldType::NUL:
                throw std::runtime_error("");
            case FieldType::BOOL:
                return data.int8 == rhs.data.int8;
            case FieldType::INT8:
                return data.int8 == rhs.data.int8;
            case FieldType::INT16:
                return data.int16 == rhs.data.int16;
            case FieldType::INT32:
                return data.int32 == rhs.data.int32;
            case FieldType::INT64:
                return data.int64 == rhs.data.int64;
            case FieldType::FLOAT:
                return std::abs(data.sp - rhs.data.sp) < std::numeric_limits<float>::epsilon();
            case FieldType::DOUBLE:
                return std::abs(data.dp - rhs.data.dp) < std::numeric_limits<double>::epsilon();
            case FieldType::DATE:
                return data.int32 == rhs.data.int32;
            case FieldType::DATETIME:
                return data.int64 == rhs.data.int64;
            case FieldType::STRING:
            case FieldType::BLOB:
            case FieldType::POINT:
            case FieldType::LINESTRING:
            case FieldType::POLYGON:
            case FieldType::SPATIAL:
                return *data.buf == *rhs.data.buf;
            case FieldType::FLOAT_VECTOR:
                throw std::runtime_error("Float vector data are not comparable now.");
            }
            throw std::runtime_error("Unhandled data type, probably corrupted data.");
        } else if (IsInteger(type) && IsInteger(rhs.type)) {
            return integer() == rhs.integer();
        } else if (IsInteger(type) && IsReal(rhs.type)) {
            return static_cast<double>(integer()) == rhs.real();
        } else if (IsReal(type) && IsInteger(rhs.type)) {
            return real() == static_cast<double>(rhs.integer());
        } else if (IsReal(type) && IsReal(rhs.type)) {
            return real() == rhs.real();
        }
        throw std::runtime_error("Unable to compare two FieldData with different types. " +
                                 lgraph_api::to_string(type) + " vs " +
                                 lgraph_api::to_string(rhs.type));
        return false;
    }

    inline bool operator!=(const FieldData& rhs) const { return !(*this == rhs); }

    inline bool operator>(const FieldData& rhs) const {
        if (type == FieldType::NUL) return false;
        if (rhs.type == FieldType::NUL) return true;
        if (type == FieldType::FLOAT_VECTOR || rhs.type == FieldType::FLOAT_VECTOR) return false;
        if (type == rhs.type) {
            switch (type) {
            case FieldType::NUL:
                throw std::runtime_error("");
            case FieldType::BOOL:
                return data.int8 > rhs.data.int8;
            case FieldType::INT8:
                return data.int8 > rhs.data.int8;
            case FieldType::INT16:
                return data.int16 > rhs.data.int16;
            case FieldType::INT32:
                return data.int32 > rhs.data.int32;
            case FieldType::INT64:
                return data.int64 > rhs.data.int64;
            case FieldType::FLOAT:
                return data.sp > rhs.data.sp;
            case FieldType::DOUBLE:
                return data.dp > rhs.data.dp;
            case FieldType::DATE:
                return data.int32 > rhs.data.int32;
            case FieldType::DATETIME:
                return data.int64 > rhs.data.int64;
            case FieldType::STRING:
            case FieldType::BLOB:
                return *data.buf > *rhs.data.buf;
            case FieldType::POINT:
            case FieldType::LINESTRING:
            case FieldType::POLYGON:
            case FieldType::SPATIAL:
                throw std::runtime_error("Spatial data are not comparable now.");
            case FieldType::FLOAT_VECTOR:
                throw std::runtime_error("Float vector data are not comparable now.");
            }
            throw std::runtime_error("Unhandled data type, probably corrupted data.");
        } else if (IsInteger(type) && IsInteger(rhs.type)) {
            return integer() > rhs.integer();
        } else if (IsInteger(type) && IsReal(rhs.type)) {
            return static_cast<double>(integer()) > rhs.real();
        } else if (IsReal(type) && IsInteger(rhs.type)) {
            return real() > static_cast<double>(rhs.integer());
        } else if (IsReal(type) && IsReal(rhs.type)) {
            return real() > rhs.real();
        }
        throw std::runtime_error("Unable to compare two FieldData with different types. " +
                                 lgraph_api::to_string(type) + " vs " +
                                 lgraph_api::to_string(rhs.type));
        return false;
    }

    inline bool operator>=(const FieldData& rhs) const {
        if (rhs.type == FieldType::NUL) return true;
        if (type == FieldType::NUL) return false;
        if (type == FieldType::FLOAT_VECTOR || rhs.type == FieldType::FLOAT_VECTOR) return false;
        if (type == rhs.type) {
            switch (type) {
            case FieldType::NUL:
                throw std::runtime_error("");
            case FieldType::BOOL:
                return data.int8 >= rhs.data.int8;
            case FieldType::INT8:
                return data.int8 >= rhs.data.int8;
            case FieldType::INT16:
                return data.int16 >= rhs.data.int16;
            case FieldType::INT32:
                return data.int32 >= rhs.data.int32;
            case FieldType::INT64:
                return data.int64 >= rhs.data.int64;
            case FieldType::FLOAT:
                return data.sp >= rhs.data.sp;
            case FieldType::DOUBLE:
                return data.dp >= rhs.data.dp;
            case FieldType::DATE:
                return data.int32 >= rhs.data.int32;
            case FieldType::DATETIME:
                return data.int64 >= rhs.data.int64;
            case FieldType::STRING:
            case FieldType::BLOB:
                return *data.buf >= *rhs.data.buf;
            case FieldType::POINT:
            case FieldType::LINESTRING:
            case FieldType::POLYGON:
            case FieldType::SPATIAL:
                throw std::runtime_error("Spatial data are not comparable now.");
            case FieldType::FLOAT_VECTOR:
                throw std::runtime_error("Float vector data are not comparable now.");
            }
            throw std::runtime_error("Unhandled data type, probably corrupted data.");
        } else if (IsInteger(type) && IsInteger(rhs.type)) {
            return integer() >= rhs.integer();
        } else if (IsInteger(type) && IsReal(rhs.type)) {
            return static_cast<double>(integer()) >= rhs.real();
        } else if (IsReal(type) && IsInteger(rhs.type)) {
            return real() >= static_cast<double>(rhs.integer());
        } else if (IsReal(type) && IsReal(rhs.type)) {
            return real() >= rhs.real();
        }
        throw std::runtime_error("Unable to compare two FieldData with different types. " +
                                 lgraph_api::to_string(type) + " vs " +
                                 lgraph_api::to_string(rhs.type));
        return false;
    }

    inline bool operator<(const FieldData& rhs) const { return !(*this >= rhs); }

    inline bool operator<=(const FieldData& rhs) const { return !(*this > rhs); }

    FieldType type;

    FieldType GetType() const { return type; }

    union {
        bool boolean;
        int8_t int8;
        int16_t int16;
        int32_t int32;
        int64_t int64;
        float sp;
        double dp;
        std::string* buf;
        std::vector<float>* vp;
    } data;

    inline bool is_null() const { return type == FieldType::NUL; }

    inline bool is_buf() const { return IsBufType(type); }

    inline bool is_empty_buf() const { return IsBufType(type) && data.buf->empty(); }

    /** @brief   Query if this object is null */
    bool IsNull() const { return type == FieldType::NUL; }

    /** @brief   Query if this object is bool */
    bool IsBool() const { return type == FieldType::BOOL; }

    /** @brief   Query if this object is BLOB */
    bool IsBlob() const { return type == FieldType::BLOB; }

    /** @brief   Query if this object is string */
    bool IsString() const { return type == FieldType::STRING; }

    /** @brief   Query if this object is INT8 */
    bool IsInt8() const { return type == FieldType::INT8; }

    /** @brief   Query if this object is INT16 */
    bool IsInt16() const { return type == FieldType::INT16; }

    /** @brief   Query if this object is INT32 */
    bool IsInt32() const { return type == FieldType::INT32; }

    /** @brief   Query if this object is INT64 */
    bool IsInt64() const { return type == FieldType::INT64; }

    /** @brief   Is this a INT8, INT16, INT32 or INT64? */
    bool IsInteger() const { return IsInteger(type); }

    /** @brief   Query if this object is float */
    bool IsFloat() const { return type == FieldType::FLOAT; }

    /** @brief   Query if this object is double */
    bool IsDouble() const { return type == FieldType::DOUBLE; }

    /** @brief   Is this a FLOAT or DOUBLE? */
    bool IsReal() const { return IsReal(type); }

    /** @brief   Query if this object is date */
    bool IsDate() const { return type == FieldType::DATE; }

    /** @brief   Query if this object is date time */
    bool IsDateTime() const { return type == FieldType::DATETIME; }

    /** @brief   Query if this object is Point */
    bool IsPoint() const { return type == FieldType::POINT; }

    /** @brief   Query if this object is LineString */
    bool IsLineString() const { return type == FieldType::LINESTRING; }

    /** @brief   Query if this object is Polygon*/
    bool IsPolygon() const { return type == FieldType::POLYGON; }

    /** @brief   Query if this object is spatial*/
    bool IsSpatial() const { return type == FieldType::SPATIAL || IsPoint() || IsLineString()
    || IsPolygon(); }

    /** @brief   Query if this object is float vector*/
    bool IsFloatVector() const { return type == FieldType::FLOAT_VECTOR; }

    struct Hash {
        size_t operator()(const FieldData& fd) const {
            switch (fd.type) {
            case FieldType::NUL:
                return 0;
            case FieldType::BOOL:
                return std::hash<bool>()(fd.AsBool());
            case FieldType::INT8:
                return std::hash<int8_t>()(fd.AsInt8());
            case FieldType::INT16:
                return std::hash<int16_t>()(fd.AsInt16());
            case FieldType::INT32:
                return std::hash<int32_t>()(fd.AsInt32());
            case FieldType::INT64:
                return std::hash<int64_t>()(fd.AsInt64());
            case FieldType::FLOAT:
                return std::hash<float>()(fd.AsFloat());
            case FieldType::DOUBLE:
                return std::hash<double>()(fd.AsDouble());
            case FieldType::DATE:
                return std::hash<int32_t>()(fd.AsDate().DaysSinceEpoch());
            case FieldType::DATETIME:
                return std::hash<int64_t>()(fd.AsDateTime().MicroSecondsSinceEpoch());
            case FieldType::STRING:
                return std::hash<std::string>()(fd.AsString());
            case FieldType::BLOB:
                return std::hash<std::string>()(fd.AsBlob());
            case FieldType::POINT:
                {
                    switch (fd.GetSRID()) {
                    case ::lgraph_api::SRID::WGS84:
                        return std::hash<std::string>()(fd.AsWgsPoint().AsEWKB());
                    case ::lgraph_api::SRID::CARTESIAN:
                        return std::hash<std::string>()(fd.AsCartesianPoint().AsEWKB());
                    default:
                        THROW_CODE(InputError, "unsupported spatial srid");
                    }
                }
            case FieldType::LINESTRING:
                {
                    switch (fd.GetSRID()) {
                    case ::lgraph_api::SRID::WGS84:
                        return std::hash<std::string>()(fd.AsWgsLineString().AsEWKB());
                    case ::lgraph_api::SRID::CARTESIAN:
                        return std::hash<std::string>()(fd.AsCartesianLineString().AsEWKB());
                    default:
                        THROW_CODE(InputError, "unsupported spatial srid");
                    }
                }
            case FieldType::POLYGON:
                {
                    switch (fd.GetSRID()) {
                    case ::lgraph_api::SRID::WGS84:
                        return std::hash<std::string>()(fd.AsWgsPolygon().AsEWKB());
                    case ::lgraph_api::SRID::CARTESIAN:
                        return std::hash<std::string>()(fd.AsCartesianPolygon().AsEWKB());
                    default:
                        THROW_CODE(InputError, "unsupported spatial srid");
                    }
                }
            case FieldType::SPATIAL:
                {
                    switch (fd.GetSRID()) {
                    case ::lgraph_api::SRID::WGS84:
                        return std::hash<std::string>()(fd.AsWgsSpatial().AsEWKB());
                    case ::lgraph_api::SRID::CARTESIAN:
                        return std::hash<std::string>()(fd.AsCartesianSpatial().AsEWKB());
                    default:
                        THROW_CODE(InputError, "unsupported spatial srid");
                    }
                }
            default:
                throw std::runtime_error("Unhandled data type, probably corrupted data.");
            }
        }
    };

 private:
    /** @brief   Query if 't' is BLOB or STRING */
    static inline bool IsBufType(FieldType t) {
        return t >= FieldType::STRING && t < FieldType::FLOAT_VECTOR;
    }

    /** @brief   Query if 't' is INT8, 16, 32, or 64 */
    static inline bool IsInteger(FieldType t) {
        return t >= FieldType::INT8 && t <= FieldType::INT64;
    }

    /** @brief   Query if 't' is FLLOAT or DOUBLE */
    static inline bool IsReal(FieldType t) {
        return t == FieldType::DOUBLE || t == FieldType::FLOAT;
    }

    static_assert(sizeof(decltype(data.int64)) == sizeof(decltype(data.buf)) &&
                      sizeof(decltype(data.int64)) >= sizeof(decltype(data.dp)),
                  "sizeof int64_t is supposed to be equal to Pointer types");
};

/** @brief   Specification for a field. */
struct FieldSpec {
    /** @brief   name of the field */
    std::string name;
    /** @brief   type of that field */
    FieldType type;
    /** @brief   is this field optional? */
    bool optional;
    /** @brief   is this field deleted? */
    bool deleted;
    /** @brief   id of this field, starts from 0 */
    uint16_t id;
    /** @brief   the value of the field is set when it is created. */
    bool set_default_value;
    /** @brief  the default value when inserting data. */
    FieldData default_value;
    /** @brief   is set init value? */

    FieldSpec()
        : name(),
          type(FieldType::NUL),
          optional(false),
          deleted(false),
          id(0),
          set_default_value(false),
          default_value(FieldData()) {}

    /**
     * @brief   Constructor
     *
     * @param   n   Field name
     * @param   t   Field type
     * @param   nu  True if field is optional
     * @param   id  Field id
     * @param   dv Default value
     */
    FieldSpec(const std::string& n, FieldType t, bool nu)
        : name(n),
          type(t),
          optional(nu),
          deleted(false),
          id(0),
          set_default_value(false),
          default_value(FieldData()) {}
    FieldSpec(const std::string& n, FieldType t, bool nu, uint16_t id)
        : name(n),
          type(t),
          optional(nu),
          deleted(false),
          id(id),
          set_default_value(false),
          default_value(FieldData()) {}
    FieldSpec(std::string&& n, FieldType t, bool nu, uint16_t id)
        : name(std::move(n)),
          type(t),
          optional(nu),
          deleted(false),
          id(id),
          set_default_value(false),
          default_value(FieldData()) {}
    FieldSpec(const std::string& n, FieldType t, bool nu, uint16_t id, const FieldData& dv)
        : name(n),
          type(t),
          optional(nu),
          deleted(false),
          id(id),
          set_default_value(true),
          default_value(dv) {}
    FieldSpec(const FieldSpec& spec)
        : name(spec.name),
          type(spec.type),
          optional(spec.optional),
          deleted(spec.deleted),
          id(spec.id),
          set_default_value(spec.set_default_value),
          default_value(spec.default_value) {}

    inline bool operator==(const FieldSpec& rhs) const {
        return name == rhs.name && type == rhs.type && optional == rhs.optional &&
            deleted == rhs.deleted && id == rhs.id  &&
            set_default_value == rhs.set_default_value && default_value == rhs.default_value;
    }

    /** @brief   Get the string representation of the FieldSpec. */
    std::string ToString() const {
        return "lgraph_api::FieldSpec(name=[" + name + "],type=" + lgraph_api::to_string(type) +
               "),optional=" + std::to_string(optional) + ",fieldid=" + std::to_string(id) +
               ",isDeleted=" + std::to_string(deleted) +
               (set_default_value ? ",default_value=" + default_value.ToString() : "");
    }
};

/** @brief  index type*/
enum class IndexType {
    /** @brief this is not unique index */
    NonuniqueIndex = 0,
    /** @brief this is a global unique index */
    GlobalUniqueIndex = 1,
    /** @brief this is a pair unique index, for edge index only
     *  key of pair unique index is one of the follow case :
     *  if src_vid < dst_vid ,key is (index field value + src_vid + dst_vid)
     *  if src_vid > dst_vid ,key is (index field value + dst_vid + src_vid)
     * */
    PairUniqueIndex = 2
};

enum class CompositeIndexType {
    /** @brief this is unique composite index */
    UniqueIndex = 1,
    /** @brief this is not unique composite index */
    NonUniqueIndex = 2
};

/** @brief   An index specifier. */
struct IndexSpec {
    /** @brief   label name */
    std::string label;
    /** @brief   field name */
    std::string field;
    IndexType type;
};

/** @brief   A composite index specifier. */
struct CompositeIndexSpec {
    /** @brief   label name */
    std::string label;
    /** @brief   fields name */
    std::vector<std::string> fields;
    CompositeIndexType type;
};

struct VectorIndexSpec {
    std::string label;
    std::string field;
    std::string index_type;
    int dimension;
    std::string distance_type;
    int hnsw_m;
    int hnsw_ef_construction;
    int ivf_flat_nlist;
};

struct EdgeUid {
    EdgeUid() : src(0), dst(0), lid(0), tid(0), eid(0) {}
    EdgeUid(int64_t s, int64_t d, uint16_t l, int64_t t, int64_t e)
        : src(s), dst(d), lid(l), tid(t), eid(e) {}

    static inline EdgeUid AnyEdge() { return EdgeUid(); }

    /** @brief   source vertex id */
    int64_t src;
    /** @brief   destination vertex id */
    int64_t dst;
    /** @brief   label id */
    uint16_t lid;
    /** @brief   timestamp */
    int64_t tid;
    /** @brief   additional edge id to distinguish edges with the same tid */
    int64_t eid;

    /** @brief   Reverses side of this edge */
    void Reverse() { std::swap(src, dst); }

    inline bool operator==(const EdgeUid& rhs) const {
        return src == rhs.src && dst == rhs.dst && lid == rhs.lid && eid == rhs.eid &&
               tid == rhs.tid;
    }

    inline bool operator!=(const EdgeUid& rhs) const {
        return !this->operator==(rhs);
    }

    inline bool operator<(const EdgeUid& rhs) const {
        return src < rhs.src || (src == rhs.src && dst < rhs.dst) ||
               (src == rhs.src && dst == rhs.dst && lid < rhs.lid) ||
               (src == rhs.src && dst == rhs.dst && lid == rhs.lid && tid < rhs.tid) ||
               (src == rhs.src && dst == rhs.dst && lid == rhs.lid &&
                tid == rhs.tid && eid < rhs.eid);
    }

    inline bool operator>(const EdgeUid& rhs) const {
        return src > rhs.src || (src == rhs.src && dst > rhs.dst) ||
               (src == rhs.src && dst == rhs.dst && lid > rhs.lid) ||
               (src == rhs.src && dst == rhs.dst && lid == rhs.lid && tid > rhs.tid) ||
               (src == rhs.src && dst == rhs.dst && lid == rhs.lid &&
                tid == rhs.tid && eid > rhs.eid);
    }

    /** @brief  Get string representation of this object */
    std::string ToString() const {
        return std::to_string(src) + "_" + std::to_string(dst) + "_" + std::to_string(lid) + "_" +
               std::to_string(tid) + "_" + std::to_string(eid);
    }

    /** @brief   Comparator for EdgeUid of out-going edges. */
    struct OutEdgeSortOrder {
        inline bool operator()(const EdgeUid& lhs, const EdgeUid& rhs) const {
            if (lhs.src < rhs.src) return true;
            if (lhs.src > rhs.src) return false;
            if (lhs.lid < rhs.lid) return true;
            if (lhs.lid > rhs.lid) return false;
            if (lhs.tid < rhs.tid) return true;
            if (lhs.tid > rhs.tid) return false;
            if (lhs.dst < rhs.dst) return true;
            if (lhs.dst > rhs.dst) return false;
            if (lhs.eid < rhs.eid) return true;
            return false;
        }
    };

    /** @brief   Comparator for EdgeUid of in-coming edges. */
    struct InEdgeSortOrder {
        inline bool operator()(const EdgeUid& lhs, const EdgeUid& rhs) const {
            if (lhs.dst < rhs.dst) return true;
            if (lhs.dst > rhs.dst) return false;
            if (lhs.lid < rhs.lid) return true;
            if (lhs.lid > rhs.lid) return false;
            if (lhs.tid < rhs.tid) return true;
            if (lhs.tid > rhs.tid) return false;
            if (lhs.src < rhs.src) return true;
            if (lhs.src > rhs.src) return false;
            if (lhs.eid < rhs.eid) return true;
            return false;
        }
    };

    struct Hash {
        size_t operator()(const EdgeUid& edgeUid) const {
            size_t hashValue = 0;
            hashValue = std::hash<int64_t>()(edgeUid.eid);
            hashValue ^= std::hash<int64_t>()(edgeUid.dst) +
                        0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
            hashValue ^= std::hash<int64_t>()(edgeUid.lid) +
                        0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
            hashValue ^= std::hash<int64_t>()(edgeUid.src) +
                        0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
            hashValue ^= std::hash<int64_t>()(edgeUid.tid) +
                        0x9e3779b9 + (hashValue << 6) + (hashValue >> 2);
            return hashValue;
        }
    };
};

/** @brief   Information about the user. */
struct UserInfo {
    /** @brief   description of the user */
    std::string desc;
    /** @brief   roles of this user */
    std::set<std::string> roles;
    /** @brief   is this user disabled? */
    bool disabled = false;
    /** @brief   memory limit for this user */
    size_t memory_limit;
};

/** @brief   Information about the role. */
struct RoleInfo {
    /** @brief   description */
    std::string desc;
    /** @brief   access levels on different graphs */
    std::map<std::string, AccessLevel> graph_access;
    /** @brief   is this role disabled? */
    bool disabled = false;
};

}  // namespace lgraph_api
