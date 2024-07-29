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

//
// Created by wt on 19-11-20.
//
#pragma once

#include <unordered_map>
#include "core/data_type.h"
#include "cypher/cypher_exception.h"

namespace cypher {

struct FieldData {
    typedef std::unordered_map<std::string, cypher::FieldData> CYPHER_FIELD_DATA_MAP;
    typedef std::vector<cypher::FieldData> CYPHER_FIELD_DATA_LIST;
    // TODO(lingsu) : a default state should be added
    enum FieldType { SCALAR, ARRAY, MAP } type;

    lgraph::FieldData scalar;
    CYPHER_FIELD_DATA_LIST* array = nullptr;
    CYPHER_FIELD_DATA_MAP* map = nullptr;

    FieldData() : type(SCALAR) {}

    explicit FieldData(bool rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(int8_t rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(int16_t rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(int32_t rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(int64_t rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(float rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(double rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::Date& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::DateTime& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const std::string& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(std::string&& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const char* rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const char* rhs, size_t s) : type(SCALAR), scalar(rhs, s) {}

    explicit FieldData(const lgraph::PointCartesian& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::PointWgs84& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::LineStringCartesian& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::LineStringWgs84& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::PolygonCartesian& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::PolygonWgs84& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::SpatialCartesian& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const lgraph::SpatialWgs84& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(const ::lgraph::FieldData& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldData(::lgraph::FieldData&& rhs) : type(SCALAR), scalar(std::move(rhs)) {}

    explicit FieldData(const std::vector<::lgraph::FieldData>& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST();
        for (auto& item : rhs) {
            array->emplace_back(item);
        }
    }


    explicit FieldData(const CYPHER_FIELD_DATA_LIST& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST(rhs);
    }

    explicit FieldData(std::vector<::lgraph::FieldData>&& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST();
        for (auto& item : rhs) {
            array->emplace_back(std::move(item));
        }
    }


    explicit FieldData(CYPHER_FIELD_DATA_LIST&& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST(std::move(rhs));
    }

    explicit FieldData(const std::unordered_map<std::string, lgraph::FieldData>& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP();
        for (auto& kv : rhs) {
            map->emplace(kv);
        }
    }
    explicit FieldData(const CYPHER_FIELD_DATA_MAP& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP(rhs);
    }

    explicit FieldData(std::unordered_map<std::string, lgraph::FieldData>&& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP();
        for (auto& kv : rhs) {
            map->emplace(std::move(kv));
        }
    }

    explicit FieldData(CYPHER_FIELD_DATA_MAP&& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP(std::move(rhs));
    }

        ~FieldData() {
        switch (type) {
        case ARRAY:
            delete array;
            break;
        case MAP:
            delete map;
            break;
        case SCALAR:
            break;
        }
    }

    FieldData(const FieldData& rhs) {
        type = rhs.type;
        switch (type) {
        case ARRAY:
            array = new CYPHER_FIELD_DATA_LIST(*rhs.array);
            break;
        case MAP:
            map = new CYPHER_FIELD_DATA_MAP(*rhs.map);
            break;
        case SCALAR:
            scalar = rhs.scalar;
            break;
        }
    }

    FieldData(FieldData&& rhs) {
        type = rhs.type;
        switch (type) {
        case ARRAY:
            array = rhs.array;
            break;
        case MAP:
            map = rhs.map;
            break;
        case SCALAR:
            scalar = std::move(rhs.scalar);
            break;
        }
        // TODO(lingsu) : rhs should return to the default state
        rhs.type = SCALAR;
    }

    FieldData& operator=(const ::lgraph::FieldData& rhs) {
        switch (type) {
        case ARRAY:
            delete array;
            break;
        case MAP:
            delete map;
            break;
        case SCALAR:
            break;
        }
        type = SCALAR;
        scalar = rhs;
        return *this;
    }

    FieldData& operator=(::lgraph::FieldData&& rhs) {
        switch (type) {
        case ARRAY:
            delete array;
            break;
        case MAP:
            delete map;
            break;
        case SCALAR:
            break;
        }
        type = SCALAR;
        scalar = std::move(rhs);
        return *this;
    }

    FieldData& operator=(const ::cypher::FieldData& rhs) {
        if (this == &rhs) return *this;
        switch (type) {
        case ARRAY:
            delete array;
            break;
        case MAP:
            delete map;
            break;
        case SCALAR:
            break;
        }

        type = rhs.type;

        switch (type) {
        case ARRAY:
            array = new CYPHER_FIELD_DATA_LIST(*rhs.array);
            break;
        case MAP:
            map = new CYPHER_FIELD_DATA_MAP(*rhs.map);
            break;
        case SCALAR:
            scalar = rhs.scalar;
            break;
        }
        return *this;
    }

    FieldData& operator=(::cypher::FieldData&& rhs) {
        if (this == &rhs) return *this;
        switch (type) {
        case ARRAY:
            delete array;
            array = nullptr;
            break;
        case MAP:
            delete map;
            map = nullptr;
            break;
        case SCALAR:
            break;
        }
        type = rhs.type;
        switch (type) {
        case ARRAY:
            std::swap(array, rhs.array);
            break;
        case MAP:
            std::swap(map, rhs.map);
            break;
        case SCALAR:
            scalar = std::move(rhs.scalar);
            break;
        }
        // TODO(lingsu) : rhs should return to the default state
        rhs.type = SCALAR;
        return *this;
    }

    bool operator==(const FieldData& rhs) const {
        if (type != rhs.type)
            throw std::runtime_error("Unable to compare between different field data types");
        switch (type) {
        case ARRAY:
            CYPHER_TODO();
        case MAP:
            CYPHER_TODO();
        case SCALAR:
            return scalar == rhs.scalar;
        }
        CYPHER_TODO();
        return false;
    }

    bool operator!=(const FieldData& rhs) const { return !(*this == rhs); }

    bool operator>(const FieldData& rhs) const {
        if (type != rhs.type)
            throw std::runtime_error("Unable to compare between different field data types");
        switch (type) {
        case ARRAY:
            CYPHER_TODO();
        case MAP:
            CYPHER_TODO();
        case SCALAR:
            return scalar > rhs.scalar;
        }
        CYPHER_TODO();
        return false;
    }

    bool operator>=(const FieldData& rhs) const {
        if (type != rhs.type)
            throw std::runtime_error("Unable to compare between different field data types");
        switch (type) {
        case ARRAY:
            CYPHER_TODO();
        case MAP:
            CYPHER_TODO();
        case SCALAR:
            return scalar >= rhs.scalar;
        }
        CYPHER_TODO();
        return false;
    }

    bool operator<(const FieldData& rhs) const { return !(*this >= rhs); }

    bool operator<=(const FieldData& rhs) const { return !(*this > rhs); }

    bool EqualNull() const {
        switch (type) {
        case ARRAY:
            return !array || array->empty();
        case MAP:
            return !map || map->empty();
        case SCALAR:
            return scalar.is_null();
        }
        CYPHER_TODO();
        return false;
    }

    bool IsNull() const { return type == SCALAR && scalar.is_null(); }

    bool IsBool() const { return type == SCALAR && scalar.type == lgraph::FieldType::BOOL; }

    bool IsInteger() const {
        return type == SCALAR && scalar.type >= lgraph::FieldType::INT8 &&
               scalar.type <= lgraph::FieldType::INT64;
    }

    bool IsReal() const {
        return type == SCALAR && (scalar.type == lgraph::FieldType::DOUBLE ||
                                  scalar.type == lgraph::FieldType::FLOAT);
    }

    bool IsString() const { return type == SCALAR && scalar.type == lgraph::FieldType::STRING; }

    bool IsPoint() const { return type == SCALAR && scalar.type == lgraph::FieldType::POINT; }

    bool IsLineString() const {
        return type == SCALAR && scalar.type == lgraph::FieldType::LINESTRING;
    }

    bool IsPolygon() const { return type == SCALAR && scalar.type == lgraph::FieldType::POLYGON; }

    bool IsSpatial() const {
        return (IsPoint() || IsLineString() || IsPolygon()) ||
               (type == SCALAR && scalar.type == lgraph::FieldType::SPATIAL);
    }

    bool IsArray() const { return type == ARRAY; }

    static FieldData Array(size_t n) { return FieldData(std::vector<::lgraph::FieldData>(n)); }

    bool IsMap() const { return type == MAP; }

    std::string ToString(const std::string& null_value = "NUL") const {
        switch (type) {
        case ARRAY: {
            std::string str("[");
            for (auto& s : *array) str.append(s.ToString(null_value)).append(",");
            if (str.size() > 1) str.pop_back();
            str.append("]");
            return str;
        }
        case MAP: {
            std::string str("{");
            for (auto& pair : *map) {
                str.append(pair.first).append(":").append(pair.second.ToString(null_value));
                str.append(",");
            }
            if (str.size() > 1) str.pop_back();
            str.append("}");
            return str;
        }
        case SCALAR:
            return scalar.ToString(null_value);
        }
        throw std::runtime_error("internal error: unhandled type: " + std::to_string((int)type));
    }
    inline bool AsBool() const {
        return scalar.AsBool();
    }

    inline int8_t AsInt8() const {
        return scalar.AsInt8();
    }

    inline int16_t AsInt16() const {
        return scalar.AsInt16();
    }

    inline int32_t AsInt32() const {
        return scalar.AsInt32();
    }

    inline int64_t AsInt64() const {
        return scalar.AsInt64();
    }

    inline float AsFloat() const {
        return scalar.AsFloat();
    }

    inline double AsDouble() const {
        return scalar.AsDouble();
    }

    inline lgraph::Date AsDate() const {
        return scalar.AsDate();
    }

    inline lgraph::DateTime AsDateTime() const {
        return scalar.AsDateTime();
    }

    inline std::string AsString() const {
        return scalar.AsString();
    }

    inline std::string AsBlob() const {
        return scalar.AsBlob();
    }

    inline std::string AsBase64Blob() const {
        return scalar.AsBase64Blob();
    }

    inline lgraph::PointWgs84 AsWgsPoint() const {
        return scalar.AsWgsPoint();
    }

    inline lgraph::PointCartesian AsCartesianPoint() const {
        return scalar.AsCartesianPoint();
    }

    inline lgraph::LineStringWgs84 AsWgsLineString()
        const {
        return scalar.AsWgsLineString();
    }

    inline lgraph::LineStringCartesian AsCartesianLineString()
        const {
        return scalar.AsCartesianLineString();
    }

    inline lgraph::PolygonWgs84 AsWgsPolygon() const {
        return scalar.AsWgsPolygon();
    }

    inline lgraph::PolygonCartesian AsCartesianPolygon() const {
        return scalar.AsCartesianPolygon();
    }

    inline std::vector<lgraph::FieldData> AsConstantArray() const {
        if (type == ARRAY) {
            std::vector<lgraph::FieldData> ans;
            for (auto& item : *array) {
                if (item.IsMap() || item.IsArray()) {
                    throw std::bad_cast();
                }
                ans.emplace_back(item.scalar);
            }
            return ans;
        }
        throw std::bad_cast();
    }
};

enum ExpandTowards {
    FORWARD,        // outgoing expand
    REVERSED,       // incoming expand
    BIDIRECTIONAL,  // two way expand
};

namespace _detail {
static inline std::string EdgeUid2String(const ::lgraph::EdgeUid& e) {
    // WARNING: Five tuple is temporary solution.
    //          You should rewrite the func when the finally plan have be proposed.
    return fma_common::StringFormatter::Format("{}_{}_{}_{}_{}", e.src, e.dst, e.lid, e.tid, e.eid);
}

[[maybe_unused]]
static ::lgraph::EdgeUid ExtractEdgeUid(const std::string& s) {
    std::stringstream ss(s);
    std::vector<int64_t> ns;
    int64_t n;
    char c;
    while (ss >> n) {
        ns.emplace_back(n);
        if (!(ss >> c)) break;
    }
    // WARNING: Five tuple is temporary solution.
    //          You should rewrite the line or func when the finally plan have be proposed.
    // TODO(heng)
    if (ns.size() != 5) THROW_CODE(InputError, "Failed extract EdgeUid from string.");
    return {ns[0], ns[1], static_cast<::lgraph::LabelId>(ns[2]), ns[3], ns[4]};
}

}  // namespace _detail

}  // namespace cypher
