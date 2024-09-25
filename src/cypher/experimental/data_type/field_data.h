
#pragma once
#include <variant>
#include <unordered_map>
#include <builder/dyn_var.h>
#include <builder/static_var.h>
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
        std::monostate, // Represent the null state
        dyn_var<short>,
        dyn_var<int>,
        dyn_var<long>,
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

    CScalarData(const lgraph::FieldData& other) {
        type_ = other.type;
        switch (other.type) {
        case lgraph::FieldType::NUL:
            constant_.emplace<std::monostate>();
            break;
        case lgraph::FieldType::INT64:
            constant_.emplace<dyn_var<long>>((long)other.integer());
            break;
        default:
            CYPHER_TODO();
        }
    }

    explicit CScalarData(long integer) {
        constant_.emplace<dyn_var<long>>(integer);
        type_ = lgraph::FieldType::INT64;
    }
    
    explicit CScalarData(const static_var<long> &integer)
    : type_(FieldType::INT64) {
        constant_ = (dyn_var<long>) integer;
    }

    explicit CScalarData(const dyn_var<long> &integer)
    : constant_(integer), type_(FieldType::INT64) {}

    explicit CScalarData(dyn_var<long>&& integer)
    : constant_(std::move(integer)), type_(FieldType::INT64) {}

    inline dyn_var<long> integer() const {
        switch (type_) {
        case FieldType::NUL:
        case FieldType::BOOL:
            throw std::bad_cast();
        case FieldType::INT8:
            return std::get<dyn_var<short>>(constant_);
        case FieldType::INT16:
            return std::get<dyn_var<short>>(constant_);
        case FieldType::INT32:
            return std::get<dyn_var<int>>(constant_);
        case FieldType::INT64:
            return std::get<dyn_var<long>>(constant_);
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
        return dyn_var<long>(0);
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

    dyn_var<long> Int64() const {
        return std::get<dyn_var<long>>(constant_);
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

    CFieldData(const CScalarData& scalar) : type(SCALAR), scalar(scalar) {}

    CFieldData(CScalarData&& scalar) : type(SCALAR), scalar(std::move(scalar)) {}

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

    explicit CFieldData(const static_var<long>& scalar) : type(SCALAR), scalar(scalar) {}

    bool is_null() const { return type == SCALAR && scalar.is_null(); }

    bool is_string() const { return type == SCALAR && scalar.is_string(); }

    bool is_integer() const { return type == SCALAR && scalar.is_integer(); }

    bool is_real() const { return type == SCALAR && scalar.is_real(); }

    CFieldData operator+(const CFieldData& other) const;

    CFieldData operator-(const CFieldData& other) const;
};
} // namespace compilation
} // namespace cypher