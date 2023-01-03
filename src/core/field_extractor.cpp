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

#include "core/field_extractor.h"

namespace lgraph {
namespace _detail {
/**
 * Parse string data as type and set the field
 *
 * \tparam  T   Type into which the data will be parsed.
 * \param [in,out]  record  The record.
 * \param           data    The string representation of the data. If it is
 * NBytes or String, then the data is stored as-is.
 *
 * \return  ErrorCode::OK if succeeds
 *          FIELD_PARSE_FAILED.
 */
template <FieldType FT>
void FieldExtractor::_ParseStringAndSet(Value& record, const std::string& data) const {
    typedef typename field_data_helper::FieldType2CType<FT>::type CT;
    typedef typename field_data_helper::FieldType2StorageType<FT>::type ST;
    CT s{};
    size_t tmp = fma_common::TextParserUtils::ParseT<CT>(data.data(), data.data() + data.size(), s);
    if (_F_UNLIKELY(tmp != data.size())) throw ParseStringException(Name(), data, FT);
    return SetFixedSizeValue(record, static_cast<ST>(s));
}

template <>
void FieldExtractor::_ParseStringAndSet<FieldType::STRING>(Value& record,
                                                           const std::string& data) const {
    return _SetVariableLengthValue(record, Value::ConstRef(data));
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
void FieldExtractor::ParseAndSet(Value& record, const std::string& data) const {
    if (data.empty() && field_data_helper::IsFixedLengthFieldType(def_.type)) {
        SetIsNull(record, true);
        return;
    }
    // empty string is treated as non-NULL
    SetIsNull(record, false);
    switch (def_.type) {
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
        FMA_ERR() << "ParseAndSet(Value, std::string) is not supposed to"
                     " be called directly. We should first parse blobs "
                     "into BlobValue and use SetBlobField(Value, FieldData)";
    case FieldType::NUL:
        FMA_ERR() << "NUL FieldType";
    }
    FMA_ERR() << "Data type " << field_data_helper::FieldTypeName(def_.type) << " not handled";
}

// parse data from FieldData and set field
// for BLOBs, only formatted data is allowed
void FieldExtractor::ParseAndSet(Value& record, const FieldData& data) const {
    // NULL FieldData is seen as explicitly setting field to NUL
    bool data_is_null = data.type == FieldType::NUL;
    SetIsNull(record, data_is_null);
    if (data_is_null) return;

#define _SET_FIXED_TYPE_VALUE_FROM_FD(ft)                                                     \
    do {                                                                                      \
        if (data.type == def_.type) {                                                         \
            return SetFixedSizeValue(record,                                                  \
                                     field_data_helper::GetStoredValue<FieldType::ft>(data)); \
        } else {                                                                              \
            typename field_data_helper::FieldType2StorageType<FieldType::ft>::type s;         \
            if (!field_data_helper::FieldDataTypeConvert<FieldType::ft>::Convert(data, s))    \
                throw ParseFieldDataException(Name(), data, Type());                          \
            return SetFixedSizeValue(record, s);                                              \
        }                                                                                     \
    } while (0)

    switch (def_.type) {
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
    default:
        FMA_ERR() << "Data type " << field_data_helper::FieldTypeName(def_.type) << " not handled";
    }
}

/**
 * Print the string representation of the field. For digital types, it prints
 * it into ASCII string; for NBytes and String, it just copies the content of
 * the field into the string.
 *
 * \param   record  The record.
 *
 * \return  String representation of the field.
 */
std::string FieldExtractor::FieldToString(const Value& record) const {
    if (GetIsNull(record)) return "\"null\"";
    std::string ret;

#define _COPY_FIELD_AND_RETURN_STR_(record, ft)                                       \
    do {                                                                              \
        typename field_data_helper::FieldType2StorageType<FieldType::ft>::type d = 0; \
        typedef typename field_data_helper::FieldType2CType<FieldType::ft>::type CT;  \
        GetCopy(record, d);                                                           \
        return fma_common::StringFormatter::Format("{}", static_cast<CT>(d));         \
    } while (0)

    switch (def_.type) {
    case FieldType::BOOL:
        _COPY_FIELD_AND_RETURN_STR_(record, BOOL);
    case FieldType::INT8:
        _COPY_FIELD_AND_RETURN_STR_(record, INT8);
    case FieldType::INT16:
        _COPY_FIELD_AND_RETURN_STR_(record, INT16);
    case FieldType::INT32:
        _COPY_FIELD_AND_RETURN_STR_(record, INT32);
    case FieldType::INT64:
        _COPY_FIELD_AND_RETURN_STR_(record, INT64);
    case FieldType::FLOAT:
        _COPY_FIELD_AND_RETURN_STR_(record, FLOAT);
    case FieldType::DOUBLE:
        _COPY_FIELD_AND_RETURN_STR_(record, DOUBLE);
    case FieldType::DATE:
        {
            int32_t i;
            GetCopy(record, i);
            return Date(i).ToString();
        }
    case FieldType::DATETIME:
        {
            int64_t i;
            GetCopy(record, i);
            return DateTime(i).ToString();
        }
    case FieldType::STRING:
        {
            std::string ret(GetDataSize(record), 0);
            GetCopyRaw(record, &ret[0], ret.size());
            return ret;
        }
    case FieldType::BLOB:
        {
            // std::string ret(GetDataSize(record), 0);
            // GetCopyRaw(record, &ret[0], ret.size());
            // return ::lgraph_api::base64::Encode(ret.substr(2));
            return fma_common::StringFormatter::Format("[BLOB]");
        }
    case lgraph_api::NUL:
        break;
    }
    FMA_ERR() << "Data type " << field_data_helper::FieldTypeName(def_.type) << " not handled";
    return "";
}

// sets variable length value to the field
void FieldExtractor::_SetVariableLengthValue(Value& record, const Value& data) const {
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
