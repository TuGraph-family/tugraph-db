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

#include <unordered_map>
#include <regex>

#include "fma-common/string_formatter.h"
#include "fma-common/string_util.h"
#include "fma-common/text_parser.h"

#include "core/data_type.h"
#include "core/value.h"
#include "lgraph/lgraph_types.h"

//===============================
// parser
//===============================
namespace fma_common {
namespace TextParserUtils {

template <>
inline size_t ParseT<::lgraph_api::Date>(const char* b, const char* e, ::lgraph_api::Date& d) {
    return ::lgraph_api::Date::Parse(b, e, d);
}

template <>
inline size_t ParseT<::lgraph_api::DateTime>(const char* b, const char* e,
                                             ::lgraph_api::DateTime& d) {
    return ::lgraph_api::DateTime::Parse(b, e, d);
}

}  // namespace TextParserUtils
}  // namespace fma_common

namespace lgraph {
typedef lgraph_api::FieldData FieldData;
typedef lgraph_api::FieldType FieldType;
typedef lgraph_api::Date Date;
typedef lgraph_api::DateTime DateTime;
typedef lgraph_api::Point<lgraph_api::Wgs84> PointWgs84;
typedef lgraph_api::Point<lgraph_api::Cartesian> PointCartesian;
typedef lgraph_api::LineString<lgraph_api::Wgs84> LineStringWgs84;
typedef lgraph_api::LineString<lgraph_api::Cartesian> LineStringCartesian;
typedef lgraph_api::Polygon<lgraph_api::Wgs84> PolygonWgs84;
typedef lgraph_api::Polygon<lgraph_api::Cartesian> PolygonCartesian;

//===============================
// type name and sizes
//===============================
namespace field_data_helper {
namespace _detail {
static constexpr const char* FieldTypeNames[] = {"NUL",   "BOOL",     "INT8",   "INT16",
                                                 "INT32", "INT64",    "FLOAT",  "DOUBLE",
                                                 "DATE",  "DATETIME", "STRING", "BLOB",
                                                 "POINT", "LINESTRING", "POLYGON", "SPATIAL",
                                                 "FLOAT_VECTOR"};

static std::unordered_map<std::string, FieldType> _FieldName2TypeDict_() {
    std::unordered_map<std::string, FieldType> ret;
    for (int i = 0; i <= (int)FieldType::FLOAT_VECTOR; i++) {
        ret[FieldTypeNames[i]] = FieldType(i);
        ret[fma_common::ToLower(FieldTypeNames[i])] = FieldType(i);
    }
    return ret;
}

static constexpr size_t FieldTypeSizes[] = {
    0,   // nul
    1,   // bool
    1,   // int8
    2,   // int16
    4,   // int32
    8,   // int64
    4,   // float
    8,   // double
    4,   // date
    8,   // datetime
    0,   // string
    0,   // blob
    50,  // Point
    0,   // LineString
    0,   // Polygon
    0,   // Spatial
    0    // float vector
};

static constexpr bool IsFixedLengthType[] = {
    false,  // nul
    true,   // bool
    true,   // int8
    true,   // int16
    true,   // int32
    true,   // int64
    true,   // float
    true,   // double
    true,   // date
    true,   // datetime
    false,  // string
    false,  // blob
    true,   // Point
    false,  // LineString
    false,  // Polygon
    false,  // Spatial
    false   // float vector
};

template <class T, size_t N>
constexpr size_t array_size(T (&)[N]) {
    return N;
}

static_assert(array_size(FieldTypeNames) == array_size(FieldTypeSizes) &&
                  array_size(FieldTypeSizes) == array_size(IsFixedLengthType),
              "FieldTypeNames, FieldTypeSizes and IsFixedLengthType must have "
              "the same size");
}  // namespace _detail

inline bool TryGetFieldType(const std::string& field_type_name, FieldType& ret) {
    static std::unordered_map<std::string, FieldType> map = _detail::_FieldName2TypeDict_();
    auto it = map.find(field_type_name);
    if (it == map.end()) return false;
    ret = it->second;
    return true;
}

inline constexpr size_t FieldTypeSize(FieldType dt) { return _detail::FieldTypeSizes[dt]; }

inline constexpr const char* FieldTypeName(FieldType dt) { return _detail::FieldTypeNames[dt]; }

inline constexpr bool IsFixedLengthFieldType(FieldType dt) {
    return _detail::IsFixedLengthType[dt];
}

inline std::string TryGetFieldTypeName(FieldType ft) {
    if (ft >= FieldType::NUL && ft <= FieldType::FLOAT_VECTOR) return FieldTypeName(ft);
    return fma_common::StringFormatter::Format("[Illegal FieldType, value={}]", (int)ft);
}

//===============================
// stored type and c types
//===============================
template <FieldType DT>
struct FieldType2StorageType {
    // typedef void type;
};

template <>
struct FieldType2StorageType<FieldType::NUL> {
    typedef void type;
};
template <>
struct FieldType2StorageType<FieldType::BOOL> {
    typedef int8_t type;
};
template <>
struct FieldType2StorageType<FieldType::INT8> {
    typedef int8_t type;
};
template <>
struct FieldType2StorageType<FieldType::INT16> {
    typedef int16_t type;
};
template <>
struct FieldType2StorageType<FieldType::INT32> {
    typedef int32_t type;
};
template <>
struct FieldType2StorageType<FieldType::INT64> {
    typedef int64_t type;
};
template <>
struct FieldType2StorageType<FieldType::FLOAT> {
    typedef float type;
};
template <>
struct FieldType2StorageType<FieldType::DOUBLE> {
    typedef double type;
};
template <>
struct FieldType2StorageType<FieldType::DATE> {
    typedef int32_t type;
};
template <>
struct FieldType2StorageType<FieldType::DATETIME> {
    typedef int64_t type;
};
template <>
struct FieldType2StorageType<FieldType::STRING> {
    typedef std::string type;
};
template <>
struct FieldType2StorageType<FieldType::BLOB> {
    typedef std::string type;
};
template <>
struct FieldType2StorageType<FieldType::POINT> {
    typedef std::string type;
};
template <>
struct FieldType2StorageType<FieldType::LINESTRING> {
    typedef std::string type;
};
template <>
struct FieldType2StorageType<FieldType::POLYGON> {
    typedef std::string type;
};
template <>
struct FieldType2StorageType<FieldType::SPATIAL> {
    typedef std::string type;
};
template <>
struct FieldType2StorageType<FieldType::FLOAT_VECTOR> {
    typedef std::vector<float> type;
};

template <FieldType FT>
struct FieldType2CType {
    typedef typename FieldType2StorageType<FT>::type type;
};

template <>
struct FieldType2CType<FieldType::BOOL> {
    typedef bool type;
};

template <>
struct FieldType2CType<FieldType::DATE> {
    typedef Date type;
};

template <>
struct FieldType2CType<FieldType::DATETIME> {
    typedef DateTime type;
};

// TODO(shw): how to correlate spatial fieldtype with ctype?
template <>
struct FieldType2CType<FieldType::POINT> {
};

template <FieldType FT>
inline FieldData MakeFieldData(const typename FieldType2CType<FT>::type& cd) {
    FMA_ASSERT(false);
    return FieldData();
}
template <>
inline FieldData MakeFieldData<FieldType::BOOL>(
    const typename FieldType2CType<FieldType::BOOL>::type& cd) {
    return FieldData::Bool(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::INT8>(
    const typename FieldType2CType<FieldType::INT8>::type& cd) {
    return FieldData::Int8(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::INT16>(
    const typename FieldType2CType<FieldType::INT16>::type& cd) {
    return FieldData::Int16(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::INT32>(
    const typename FieldType2CType<FieldType::INT32>::type& cd) {
    return FieldData::Int32(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::INT64>(
    const typename FieldType2CType<FieldType::INT64>::type& cd) {
    return FieldData::Int64(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::FLOAT>(
    const typename FieldType2CType<FieldType::FLOAT>::type& cd) {
    return FieldData::Float(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::DOUBLE>(
    const typename FieldType2CType<FieldType::DOUBLE>::type& cd) {
    return FieldData::Double(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::DATE>(
    const typename FieldType2CType<FieldType::DATE>::type& cd) {
    return FieldData::Date(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::DATETIME>(
    const typename FieldType2CType<FieldType::DATETIME>::type& cd) {
    return FieldData::DateTime(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::STRING>(
    const typename FieldType2CType<FieldType::STRING>::type& cd) {
    return FieldData::String(cd);
}
template <>
inline FieldData MakeFieldData<FieldType::BLOB>(
    const typename FieldType2CType<FieldType::BLOB>::type& cd) {
    return FieldData::Blob(cd);
}

//===============================
// max and min values
//===============================
template <FieldType FT>
constexpr inline typename FieldType2StorageType<FT>::type MaxStoreValue() {
    return std::numeric_limits<typename FieldType2StorageType<FT>::type>::max();
}
template <>
constexpr inline typename FieldType2StorageType<FieldType::BOOL>::type
MaxStoreValue<FieldType::BOOL>() {
    return 1;
}
template <>
constexpr inline typename FieldType2StorageType<FieldType::DATE>::type
MaxStoreValue<FieldType::DATE>() {
    return ::lgraph_api::MaxDaysSinceEpochForDate();
}
template <>
constexpr inline typename FieldType2StorageType<FieldType::DATETIME>::type
MaxStoreValue<FieldType::DATETIME>() {
    return ::lgraph_api::MaxMicroSecondsSinceEpochForDateTime();
}

template <FieldType FT>
constexpr inline typename FieldType2StorageType<FT>::type MinStoreValue() {
    return std::numeric_limits<typename FieldType2StorageType<FT>::type>::lowest();
}
template <>
constexpr inline typename FieldType2StorageType<FieldType::BOOL>::type
MinStoreValue<FieldType::BOOL>() {
    return 0;
}
template <>
constexpr inline typename FieldType2StorageType<FieldType::DATE>::type
MinStoreValue<FieldType::DATE>() {
    return ::lgraph_api::MinDaysSinceEpochForDate();
}
template <>
constexpr inline typename FieldType2StorageType<FieldType::DATETIME>::type
MinStoreValue<FieldType::DATETIME>() {
    return ::lgraph_api::MinMicroSecondsSinceEpochForDateTime();
}

template <FieldType DstType>
struct FieldDataRangeCheck {
    template <typename FromT>
    static inline bool CheckAndCopy(const FromT& src,
                                    typename FieldType2StorageType<DstType>::type& dst) {
        if (src >= MinStoreValue<DstType>() && src <= MaxStoreValue<DstType>()) {
            dst = static_cast<typename FieldType2StorageType<DstType>::type>(src);
            return true;
        }
        return false;
    }
};

template <>
struct FieldDataRangeCheck<FieldType::STRING> {
    template <typename FromT>
    static inline bool CheckAndCopy(const FromT& src,
                                    typename FieldType2StorageType<FieldType::STRING>::type& dst) {
        return true;
    }
};

template <>
struct FieldDataRangeCheck<FieldType::BLOB> {
    template <typename FromT>
    static inline bool CheckAndCopy(const FromT& src,
                                    typename FieldType2StorageType<FieldType::STRING>::type& dst) {
        return true;
    }
};

template <>
struct FieldDataRangeCheck<FieldType::POINT> {
    template <typename FromT>
    static inline bool CheckAndCopy(const FromT& src,
                                    typename FieldType2StorageType<FieldType::STRING>::type& dst) {
        return true;
    }
};

template <>
struct FieldDataRangeCheck<FieldType::LINESTRING> {
    template <typename FromT>
    static inline bool CheckAndCopy(const FromT& src,
                                    typename FieldType2StorageType<FieldType::STRING>::type& dst) {
        return true;
    }
};

template <>
struct FieldDataRangeCheck<FieldType::POLYGON> {
    template <typename FromT>
    static inline bool CheckAndCopy(const FromT& src,
                                    typename FieldType2StorageType<FieldType::STRING>::type& dst) {
        return true;
    }
};

template <>
struct FieldDataRangeCheck<FieldType::SPATIAL> {
    template <typename FromT>
    static inline bool CheckAndCopy(const FromT& src,
                                    typename FieldType2StorageType<FieldType::STRING>::type& dst) {
        return true;
    }
};

template <>
struct FieldDataRangeCheck<FieldType::FLOAT_VECTOR> {
    template <typename FromT>
    static inline bool CheckAndCopy(
        const FromT& src, typename FieldType2StorageType<FieldType::FLOAT_VECTOR>::type& dst) {
        return true;
    }
};

template <FieldType DstType>
inline bool CopyFdIntoDstStorageType(const FieldData& fd,
                                     typename FieldType2StorageType<DstType>::type& dst) {
    return false;
}

template <>
inline bool CopyFdIntoDstStorageType<FieldType::DATE>(
    const FieldData& fd, typename FieldType2StorageType<FieldType::DATE>::type& dst) {
    FMA_DBG_ASSERT(fd.IsDateTime());
    dst = Date(fd.AsDateTime()).DaysSinceEpoch();
    return true;
}

template <>
inline bool CopyFdIntoDstStorageType<FieldType::DATETIME>(
    const FieldData& fd, typename FieldType2StorageType<FieldType::DATETIME>::type& dst) {
    FMA_DBG_ASSERT(fd.IsDate());
    dst = DateTime(fd.AsDate()).MicroSecondsSinceEpoch();
    return true;
}

//===============================
// copying and referencing data
//===============================
inline Value GetConstRefOfFieldDataContent(const FieldData& fd) {
    switch (fd.type) {
    case FieldType::NUL:
        return Value();
    case FieldType::BOOL:
        return Value::ConstRef(fd.data.boolean);
    case FieldType::INT8:
        return Value::ConstRef(fd.data.int8);
    case FieldType::INT16:
        return Value::ConstRef(fd.data.int16);
    case FieldType::INT32:
        return Value::ConstRef(fd.data.int32);
    case FieldType::INT64:
        return Value::ConstRef(fd.data.int64);
    case FieldType::FLOAT:
        return Value::ConstRef(fd.data.sp);
    case FieldType::DOUBLE:
        return Value::ConstRef(fd.data.dp);
    case FieldType::DATE:
        return Value::ConstRef(fd.data.int32);
    case FieldType::DATETIME:
        return Value::ConstRef(fd.data.int64);
    case FieldType::STRING:
    case FieldType::BLOB:
    case FieldType::POINT:
    case FieldType::LINESTRING:
    case FieldType::POLYGON:
    case FieldType::SPATIAL:
        return Value::ConstRef(*fd.data.buf);
    case FieldType::FLOAT_VECTOR:
        return Value::ConstRef(*fd.data.vp);
    }
    FMA_ASSERT(false);
    return Value();
}

template <FieldType FT>
inline const typename FieldType2StorageType<FT>::type& GetStoredValue(const FieldData& fd) {}
template <>
inline const typename FieldType2StorageType<FieldType::BOOL>::type& GetStoredValue<FieldType::BOOL>(
    const FieldData& fd) {
    return fd.data.int8;
}
template <>
inline const typename FieldType2StorageType<FieldType::INT8>::type& GetStoredValue<FieldType::INT8>(
    const FieldData& fd) {
    return fd.data.int8;
}
template <>
inline const typename FieldType2StorageType<FieldType::INT16>::type&
GetStoredValue<FieldType::INT16>(const FieldData& fd) {
    return fd.data.int16;
}
template <>
inline const typename FieldType2StorageType<FieldType::INT32>::type&
GetStoredValue<FieldType::INT32>(const FieldData& fd) {
    return fd.data.int32;
}
template <>
inline const typename FieldType2StorageType<FieldType::INT64>::type&
GetStoredValue<FieldType::INT64>(const FieldData& fd) {
    return fd.data.int64;
}
template <>
inline const typename FieldType2StorageType<FieldType::FLOAT>::type&
GetStoredValue<FieldType::FLOAT>(const FieldData& fd) {
    return fd.data.sp;
}
template <>
inline const typename FieldType2StorageType<FieldType::DOUBLE>::type&
GetStoredValue<FieldType::DOUBLE>(const FieldData& fd) {
    return fd.data.dp;
}
template <>
inline const typename FieldType2StorageType<FieldType::DATE>::type& GetStoredValue<FieldType::DATE>(
    const FieldData& fd) {
    return fd.data.int32;
}
template <>
inline const typename FieldType2StorageType<FieldType::DATETIME>::type&
GetStoredValue<FieldType::DATETIME>(const FieldData& fd) {
    return fd.data.int64;
}
template <>
inline const typename FieldType2StorageType<FieldType::STRING>::type&
GetStoredValue<FieldType::STRING>(const FieldData& fd) {
    return *fd.data.buf;
}
template <>
inline const typename FieldType2StorageType<FieldType::BLOB>::type&
GetStoredValue<FieldType::BLOB>(
    const FieldData& fd) {
    return *fd.data.buf;
}
template <>
inline const typename FieldType2StorageType<FieldType::POINT>::type&
GetStoredValue<FieldType::POINT>(
    const FieldData& fd) {
    return *fd.data.buf;
}
template <>
inline const typename FieldType2StorageType<FieldType::LINESTRING>::type&
GetStoredValue<FieldType::LINESTRING>(
    const FieldData& fd) {
    return *fd.data.buf;
}
template <>
inline const typename FieldType2StorageType<FieldType::POLYGON>::type&
GetStoredValue<FieldType::POLYGON>(
    const FieldData& fd) {
    return *fd.data.buf;
}
template <>
inline const typename FieldType2StorageType<FieldType::SPATIAL>::type&
GetStoredValue<FieldType::SPATIAL>(
    const FieldData& fd) {
    return *fd.data.buf;
}
template <>
inline const typename FieldType2StorageType<FieldType::FLOAT_VECTOR>::type&
GetStoredValue<FieldType::FLOAT_VECTOR>(
    const FieldData& fd) {
    return *fd.data.vp;
}

template <FieldType FT>
inline typename FieldType2CType<FT>::type GetFieldDataCValue(const FieldData& fd) {
    return static_cast<typename FieldType2CType<FT>::type>(GetStoredValue<FT>(fd));
}

template <FieldType FT>
struct ScalarToStorageType {
    typedef typename FieldType2StorageType<FT>::type ST;
    template <typename D>
    static inline bool Copy(D d, ST& sd) {
        if (d < MinStoreValue<FT>() || d > MaxStoreValue<FT>()) return false;
        sd = static_cast<ST>(d);
        return true;
    }
};

//===============================
// converting between different data types
//===============================
template <FieldType DstType>
inline bool IsCompatibleType(FieldType st) {
    return false;
}
template <>
inline bool IsCompatibleType<FieldType::BOOL>(FieldType st) {
    return st <= FieldType::INT64;
}
template <>
inline bool IsCompatibleType<FieldType::INT8>(FieldType st) {
    return st <= FieldType::DOUBLE;
}
template <>
inline bool IsCompatibleType<FieldType::INT16>(FieldType st) {
    return st <= FieldType::DOUBLE;
}
template <>
inline bool IsCompatibleType<FieldType::INT32>(FieldType st) {
    return st <= FieldType::DOUBLE;
}
template <>
inline bool IsCompatibleType<FieldType::INT64>(FieldType st) {
    return st <= FieldType::DOUBLE;
}
template <>
inline bool IsCompatibleType<FieldType::FLOAT>(FieldType st) {
    return st <= FieldType::DOUBLE;
}
template <>
inline bool IsCompatibleType<FieldType::DOUBLE>(FieldType st) {
    return st <= FieldType::DOUBLE;
}
template <>
inline bool IsCompatibleType<FieldType::DATE>(FieldType st) {
    return st == FieldType::STRING || st == FieldType::DATE || st == FieldType::DATETIME;
}
template <>
inline bool IsCompatibleType<FieldType::DATETIME>(FieldType st) {
    return st == FieldType::STRING || st == FieldType::DATE || st == FieldType::DATETIME;
}
template <>
inline bool IsCompatibleType<FieldType::STRING>(FieldType st) {
    return st == FieldType::STRING;
}
template <>
inline bool IsCompatibleType<FieldType::BLOB>(FieldType st) {
    return st == FieldType::STRING || st == FieldType::BLOB;
}
template <>
inline bool IsCompatibleType<FieldType::POINT>(FieldType st) {
    return st == FieldType::STRING;
}
template <>
inline bool IsCompatibleType<FieldType::LINESTRING>(FieldType st) {
    return st == FieldType::STRING;
}
template <>
inline bool IsCompatibleType<FieldType::POLYGON>(FieldType st) {
    return st == FieldType::STRING;
}
template <>
inline bool IsCompatibleType<FieldType::SPATIAL>(FieldType st) {
    return st == FieldType::STRING;
}
template <>
inline bool IsCompatibleType<FieldType::FLOAT_VECTOR>(FieldType st) {
    return st == FieldType::FLOAT_VECTOR;
}

template <FieldType DstType>
inline size_t ParseStringIntoFieldData(const char* beg, const char* end, FieldData& fd) {
    typename FieldType2CType<DstType>::type cd;
    size_t s = fma_common::TextParserUtils::ParseT(beg, end, cd);
    if (s == 0) return 0;
    fd = MakeFieldData<DstType>(cd);
    return s;
}
template <>
inline size_t ParseStringIntoFieldData<FieldType::STRING>(const char* beg, const char* end,
                                                          FieldData& fd) {
    std::string cd;
    size_t s = fma_common::TextParserUtils::ParseCsvString(beg, end, cd);
    if (s == 0) return 0;
    fd = FieldData::String(std::move(cd));
    return s;
}
template <>
inline size_t ParseStringIntoFieldData<FieldType::BLOB>(const char* beg, const char* end,
                                                        FieldData& fd) {
    std::string cd;
    size_t s = fma_common::TextParserUtils::ParseCsvString(beg, end, cd);
    if (s == 0) return 0;
    std::string decoded;
    if (!::lgraph_api::base64::TryDecode(cd, decoded)) {
        THROW_CODE(InputError, "Value is not a valid BASE64 string: " + cd.substr(0, 64) +
                         (cd.size() > 64 ? "..." : ""));
    }
    fd = FieldData::Blob(std::move(decoded));
    return s;
}
template <>
inline size_t ParseStringIntoFieldData<FieldType::FLOAT_VECTOR>(const char* beg, const char* end,
                                                                FieldData& fd) {
    // string copy
    std::string cd;
    size_t s = fma_common::TextParserUtils::ParseCsvString(beg, end, cd);
    if (s == 0) return 0;
    // check if there are only numbers and commas
    std::regex nonNumbersAndCommas("[^0-9,.]");
    if (std::regex_search(cd, nonNumbersAndCommas)) {
        THROW_CODE(InputError, "This is not a float vector string");
    }
    // Check if the string conforms to the following format : 1.000000,2.000000,3.000000,...
    std::regex vector("^(?:[-+]?\\d*(?:\\.\\d+)?)(?:,[-+]?\\d*(?:\\.\\d+)?){1,}$");
    if (!std::regex_match(cd, vector)) {
        THROW_CODE(InputError, "This is not a float vector string");
    }
    // check if there are 1.000,,2.000 & 1.000,2.000,
    if (cd.front() == ',' || cd.back() == ',' || cd.find(",,") != std::string::npos) {
        THROW_CODE(InputError, "This is not a float vector string");
    }
    std::vector<float> vec;
    std::regex pattern("-?[0-9]+\\.?[0-9]*");
    std::sregex_iterator begin_it(cd.begin(), cd.end(), pattern), end_it;
    while (begin_it != end_it) {
        std::smatch match = *begin_it;
        vec.push_back(std::stof(match.str()));
        ++begin_it;
    }
    fd = FieldData::FloatVector(vec);
    return s;
}

template <FieldType DstType>
inline bool ParseStringIntoStorageType(const std::string& str,
                                       typename FieldType2StorageType<DstType>::type& sd) {
    typename FieldType2CType<DstType>::type cd;
    size_t s = fma_common::TextParserUtils::ParseT(str.data(), str.data() + str.size(), cd);
    if (!s) return false;
    sd = static_cast<typename FieldType2StorageType<DstType>::type>(cd);
    return s == str.size();
}
template <>
inline bool ParseStringIntoStorageType<FieldType::STRING>(const std::string& str, std::string& sd) {
    sd.assign(str.begin(), str.end());
    return true;
}
template <>
inline bool ParseStringIntoStorageType<FieldType::BLOB>(const std::string& str, std::string& sd) {
    return ::lgraph_api::base64::TryDecode(str, sd);
}
template <>
inline bool ParseStringIntoStorageType<FieldType::POINT>(const std::string& str, std::string& sd) {
    if (!::lgraph_api::TryDecodeEWKB(str,
    ::lgraph_api::SpatialType::POINT)) {
        return false;
    }

    sd.assign(str.begin(), str.end());
    return true;
}
template <>
inline bool ParseStringIntoStorageType<FieldType::LINESTRING>
(const std::string& str, std::string& sd) {
    if (!::lgraph_api::TryDecodeEWKB(str,
    ::lgraph_api::SpatialType::LINESTRING)) {
        return false;
    }

    sd.assign(str.begin(), str.end());
    return true;
}
template <>
inline bool ParseStringIntoStorageType<FieldType::POLYGON>
(const std::string& str, std::string& sd) {
    if (!::lgraph_api::TryDecodeEWKB(str,
    ::lgraph_api::SpatialType::POLYGON)) {
        return false;
    }

    sd.assign(str.begin(), str.end());
    return true;
}
template <>
inline bool ParseStringIntoStorageType<FieldType::SPATIAL>
(const std::string& str, std::string& sd) {
    ::lgraph_api::SpatialType s;
    // extracttype may throw any exception if the input str is not valid!
    // return false instead of throw exception;
    try {
        s = ::lgraph_api::ExtractType(str);
    } catch (...) {
        return false;
    }
    if (!::lgraph_api::TryDecodeEWKB(str, s)) {
        return false;
    }

    sd.assign(str.begin(), str.end());
    return true;
}
template <>
inline bool ParseStringIntoStorageType<FieldType::FLOAT_VECTOR>
(const std::string& str, std::vector<float>& sd) {
    // check if there are only numbers and commas
    std::regex nonNumbersAndCommas("[^0-9,.]");
    if (std::regex_search(str, nonNumbersAndCommas)) {
        return false;
    }
    // Check if the string conforms to the following format : 1.000000,2.000000,3.000000,...
    std::regex vector("^(?:[-+]?\\d*(?:\\.\\d+)?)(?:,[-+]?\\d*(?:\\.\\d+)?){1,}$");
    if (!std::regex_match(str, vector)) {
        return false;
    }
    // check if there are 1.000,,2.000 & 1.000,2.000,
    if (str.front() == ',' || str.back() == ',' || str.find(",,") != std::string::npos) {
        return false;
    }
    std::regex pattern("-?[0-9]+\\.?[0-9]*");
    std::sregex_iterator begin_it(str.begin(), str.end(), pattern), end_it;
    while (begin_it != end_it) {
        std::smatch match = *begin_it;
        sd.push_back(std::stof(match.str()));
        ++begin_it;
    }
    return true;
}

// This converts FieldData of different type into DstType
// It is used only in schema.h, and the case when fd.type==DstType is already
// handled there, so here we just assume they are not the same type
template <FieldType DstType>
struct FieldDataTypeConvert {
    typedef typename FieldType2StorageType<DstType>::type ST;
    static inline bool Convert(const FieldData& fd, ST& s) {
        FMA_DBG_CHECK_NEQ(DstType, fd.type);
        if (!IsCompatibleType<DstType>(fd.type)) return false;

#define _GET_SCALAR_VALUE_AND_SET_S(FT)                          \
    do {                                                         \
        auto v = GetStoredValue<FieldType::FT>(fd);              \
        return FieldDataRangeCheck<DstType>::CheckAndCopy(v, s); \
    } while (0)

        switch (fd.type) {
        case FieldType::NUL:
            FMA_ASSERT(false);
        case FieldType::BOOL:
            _GET_SCALAR_VALUE_AND_SET_S(BOOL);
        case FieldType::INT8:
            _GET_SCALAR_VALUE_AND_SET_S(INT8);
        case FieldType::INT16:
            _GET_SCALAR_VALUE_AND_SET_S(INT16);
        case FieldType::INT32:
            _GET_SCALAR_VALUE_AND_SET_S(INT32);
        case FieldType::INT64:
            _GET_SCALAR_VALUE_AND_SET_S(INT64);
        case FieldType::FLOAT:
            _GET_SCALAR_VALUE_AND_SET_S(FLOAT);
        case FieldType::DOUBLE:
            _GET_SCALAR_VALUE_AND_SET_S(DOUBLE);
        case FieldType::DATE:
            // date can only be converted to datetime
            return CopyFdIntoDstStorageType<DstType>(fd, s);
        case FieldType::DATETIME:
            // datetime can only be converted to date
            return CopyFdIntoDstStorageType<DstType>(fd, s);
        case FieldType::STRING:
            // can be converted to date, datetime, bin or spatial;
            return ParseStringIntoStorageType<DstType>(*fd.data.buf, s);
        case FieldType::BLOB:
            // nothing can be converted from bin
            break;
        case FieldType::POINT:
            // nothing can be converted from Point
            break;
        case FieldType::LINESTRING:
            // nothing can be converted from LineString
            break;
        case FieldType::POLYGON:
            // nothing can be converted from Polygon;
            break;
        case FieldType::SPATIAL:
            // nothing can be converted from spatial
            break;
        case FieldType::FLOAT_VECTOR:
            // nothing can be converted from float vector
            break;
        }
        return false;
    }
};  // namespace field_data_helper

inline void ThrowParseError(const FieldData& fd, FieldType ft) {
    THROW_CODE(InputError,
        "Failed to convert field data [{}] into type {}", fd.ToString(), FieldTypeName(ft));
}

inline void ThrowParseError(const std::string& str, FieldType ft) {
    THROW_CODE(InputError,
        "Failed to convert string [{}] into type {}", str, FieldTypeName(ft));
}

template <typename ParseStringFuncT>
inline size_t ParseStringIntoFieldData(FieldType ft, const char* b, const char* e, FieldData& fd,
                                       const ParseStringFuncT& parse_string) {
    switch (ft) {
    case FieldType::NUL:
        FMA_ASSERT(false);
    case FieldType::BOOL:
        return ParseStringIntoFieldData<FieldType::BOOL>(b, e, fd);
    case FieldType::INT8:
        return ParseStringIntoFieldData<FieldType::INT8>(b, e, fd);
    case FieldType::INT16:
        return ParseStringIntoFieldData<FieldType::INT16>(b, e, fd);
    case FieldType::INT32:
        return ParseStringIntoFieldData<FieldType::INT32>(b, e, fd);
    case FieldType::INT64:
        return ParseStringIntoFieldData<FieldType::INT64>(b, e, fd);
    case FieldType::DATE:
        return ParseStringIntoFieldData<FieldType::DATE>(b, e, fd);
    case FieldType::DATETIME:
        return ParseStringIntoFieldData<FieldType::DATETIME>(b, e, fd);
    case FieldType::FLOAT:
        return ParseStringIntoFieldData<FieldType::FLOAT>(b, e, fd);
    case FieldType::DOUBLE:
        return ParseStringIntoFieldData<FieldType::DOUBLE>(b, e, fd);
    case FieldType::STRING:
        {
            std::string cd;
            size_t s = parse_string(b, e, cd);
            if (s == 0) return 0;
            fd = FieldData::String(std::move(cd));
            return s;
        }
    case FieldType::BLOB:
        {
            std::string cd;
            size_t s = parse_string(b, e, cd);
            if (s == 0) return 0;
            std::string decoded;
            if (!::lgraph_api::base64::TryDecode(cd, decoded)) {
                LOG_INFO() << ("Value is not a valid BASE64 string: " + cd.substr(0, 64) +
                              (cd.size() > 64 ? "..." : ""));
                return 0;
            }
            fd = FieldData::Blob(std::move(decoded));
            return s;
        }
    case FieldType::POINT:
    case FieldType::LINESTRING:
    case FieldType::POLYGON:
    case FieldType::SPATIAL:
        throw std::runtime_error("do not support spatial now!");
    case FieldType::FLOAT_VECTOR:
        return ParseStringIntoFieldData<FieldType::FLOAT_VECTOR>(b, e, fd);
    }  // switch
    FMA_ASSERT(false);
    return 0;
}

// try to get a Value referring to storage data in fd, converted to type ft
inline bool TryFieldDataToValueOfFieldType(const FieldData& fd, FieldType ft, Value& v) {
    if (fd.type == ft) {
        v = GetConstRefOfFieldDataContent(fd);
        return true;
    }

#define _CONVERT_AND_RETURN_COPY_AS_VALUE(FT)                          \
    do {                                                               \
        typename FieldType2StorageType<FieldType::FT>::type sd;        \
        bool r = FieldDataTypeConvert<FieldType::FT>::Convert(fd, sd); \
        if (!r) return false;                                          \
        v.Copy(sd);                                                    \
        return true;                                                   \
    } while (0)

    switch (ft) {
    case FieldType::NUL:
        return false;
    case FieldType::BOOL:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(BOOL);
    case FieldType::INT8:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(INT8);
    case FieldType::INT16:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(INT16);
    case FieldType::INT32:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(INT32);
    case FieldType::INT64:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(INT64);
    case FieldType::FLOAT:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(FLOAT);
    case FieldType::DOUBLE:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(DOUBLE);
    case FieldType::DATE:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(DATE);
    case FieldType::DATETIME:
        _CONVERT_AND_RETURN_COPY_AS_VALUE(DATETIME);
    case FieldType::STRING:
        return false;
    case FieldType::BLOB:
        {
            // can only convert string to bin
            if (fd.type != FieldType::STRING) return false;
            std::string decoded;
            const std::string s = *fd.data.buf;
            if (!::lgraph_api::base64::TryDecode(s.data(), s.size(), decoded)) return false;
            v.Copy(decoded);
            return true;
        }
    case FieldType::POINT:
        {
            // can only convert string to Point
            if (fd.type != FieldType::STRING) return false;
            const std::string EWKB = *fd.data.buf;
            if (!::lgraph_api::TryDecodeEWKB(EWKB, ::lgraph_api::SpatialType::POINT))
                return false;
            v.Copy(EWKB);
            return true;
        }
    case FieldType::LINESTRING:
        {
            // can only convert string to LineString
            if (fd.type != FieldType::STRING) return false;
            const std::string EWKB = *fd.data.buf;
            if (!::lgraph_api::TryDecodeEWKB(EWKB, ::lgraph_api::SpatialType::LINESTRING))
                return false;
            v.Copy(EWKB);
            return true;
        }
    case FieldType::POLYGON:
        {
            // can only convert string to Polygon
            if (fd.type != FieldType::STRING) return false;
            const std::string EWKB = *fd.data.buf;
            if (!::lgraph_api::TryDecodeEWKB(EWKB, ::lgraph_api::SpatialType::POLYGON))
                return false;
            v.Copy(EWKB);
            return true;
        }
    case FieldType::SPATIAL:
        {
            // can only convert string to spatial
            if (fd.type != FieldType::STRING) return false;
            const std::string EWKB = *fd.data.buf;
            ::lgraph_api::SpatialType s;
            // return false instead of throw exception;
            try {
                s = ::lgraph_api::ExtractType(EWKB);
            } catch (...) {
                return false;
            }

            if (!::lgraph_api::TryDecodeEWKB(EWKB, s))
                return false;
            v.Copy(EWKB);
            return true;
        }
    case FieldType::FLOAT_VECTOR:
        {
            throw std::runtime_error("cannot convert any type to float vector");
            return false;
        }
    }

    FMA_ASSERT(false);
    return false;
}

inline Value FieldDataToValueOfFieldType(const FieldData& fd, FieldType ft) {
    Value v;
    if (!TryFieldDataToValueOfFieldType(fd, ft, v)) ThrowParseError(fd, ft);
    return v;
}

static inline Value ParseStringToValueOfFieldType(const std::string& str, FieldType ft) {
#define _PARSE_STRING_AND_RETURN_VALUE(FT)                                                    \
    do {                                                                                      \
        typedef typename FieldType2CType<FieldType::FT>::type CT;                             \
        typedef typename FieldType2StorageType<FieldType::FT>::type ST;                       \
        CT cd{};                                                                              \
        size_t s =                                                                            \
            fma_common::TextParserUtils::ParseT<CT>(str.data(), str.data() + str.size(), cd); \
        if (s != str.size()) ThrowParseError(str, FieldType::FT);                             \
        Value v;                                                                              \
        v.Copy(static_cast<ST>(cd));                                                          \
        return v;                                                                             \
    } while (0)

    switch (ft) {
    case FieldType::NUL:
        FMA_ASSERT(false);
    case FieldType::BOOL:
        _PARSE_STRING_AND_RETURN_VALUE(BOOL);
    case FieldType::INT8:
        _PARSE_STRING_AND_RETURN_VALUE(INT8);
    case FieldType::INT16:
        _PARSE_STRING_AND_RETURN_VALUE(INT16);
    case FieldType::INT32:
        _PARSE_STRING_AND_RETURN_VALUE(INT32);
    case FieldType::INT64:
        _PARSE_STRING_AND_RETURN_VALUE(INT64);
    case FieldType::DATE:
        _PARSE_STRING_AND_RETURN_VALUE(DATE);
    case FieldType::DATETIME:
        _PARSE_STRING_AND_RETURN_VALUE(DATETIME);
    case FieldType::FLOAT:
        _PARSE_STRING_AND_RETURN_VALUE(FLOAT);
    case FieldType::DOUBLE:
        _PARSE_STRING_AND_RETURN_VALUE(DOUBLE);
    case FieldType::STRING:
        return Value::ConstRef(str);
    case FieldType::BLOB:
        {
            std::string decoded;
            if (!::lgraph_api::base64::TryDecode(str.data(), str.size(), decoded))
                ThrowParseError(str, FieldType::BLOB);
            Value v;
            v.Copy(decoded);
            return v;
        }
    case FieldType::POINT:
        {
            if (!::lgraph_api::TryDecodeEWKB(str, ::lgraph_api::SpatialType::POINT))
                ThrowParseError(str, FieldType::POINT);
            Value v;
            v.Copy(str);
            return v;
        }
    case FieldType::LINESTRING:
        {
            if (!::lgraph_api::TryDecodeEWKB(str, ::lgraph_api::SpatialType::LINESTRING))
                ThrowParseError(str, FieldType::LINESTRING);
            Value v;
            v.Copy(str);
            return v;
        }
    case FieldType::POLYGON:
        {
            if (!::lgraph_api::TryDecodeEWKB(str, ::lgraph_api::SpatialType::POLYGON))
                ThrowParseError(str, FieldType::POLYGON);
            Value v;
            v.Copy(str);
            return v;
        }
    case FieldType::SPATIAL:
        {
            // throw ParseError in this function;
            ::lgraph_api::SpatialType s;
            try {
                s = ::lgraph_api::ExtractType(str);
            } catch (...) {
                ThrowParseError(str, FieldType::SPATIAL);
            }
            if (!::lgraph_api::TryDecodeEWKB(str, s))
                ThrowParseError(str, FieldType::SPATIAL);
            Value v;
            v.Copy(str);
            return v;
        }
    case FieldType::FLOAT_VECTOR:
        {
            std::vector<float> vec;
            // check if there are only numbers and commas
            std::regex nonNumbersAndCommas("[^0-9,.]");
            if (std::regex_search(str, nonNumbersAndCommas)) {
                ThrowParseError(str, FieldType::FLOAT_VECTOR);
            }
            // Check if the string conforms to the following format : 1.000000,2.000000,3.000000,...
            std::regex vector("^(?:[-+]?\\d*(?:\\.\\d+)?)(?:,[-+]?\\d*(?:\\.\\d+)?){1,}$");
            if (!std::regex_match(str, vector)) {
                ThrowParseError(str, FieldType::FLOAT_VECTOR);
            }
            // check if there are 1.000,,2.000 & 1.000,2.000,
            if (str.front() == ',' || str.back() == ',' || str.find(",,") != std::string::npos) {
                ThrowParseError(str, FieldType::FLOAT_VECTOR);
            }
            std::regex pattern("-?[0-9]+\\.?[0-9]*");
            std::sregex_iterator begin_it(str.begin(), str.end(), pattern), end_it;
            while (begin_it != end_it) {
                std::smatch match = *begin_it;
                vec.push_back(std::stof(match.str()));
                ++begin_it;
            }
            Value v;
            v.Copy(vec);
            return v;
        }
    }

    FMA_ASSERT(false);
    return Value();
}

//===============================
// copying between FieldData and Value
//===============================
inline FieldData ValueToFieldData(const Value& v, FieldType ft) {
    switch (ft) {
    case FieldType::NUL:
        return FieldData();
    case FieldType::BOOL:
        return FieldData(v.AsType<bool>());
    case FieldType::INT8:
        return FieldData(v.AsType<int8_t>());
    case FieldType::INT16:
        return FieldData(v.AsType<int16_t>());
    case FieldType::INT32:
        return FieldData(v.AsType<int32_t>());
    case FieldType::INT64:
        return FieldData(v.AsType<int64_t>());
    case FieldType::FLOAT:
        return FieldData(v.AsType<float>());
    case FieldType::DOUBLE:
        return FieldData(v.AsType<double>());
    case FieldType::DATE:
        return FieldData(Date(v.AsType<int32_t>()));
    case FieldType::DATETIME:
        return FieldData(DateTime(v.AsType<int64_t>()));
    case FieldType::STRING:
        return FieldData(v.AsString());
    case FieldType::BLOB:
        return FieldData::Blob(v.AsString());
    case FieldType::POINT:
        {
            std::string ewkb = v.AsString();
            ::lgraph_api::SRID s = ::lgraph_api::ExtractSRID(ewkb);
            switch (s) {
                case ::lgraph_api::SRID::NUL:
                    throw std::runtime_error("cannot convert to Point data!");
                case ::lgraph_api::SRID::WGS84:
                    return FieldData(::lgraph_api::Point<::lgraph_api::Wgs84>(ewkb));
                case ::lgraph_api::SRID::CARTESIAN:
                    return FieldData(::lgraph_api::Point<::lgraph_api::Cartesian>(ewkb));
            }
        }
    case FieldType::LINESTRING:
        {
            std::string ewkb = v.AsString();
            ::lgraph_api::SRID s = ::lgraph_api::ExtractSRID(ewkb);
            switch (s) {
                case ::lgraph_api::SRID::NUL:
                    throw std::runtime_error("cannot convert to Point data!");
                case ::lgraph_api::SRID::WGS84:
                    return FieldData(::lgraph_api::LineString<::lgraph_api::Wgs84>(ewkb));
                case ::lgraph_api::SRID::CARTESIAN:
                    return FieldData(::lgraph_api::LineString<::lgraph_api::Cartesian>(ewkb));
            }
        }
    case FieldType::POLYGON:
        {
            std::string ewkb = v.AsString();
            ::lgraph_api::SRID s = ::lgraph_api::ExtractSRID(ewkb);
            switch (s) {
                case ::lgraph_api::SRID::NUL:
                    throw std::runtime_error("cannot convert to Point data!");
                case ::lgraph_api::SRID::WGS84:
                    return FieldData(::lgraph_api::Polygon<::lgraph_api::Wgs84>(ewkb));
                case ::lgraph_api::SRID::CARTESIAN:
                    return FieldData(::lgraph_api::Polygon<::lgraph_api::Cartesian>(ewkb));
            }
        }
    case FieldType::SPATIAL:
        {
            std::string ewkb = v.AsString();
            ::lgraph_api::SRID s = ::lgraph_api::ExtractSRID(ewkb);
            switch (s) {
                case ::lgraph_api::SRID::NUL:
                    throw std::runtime_error("cannot convert to spatial data!");
                case ::lgraph_api::SRID::WGS84:
                    return FieldData(::lgraph_api::Spatial<::lgraph_api::Wgs84>(ewkb));
                case ::lgraph_api::SRID::CARTESIAN:
                    return FieldData(::lgraph_api::Spatial<::lgraph_api::Cartesian>(ewkb));
            }
        }
    case FieldType::FLOAT_VECTOR:
        return FieldData(v.AsType<std::vector<float>>());
    }
    FMA_ASSERT(false);
    return FieldData();
}

//===============================
// compare
//===============================
template <FieldType T>
inline int ValueCompare(const void* p1, size_t s1, const void* p2, size_t s2) {
    if (s1 == (size_t)32 || s2 == (size_t)32) throw std::runtime_error("cannot compare");
    FMA_DBG_ASSERT(s1 == _detail::FieldTypeSizes[T] && s2 == _detail::FieldTypeSizes[T]);
    typename FieldType2StorageType<T>::type v1, v2;
    memcpy(&v1, p1, s1);
    memcpy(&v2, p2, s1);
    return v1 < v2 ? -1 : v1 > v2 ? 1 : 0;
}

template <>
inline int ValueCompare<FieldType::STRING>(const void* p1, size_t s1, const void* p2, size_t s2) {
    size_t sh = std::min<size_t>(s1, s2);
    int r = memcmp(p1, p2, sh);
    if (r != 0) return r;
    if (s1 > sh) return 1;
    if (s2 > sh) return -1;
    return 0;
}

template <>
inline int ValueCompare<FieldType::BLOB>(const void* p1, size_t s1, const void* p2, size_t s2) {
    return ValueCompare<FieldType::STRING>(p1, s1, p2, s2);
}

template <>
inline int ValueCompare<FieldType::POINT>(const void* p1, size_t s1, const void* p2, size_t s2) {
    return ValueCompare<FieldType::STRING>(p1, s1, p2, s2);
}

template <>
inline int ValueCompare<FieldType::LINESTRING>(const void* p1, size_t s1, const void* p2,
size_t s2) {
    return ValueCompare<FieldType::STRING>(p1, s1, p2, s2);
}

template <>
inline int ValueCompare<FieldType::POLYGON>(const void* p1, size_t s1, const void* p2, size_t s2) {
    return ValueCompare<FieldType::STRING>(p1, s1, p2, s2);
}

template <>
inline int ValueCompare<FieldType::SPATIAL>(const void* p1, size_t s1, const void* p2, size_t s2) {
    return ValueCompare<FieldType::STRING>(p1, s1, p2, s2);
}

template <>
inline int ValueCompare<FieldType::FLOAT_VECTOR>(const void* p1, size_t s1, const void* p2,
                                                 size_t s2) {
    throw std::runtime_error("cannot compare vectors");
}

}  // namespace field_data_helper
}  // namespace lgraph

namespace fma_common {
template <>
inline std::string ToString<::lgraph::FieldSpec>(const ::lgraph::FieldSpec& fd) {
    if (fd.optional) {
        return fma_common::StringFormatter::Format(
            "\\{name={}, type={}, nullable\\}", fd.name,
            ::lgraph::field_data_helper::FieldTypeName(fd.type));
    } else {
        return fma_common::StringFormatter::Format(
            "\\{name={}, type={}\\}", fd.name, ::lgraph::field_data_helper::FieldTypeName(fd.type));
    }
}
}  // namespace fma_common
