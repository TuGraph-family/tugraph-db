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

#include "core/field_extractor_v1.h"

namespace lgraph {

namespace _detail {

bool FieldExtractorV1::GetIsNull(const Value& record) const {
    if (!IsOptional()) {
        return false;
    } else {
        // get the Kth bit from NullArray
        char* arr = GetNullArray(record);
        return arr[null_bit_off_ / 8] & (0x1 << (null_bit_off_ % 8));
    }
}

// set field value to null
void FieldExtractorV1::SetIsNull(const Value& record, bool is_null) const {
    if (!IsOptional()) {
        if (is_null) throw FieldCannotBeSetNullException(Name());
        return;
    }
    // set the Kth bit from NullArray
    char* arr = GetNullArray(record);
    if (is_null) {
        arr[null_bit_off_ / 8] |= (0x1 << (null_bit_off_ % 8));
    } else {
        arr[null_bit_off_ / 8] &= ~(0x1 << (null_bit_off_ % 8));
    }
}

template <FieldType FT>
void FieldExtractorV1::_ParseStringAndSet(Value& record, const std::string& data) const {
    typedef typename field_data_helper::FieldType2CType<FT>::type CT;
    typedef typename field_data_helper::FieldType2StorageType<FT>::type ST;
    CT s{};
    size_t tmp = fma_common::TextParserUtils::ParseT<CT>(data.data(), data.data() + data.size(), s);
    if (_F_UNLIKELY(tmp != data.size())) throw ParseStringException(Name(), data, FT);
    return SetFixedSizeValue(record, static_cast<ST>(s));
}

template <>
void FieldExtractorV1::_ParseStringAndSet<FieldType::STRING>(Value& record,
                                                           const std::string& data) const {
    return _SetVariableLengthValue(record, Value::ConstRef(data));
}

template <>
void FieldExtractorV1::_ParseStringAndSet<FieldType::POINT>(Value& record,
                                                          const std::string& data) const {
    FMA_DBG_ASSERT(!is_vfield_);
    // check whether the point data is valid;
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::POINT))
        throw ParseStringException(Name(), data, FieldType::POINT);
    // FMA_DBG_CHECK_EQ(sizeof(data), field_data_helper::FieldTypeSize(Type()));
    size_t Size = record.Size();
    record.Resize(Size);
    char* ptr = (char*)record.Data() + offset_.data_off;
    memcpy(ptr, data.data(), 50);
}

template <>
void FieldExtractorV1::_ParseStringAndSet<FieldType::LINESTRING>(Value& record,
                                                               const std::string& data) const {
    // check whether the linestring data is valid;
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::LINESTRING))
        throw ParseStringException(Name(), data, FieldType::LINESTRING);
    return _SetVariableLengthValue(record, Value::ConstRef(data));
}

template <>
void FieldExtractorV1::_ParseStringAndSet<FieldType::POLYGON>(Value& record,
                                                            const std::string& data) const {
    if (!::lgraph_api::TryDecodeEWKB(data, ::lgraph_api::SpatialType::POLYGON))
        throw ParseStringException(Name(), data, FieldType::POLYGON);
    return _SetVariableLengthValue(record, Value::ConstRef(data));
}

template <>
void FieldExtractorV1::_ParseStringAndSet<FieldType::SPATIAL>(Value& record,
                                                            const std::string& data) const {
    ::lgraph_api::SpatialType s;
    // throw ParseStringException in this function;
    try {
        s = ::lgraph_api::ExtractType(data);
    } catch (...) {
        throw ParseStringException(Name(), data, FieldType::SPATIAL);
    }

    if (!::lgraph_api::TryDecodeEWKB(data, s))
        throw ParseStringException(Name(), data, FieldType::SPATIAL);
    return _SetVariableLengthValue(record, Value::ConstRef(data));
}

template <>
void FieldExtractorV1::_ParseStringAndSet<FieldType::FLOAT_VECTOR>(Value& record,
                                                                 const std::string& data) const {
    std::vector<float> vec;
    // check if there are only numbers and commas
    std::regex nonNumbersAndCommas("[^0-9,.]");
    if (std::regex_search(data, nonNumbersAndCommas)) {
        throw ParseStringException(Name(), data, FieldType::FLOAT_VECTOR);
    }
    // Check if the string conforms to the following format : 1.000000,2.000000,3.000000,...
    std::regex vector("^(?:[-+]?\\d*(?:\\.\\d+)?)(?:,[-+]?\\d*(?:\\.\\d+)?){1,}$");
    if (!std::regex_match(data, vector)) {
        throw ParseStringException(Name(), data, FieldType::FLOAT_VECTOR);
    }
    // check if there are 1.000,,2.000 & 1.000,2.000,
    if (data.front() == ',' || data.back() == ',' || data.find(",,") != std::string::npos) {
        throw ParseStringException(Name(), data, FieldType::FLOAT_VECTOR);
    }
    std::regex pattern("-?[0-9]+\\.?[0-9]*");
    std::sregex_iterator begin_it(data.begin(), data.end(), pattern), end_it;
    while (begin_it != end_it) {
        std::smatch match = *begin_it;
        vec.push_back(std::stof(match.str()));
        ++begin_it;
    }
    if (vec.size() <= 0) throw ParseStringException(Name(), data, FieldType::FLOAT_VECTOR);
    return _SetVariableLengthValue(record, Value::ConstRef(vec));
}
/**
 * Parse the string data and set the field
 *
 * \param [in,out]  record  The record.
 * \param           data    The string representation of the data.
 *
 * \return  ErrorCode::OK if succeeds, or
 *          FIELD_CANNOT_BE_NULL
 *          DATA_SIZE_TOO_LARGE  if record size exceeds limit (currently 2^32)
 *          DATA_RANGE_OVERFLOW   if record size overflow
 *          FIELD_PARSE_FAILED.
 */
void FieldExtractorV1::ParseAndSet(Value& record, const std::string& data) const {
    if (data.empty() &&
        (IsFixedType() || Type() == FieldType::LINESTRING || Type() == FieldType::POLYGON ||
         Type() == FieldType::SPATIAL || Type() == FieldType::FLOAT_VECTOR)) {
        SetIsNull(record, true);
        return;
    }
    // empty string is treated as non-NULL
    SetIsNull(record, false);
    switch (Type()) {
    case FieldType::BOOL:
        return _ParseStringAndSet<FieldType::BOOL>(record, data);
    case FieldType::INT8:
        return _ParseStringAndSet<FieldType::INT8>(record, data);
    case FieldType::INT16:
        return _ParseStringAndSet<FieldType::INT16>(record, data);
    case FieldType::INT32:
        return _ParseStringAndSet<FieldType::INT32>(record, data);
    case FieldType::INT64:
        return _ParseStringAndSet<FieldType::INT64>(record, data);
    case FieldType::FLOAT:
        return _ParseStringAndSet<FieldType::FLOAT>(record, data);
    case FieldType::DOUBLE:
        return _ParseStringAndSet<FieldType::DOUBLE>(record, data);
    case FieldType::DATE:
        return _ParseStringAndSet<FieldType::DATE>(record, data);
    case FieldType::DATETIME:
        return _ParseStringAndSet<FieldType::DATETIME>(record, data);
    case FieldType::STRING:
        return _ParseStringAndSet<FieldType::STRING>(record, data);
    case FieldType::BLOB:
        LOG_ERROR() << "ParseAndSet(Value, std::string) is not supposed to"
                       " be called directly. We should first parse blobs "
                       "into BlobValue and use SetBlobField(Value, FieldData)";
    case FieldType::POINT:
        return _ParseStringAndSet<FieldType::POINT>(record, data);
    case FieldType::LINESTRING:
        return _ParseStringAndSet<FieldType::LINESTRING>(record, data);
    case FieldType::POLYGON:
        return _ParseStringAndSet<FieldType::POLYGON>(record, data);
    case FieldType::SPATIAL:
        return _ParseStringAndSet<FieldType::SPATIAL>(record, data);
    case FieldType::FLOAT_VECTOR:
        return _ParseStringAndSet<FieldType::FLOAT_VECTOR>(record, data);
    case FieldType::NUL:
        LOG_ERROR() << "NUL FieldType";
    }
    LOG_ERROR() << "Data type " << field_data_helper::FieldTypeName(Type()) << " not handled";
}

// parse data from FieldData and set field
// for BLOBs, only formatted data is allowed
void FieldExtractorV1::ParseAndSet(Value& record, const FieldData& data) const {
    // NULL FieldData is seen as explicitly setting field to NUL
    bool data_is_null = data.type == FieldType::NUL;
    SetIsNull(record, data_is_null);
    if (data_is_null) return;

#define _SET_FIXED_TYPE_VALUE_FROM_FD(ft)                                                     \
    do {                                                                                      \
        if (data.type == Type()) {                                                            \
            return SetFixedSizeValue(record,                                                  \
                                     field_data_helper::GetStoredValue<FieldType::ft>(data)); \
        } else {                                                                              \
            typename field_data_helper::FieldType2StorageType<FieldType::ft>::type s;         \
            if (!field_data_helper::FieldDataTypeConvert<FieldType::ft>::Convert(data, s))    \
                throw ParseFieldDataException(Name(), data, Type());                          \
            return SetFixedSizeValue(record, s);                                              \
        }                                                                                     \
    } while (0)

    switch (Type()) {
    case FieldType::BOOL:
        _SET_FIXED_TYPE_VALUE_FROM_FD(BOOL);
    case FieldType::INT8:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT8);
    case FieldType::INT16:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT16);
    case FieldType::INT32:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT32);
    case FieldType::INT64:
        _SET_FIXED_TYPE_VALUE_FROM_FD(INT64);
    case FieldType::DATE:
        _SET_FIXED_TYPE_VALUE_FROM_FD(DATE);
    case FieldType::DATETIME:
        _SET_FIXED_TYPE_VALUE_FROM_FD(DATETIME);
    case FieldType::FLOAT:
        _SET_FIXED_TYPE_VALUE_FROM_FD(FLOAT);
    case FieldType::DOUBLE:
        _SET_FIXED_TYPE_VALUE_FROM_FD(DOUBLE);
    case FieldType::STRING:
        if (data.type != FieldType::STRING)
            throw ParseIncompatibleTypeException(Name(), data.type, FieldType::STRING);
        return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf));
    case FieldType::BLOB:
        {
            // used in AlterLabel, when copying old blob value to new
            // In this case, the value must already be correctly formatted, so just copy it
            if (data.type != FieldType::BLOB)
                throw ParseIncompatibleTypeException(Name(), data.type, FieldType::BLOB);
            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf));
        }
    case FieldType::POINT:
        {
            // point type can only be converted from point and string;
            if (data.type != FieldType::POINT && data.type != FieldType::STRING)
                throw ParseFieldDataException(Name(), data, Type());
            FMA_DBG_ASSERT(!is_vfield_);
            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, ::lgraph_api::SpatialType::POINT))
                throw ParseStringException(Name(), *data.data.buf, FieldType::POINT);

            record.Resize(record.Size());
            char* ptr = (char*)record.Data() + offset_.data_off;
            memcpy(ptr, (*data.data.buf).data(), 50);
            return;
        }
    case FieldType::LINESTRING:
        {
            if (data.type != FieldType::LINESTRING && data.type != FieldType::STRING)
                throw ParseFieldDataException(Name(), data, Type());
            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, ::lgraph_api::SpatialType::LINESTRING))
                throw ParseStringException(Name(), *data.data.buf, FieldType::LINESTRING);

            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf));
        }
    case FieldType::POLYGON:
        {
            if (data.type != FieldType::POLYGON && data.type != FieldType::STRING)
                throw ParseFieldDataException(Name(), data, Type());
            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, ::lgraph_api::SpatialType::POLYGON))
                throw ParseStringException(Name(), *data.data.buf, FieldType::POLYGON);

            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf));
        }
    case FieldType::SPATIAL:
        {
            if (data.type != FieldType::SPATIAL && data.type != FieldType::STRING)
                throw ParseFieldDataException(Name(), data, Type());
            ::lgraph_api::SpatialType s;

            // throw ParseStringException in this function;
            try {
                s = ::lgraph_api::ExtractType(*data.data.buf);
            } catch (...) {
                throw ParseStringException(Name(), *data.data.buf, FieldType::SPATIAL);
            }

            if (!::lgraph_api::TryDecodeEWKB(*data.data.buf, s))
                throw ParseStringException(Name(), *data.data.buf, FieldType::SPATIAL);

            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.buf));
        }
    case FieldType::FLOAT_VECTOR:
        {
            if (data.type != FieldType::FLOAT_VECTOR) {
                throw ParseFieldDataException(Name(), data, Type());
            }
            return _SetVariableLengthValue(record, Value::ConstRef(*data.data.vp));
        }
    default:
        LOG_ERROR() << "Data type " << field_data_helper::FieldTypeName(Type()) << " not handled";
    }
}

// sets variable length value to the field
void FieldExtractorV1::_SetVariableLengthValue(Value& record, const Value& data) const {
    FMA_DBG_ASSERT(is_vfield_);
    if (data.Size() > _detail::MAX_STRING_SIZE)
        throw DataSizeTooLargeException(Name(), data.Size(), _detail::MAX_STRING_SIZE);
    size_t foff = GetFieldOffset(record);
    size_t fsize = GetDataSize(record);
    // realloc record with original size to make sure we own the memory
    if (fsize > data.Size()) {
        // shrinking, move before realloc
        size_t diff = fsize - data.Size();
        record.Resize(record.Size());
        char* rptr = (char*)record.Data();
        memmove(rptr + foff + data.Size(), rptr + foff + fsize, record.Size() - (foff + fsize));
        record.Resize(record.Size() - diff);
        rptr = (char*)record.Data();
        memcpy(rptr + foff, data.Data(), data.Size());
        // adjust offset of other fields
        /** Note we store only n-1 offsets, since the first offset is always
         * known
         */
        char* offsets = rptr + offset_.v_offs;
        for (size_t i = offset_.idx; i < offset_.last_idx; i++) {
            char* ptr = offsets + i * sizeof(DataOffset);
            DataOffset off = ::lgraph::_detail::UnalignedGet<DataOffset>(ptr);
            FMA_DBG_CHECK_GE(off, (DataOffset)diff);
            off -= static_cast<DataOffset>(diff);
            ::lgraph::_detail::UnalignedSet<DataOffset>(ptr, off);
        }
    } else {
        // expanding, realloc before move
        size_t orig_rsize = record.Size();
        size_t diff = data.Size() - fsize;
        if (orig_rsize + diff > _detail::MAX_PROP_SIZE)
            throw RecordSizeLimitExceededException(Name(), orig_rsize + diff,
                                                   _detail::MAX_PROP_SIZE);
        record.Resize(orig_rsize + diff);
        char* rptr = (char*)record.Data();
        memmove(rptr + foff + data.Size(), rptr + foff + fsize, orig_rsize - (foff + fsize));
        memcpy(rptr + foff, data.Data(), data.Size());
        // adjust offset of other fields
        char* offsets = rptr + offset_.v_offs;
        for (size_t i = offset_.idx; i < offset_.last_idx; i++) {
            char* ptr = offsets + i * sizeof(DataOffset);
            size_t new_off = diff + ::lgraph::_detail::UnalignedGet<DataOffset>(ptr);
            ::lgraph::_detail::UnalignedSet<DataOffset>(ptr, static_cast<DataOffset>(new_off));
        }
    }
}

}  // namespace _detail
}  // namespace lgraph
