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
#include <string>
#include <boost/endian/conversion.hpp>
#include "lgraph/lgraph_types.h"
#include "core/field_data_helper.h"

namespace lgraph {
namespace import_v3 {

template<class T>
static void encodeNumToStr(T num, std::string& ret) {
    static_assert(std::is_same_v<T, int8_t> ||
                  std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, float> ||
                  std::is_same_v<T, double>);

    if constexpr(std::is_same_v<T, int8_t>) {
        uint8_t val = uint8_t(num ^ (uint8_t)1<<7);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, int16_t>) {
        uint16_t val = uint16_t(num ^ (uint16_t)1<<15);
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, int32_t>) {
        uint32_t val = uint32_t(num ^ (uint32_t)1<<31);
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, int64_t>) {
        uint64_t val = uint64_t(num ^ (uint64_t)1<<63);
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, float>) {
        uint32_t val = *(uint32_t*)&num;
        if (num >= 0) {
            val |= (uint32_t)1<<31;
        } else {
            val = ~val;
        }
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    } else if constexpr(std::is_same_v<T, double>) {
        uint64_t val = *(uint64_t*)&num;
        if (num >= 0) {
            val |= (uint64_t)1<<63;
        } else {
            val = ~val;
        }
        boost::endian::native_to_big_inplace(val);
        ret.append((const char*)&val, sizeof(val));
    }
}

template<class T>
static T decodeStrToNum(const char* str) {
    static_assert(std::is_same_v<T, int8_t> ||
                  std::is_same_v<T, int16_t> ||
                  std::is_same_v<T, int32_t> ||
                  std::is_same_v<T, int64_t> ||
                  std::is_same_v<T, float> ||
                  std::is_same_v<T, double>);

    if constexpr(std::is_same_v<T, int8_t>) {
        uint8_t newVal = *(uint8_t*)str;
        return int8_t(newVal ^ (uint8_t)1<<7);
    } else if constexpr(std::is_same_v<T, int16_t>) {
        uint16_t newVal = *(uint16_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        return int16_t(newVal ^ (uint16_t)1<<15);
    } else if constexpr(std::is_same_v<T, int32_t>) {
        uint32_t newVal = *(uint32_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        return int32_t(newVal ^ (uint32_t)1<<31);
    } else if constexpr(std::is_same_v<T, int64_t>) {
        uint64_t newVal = *(uint64_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        return int64_t(newVal ^ (uint64_t)1<<63);
    } else if constexpr(std::is_same_v<T, float>) {
        uint32_t newVal = *(uint32_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        if ((newVal & (uint32_t)1<<31) > 0) {
            newVal &= ~((uint32_t)1<<31);
        } else {
            newVal = ~newVal;
        }
        return *(float*)(&newVal);
    } else if constexpr(std::is_same_v<T, double>) {
        uint64_t newVal = *(uint64_t*)str;
        boost::endian::big_to_native_inplace(newVal);
        if ((newVal & (uint64_t)1<<63) > 0) {
            newVal &= ~((uint64_t)1<<63);
        } else {
            newVal = ~newVal;
        }
        return *(double*)(&newVal);
    }
}

static void AppendFieldData(std::string& ret, const FieldData& data) {
    switch (data.GetType()) {
    case FieldType::NUL:
        FMA_ASSERT(false);
        break;
    case FieldType::BOOL:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::BOOL>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::FLOAT:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::FLOAT>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::DOUBLE:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::DOUBLE>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::DATE:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::DATE>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::DATETIME:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::DATETIME>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT64:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT64>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT32:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT32>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT16:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT16>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::INT8:
        {
            auto val = field_data_helper::GetStoredValue<FieldType::INT8>(data);
            encodeNumToStr(val, ret);
        }
        break;
    case FieldType::STRING:
        {
            const auto& val = field_data_helper::GetStoredValue<FieldType::STRING>(data);
            ret.append(val);
        }
        break;
    case FieldType::BLOB:
        {
            const auto& val = field_data_helper::GetStoredValue<FieldType::BLOB>(data);
            ret.append(val);
        }
        break;
    default:
        FMA_ASSERT(false);
        break;
    }
}

}  // namespace import_v3
}  // namespace lgraph
