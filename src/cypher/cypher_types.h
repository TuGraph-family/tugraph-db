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
#include "common/value.h"
#include "cypher/cypher_exception.h"

namespace cypher {

struct FieldDatav1 {
    typedef std::unordered_map<std::string, cypher::FieldDatav1> CYPHER_FIELD_DATA_MAP;
    typedef std::vector<cypher::FieldDatav1> CYPHER_FIELD_DATA_LIST;
    // TODO(lingsu) : a default state should be added
    enum FieldType { SCALAR, ARRAY, MAP } type;

    //lgraph_api::FieldData scalar;
    Value scalar;
    CYPHER_FIELD_DATA_LIST* array = nullptr;
    CYPHER_FIELD_DATA_MAP* map = nullptr;

    FieldDatav1() : type(SCALAR) {}

    explicit FieldDatav1(bool rhs) : type(SCALAR), scalar(rhs) {}


    explicit FieldDatav1(int64_t rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldDatav1(float rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldDatav1(double rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldDatav1(const std::string& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldDatav1(std::string&& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldDatav1(const char* rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldDatav1(const char* rhs, size_t s) : type(SCALAR), scalar(std::string(rhs, s)) {}

    explicit FieldDatav1(const Value& rhs) : type(SCALAR), scalar(rhs) {}

    explicit FieldDatav1(Value&& rhs) : type(SCALAR), scalar(std::move(rhs)) {}

    explicit FieldDatav1(const std::vector<Value>& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST();
        for (auto& item : rhs) {
            array->emplace_back(item);
        }
    }


    explicit FieldDatav1(const CYPHER_FIELD_DATA_LIST& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST(rhs);
    }

    explicit FieldDatav1(std::vector<Value>&& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST();
        for (auto& item : rhs) {
            array->emplace_back(std::move(item));
        }
    }


    explicit FieldDatav1(CYPHER_FIELD_DATA_LIST&& rhs) {
        type = ARRAY;
        array = new CYPHER_FIELD_DATA_LIST(std::move(rhs));
    }

    explicit FieldDatav1(const std::unordered_map<std::string, Value>& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP();
        for (auto& kv : rhs) {
            map->emplace(kv);
        }
    }
    explicit FieldDatav1(const CYPHER_FIELD_DATA_MAP& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP(rhs);
    }

    explicit FieldDatav1(std::unordered_map<std::string, Value>&& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP();
        for (auto& kv : rhs) {
            map->emplace(std::move(kv));
        }
    }

    explicit FieldDatav1(CYPHER_FIELD_DATA_MAP&& rhs) {
        type = MAP;
        map = new CYPHER_FIELD_DATA_MAP(std::move(rhs));
    }

        ~FieldDatav1() {
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

    FieldDatav1(const FieldDatav1& rhs) {
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

    FieldDatav1(FieldDatav1&& rhs) {
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

    FieldDatav1& operator=(const Value& rhs) {
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

    FieldDatav1& operator=(Value&& rhs) {
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

    FieldDatav1& operator=(const ::cypher::FieldDatav1& rhs) {
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

    FieldDatav1& operator=(::cypher::FieldDatav1&& rhs) {
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

    bool operator==(const FieldDatav1& rhs) const {
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

    bool operator!=(const FieldDatav1& rhs) const { return !(*this == rhs); }

    bool operator>(const FieldDatav1& rhs) const {
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

    bool operator>=(const FieldDatav1& rhs) const {
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

    bool operator<(const FieldDatav1& rhs) const { return !(*this >= rhs); }

    bool operator<=(const FieldDatav1& rhs) const { return !(*this > rhs); }

    bool EqualNull() const {
        switch (type) {
        case ARRAY:
            return !array || array->empty();
        case MAP:
            return !map || map->empty();
        case SCALAR:
            return scalar.IsNull();
        }
        CYPHER_TODO();
        return false;
    }

    bool IsNull() const { return type == SCALAR && scalar.IsNull(); }

    bool IsBool() const { return type == SCALAR && scalar.IsBool(); }

    bool IsInteger() const {
        return type == SCALAR && scalar.IsInteger();
    }

    bool IsReal() const {
        return type == SCALAR && scalar.IsDouble();
    }

    bool IsString() const { return type == SCALAR && scalar.IsString(); }

    bool IsArray() const { return type == ARRAY; }

    static FieldDatav1 Array(size_t n) { return FieldDatav1(std::vector<cypher::FieldDatav1>(n)); }

    bool IsMap() const { return type == MAP; }

    std::string ToString() const {
        switch (type) {
        case ARRAY: {
            std::string str("[");
            for (auto& s : *array) str.append(s.ToString()).append(",");
            if (str.size() > 1) str.pop_back();
            str.append("]");
            return str;
        }
        case MAP: {
            std::string str("{");
            for (auto& pair : *map) {
                str.append(pair.first).append(":").append(pair.second.ToString());
                str.append(",");
            }
            if (str.size() > 1) str.pop_back();
            str.append("}");
            return str;
        }
        case SCALAR:
            return scalar.ToString();
        }
        throw std::runtime_error("internal error: unhandled type: " + std::to_string((int)type));
    }
    inline bool AsBool() const {
        return scalar.AsBool();
    }

    inline int64_t AsInteger() const {
        return scalar.AsInteger();
    }


    inline double AsDouble() const {
        return scalar.AsDouble();
    }

    inline std::string AsString() const {
        return scalar.AsString();
    }

    inline std::vector<Value> AsConstantArray() const {
        if (type == ARRAY) {
            std::vector<Value> ans;
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

enum PredicateType {
    None = 1,
    Single,
    Any,
    All
};

}  // namespace cypher
