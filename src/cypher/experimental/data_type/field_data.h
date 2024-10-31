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
#pragma once
#include <builder/dyn_var.h>
#include <builder/static_var.h>
#include <variant>
#include <unordered_map>
#include "core/data_type.h"
#include "cypher/cypher_types.h"
#include "cypher/cypher_exception.h"

using builder::static_var;
using builder::dyn_var;
using lgraph::FieldType;

namespace cypher {
namespace compilation {

struct CScalarData {
    static constexpr const char* type_name = "CScalarData";
    std::variant<
        std::monostate,  // Represent the null state
        dyn_var<int16_t>,
        dyn_var<int>,
        dyn_var<int64_t>,
        dyn_var<float>,
        dyn_var<double>
    > constant_;

    lgraph::FieldType type_;

    CScalarData() {
        type_ = lgraph_api::FieldType::NUL;
    }

    CScalarData(CScalarData &&data)
    : constant_(std::move(data.constant_)), type_(data.type_) {}

    CScalarData(const CScalarData& other)
    : constant_(other.constant_), type_(other.type_) {}

    explicit CScalarData(const lgraph::FieldData& other) {
        type_ = other.type;
        switch (other.type) {
        case lgraph::FieldType::NUL:
            constant_.emplace<std::monostate>();
            break;
        case lgraph::FieldType::INT64:
            constant_.emplace<dyn_var<int64_t>>((int64_t)other.integer());
            break;
        default:
            CYPHER_TODO();
        }
    }

    explicit CScalarData(int64_t integer) {
        constant_.emplace<dyn_var<int64_t>>(integer);
        type_ = lgraph::FieldType::INT64;
    }

    explicit CScalarData(const static_var<int64_t> &integer)
    : type_(FieldType::INT64) {
        constant_ = (dyn_var<int64_t>) integer;
    }

    explicit CScalarData(const dyn_var<int64_t> &integer)
    : constant_(integer), type_(FieldType::INT64) {}

    explicit CScalarData(dyn_var<int64_t>&& integer)
    : constant_(std::move(integer)), type_(FieldType::INT64) {}

    inline dyn_var<int64_t> integer() const {
        switch (type_) {
        case FieldType::NUL:
        case FieldType::BOOL:
            throw std::bad_cast();
        case FieldType::INT8:
            return std::get<dyn_var<int32_t>>(constant_);
        case FieldType::INT16:
            return std::get<dyn_var<int32_t>>(constant_);
        case FieldType::INT32:
            return std::get<dyn_var<int>>(constant_);
        case FieldType::INT64:
            return std::get<dyn_var<int64_t>>(constant_);
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
        return dyn_var<int64_t>(0);
    }

    inline dyn_var<double> real() const {
        switch (type_) {
        case FieldType::NUL:
        case FieldType::BOOL:
        case FieldType::INT8:
        case FieldType::INT16:
        case FieldType::INT32:
        case FieldType::INT64:
            throw std::bad_cast();
        case FieldType::FLOAT:
            std::get<dyn_var<float>>(constant_);
        case FieldType::DOUBLE:
            std::get<dyn_var<double>>(constant_);
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
        return dyn_var<double>(0);
    }

    dyn_var<int64_t> Int64() const {
        return std::get<dyn_var<int64_t>>(constant_);
    }

    inline bool is_integer() const {
        return type_ >= FieldType::INT8 && type_ <= FieldType::INT64;
    }

    inline bool is_real() const {
        return type_ == FieldType::DOUBLE || type_ == FieldType::FLOAT;
    }

    bool is_null() const { return type_ == lgraph::FieldType::NUL; }

    bool is_string() const { return type_ == lgraph::FieldType::STRING; }

    CScalarData& operator=(CScalarData&& other) noexcept {
        if (this != &other) {
            constant_ = std::move(other.constant_);
            type_ = std::move(other.type_);
        }
        return *this;
    }

    CScalarData& operator=(const CScalarData& other) {
        if (this != &other) {
            constant_ = other.constant_;
            type_ = other.type_;
        }
        return *this;
    }

    CScalarData operator+(const CScalarData& other) const;
};

struct CFieldData {
    enum FieldType { SCALAR, ARRAY, MAP} type;

    CScalarData scalar;
    std::vector<CFieldData>* array = nullptr;
    std::unordered_map<std::string, CFieldData>* map = nullptr;

    CFieldData() : type(SCALAR) {}

    CFieldData(const CFieldData &data) : type(data.type), scalar(data.scalar) {}

    explicit CFieldData(const CScalarData& scalar) : type(SCALAR), scalar(scalar) {}

    CFieldData& operator=(const CFieldData& data) {
        this->type = data.type;
        this->scalar = data.scalar;
        return *this;
    }

    CFieldData& operator=(CFieldData&& data) {
        this->type = std::move(data.type);
        this->scalar = std::move(data.scalar);
        return *this;
    }

    explicit CFieldData(const static_var<int64_t>& scalar) : type(SCALAR), scalar(scalar) {}

    bool is_null() const { return type == SCALAR && scalar.is_null(); }

    bool is_string() const { return type == SCALAR && scalar.is_string(); }

    bool is_integer() const { return type == SCALAR && scalar.is_integer(); }

    bool is_real() const { return type == SCALAR && scalar.is_real(); }

    CFieldData operator+(const CFieldData& other) const;

    CFieldData operator-(const CFieldData& other) const;
};
}  // namespace compilation
}  // namespace cypher
