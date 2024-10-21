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

#include <cstdint>
#include <cstring>
#include <memory>
#include <stdexcept>
#include "cypher/resultset/bit_mask.h"
#include "cypher/resultset/cypher_string_t.h"

namespace cypher {

constexpr size_t DEFAULT_VECTOR_CAPACITY = 2048;

class ColumnVector {
    friend class StringColumn;

 public:
    explicit ColumnVector(size_t element_size, size_t capacity = DEFAULT_VECTOR_CAPACITY,
                        lgraph_api::FieldType field_type = lgraph_api::FieldType::NUL)
        : element_size_(element_size),
          capacity_(capacity),
          field_type_(field_type),
          data_(new uint8_t[element_size * capacity]()),
          bitmask_(capacity) {}

    ColumnVector(const ColumnVector& other)
        : element_size_(other.element_size_),
        capacity_(other.capacity_),
        field_type_(other.field_type_),
        data_(new uint8_t[other.element_size_ * other.capacity_]),
        bitmask_(other.bitmask_) {
        // Check if the ColumnVector contains strings
        if (element_size_ == sizeof(cypher_string_t)) {
            // Initialize overflow buffer
            if (other.overflow_buffer_) {
                overflow_buffer_capacity_ = other.overflow_buffer_capacity_;
                overflow_buffer_ = std::make_unique<uint8_t[]>(overflow_buffer_capacity_);
                overflow_offset_ = 0;  // will update this as we copy strings
            }
            // Copy each cypher_string_t individually
            for (uint32_t i = 0; i < capacity_; ++i) {
                auto& src_str = reinterpret_cast<cypher_string_t*>(other.data_.get())[i];
                auto& dst_str = reinterpret_cast<cypher_string_t*>(data_.get())[i];
                dst_str.len = src_str.len;
                if (cypher_string_t::IsShortString(src_str.len)) {
                    // Copy the short string directly
                    std::memcpy(dst_str.prefix, src_str.prefix, src_str.len);
                } else {
                    // Copy the prefix
                    std::memcpy(dst_str.prefix, src_str.prefix, cypher_string_t::PREFIX_LENGTH);
                    // Allocate overflow space in the new overflow buffer
                    uint64_t overflow_size = src_str.len - cypher_string_t::PREFIX_LENGTH;
                    if (!overflow_buffer_) {
                        // Initialize overflow buffer if not already done
                        overflow_buffer_capacity_ = std::max(overflow_size,
                                                            static_cast<uint64_t>(1024));
                        overflow_buffer_ = std::make_unique<uint8_t[]>(overflow_buffer_capacity_);
                        overflow_offset_ = 0;
                    } else if (overflow_offset_ + overflow_size > overflow_buffer_capacity_) {
                        // Resize overflow buffer if necessary
                        ResizeOverflowBuffer(overflow_offset_ + overflow_size);
                    }
                    // Copy the overflow data
                    void* dst_overflow_ptr = overflow_buffer_.get() + overflow_offset_;
                    std::memcpy(dst_overflow_ptr, reinterpret_cast<void*>(src_str.overflowPtr),
                                overflow_size);
                    dst_str.overflowPtr = reinterpret_cast<uint64_t>(dst_overflow_ptr);
                    overflow_offset_ += overflow_size;
                }
            }
        } else {
            // For non-string data, we can copy the data directly
            std::memcpy(data_.get(), other.data_.get(), element_size_ * capacity_);
            /* Copy overflow buffer if it exists (though for non-string data, it shouldn't). 
               Just in case for future use. */
            if (other.overflow_buffer_) {
                overflow_buffer_capacity_ = other.overflow_buffer_capacity_;
                overflow_offset_ = other.overflow_offset_;
                overflow_buffer_ = std::make_unique<uint8_t[]>(overflow_buffer_capacity_);
                std::memcpy(overflow_buffer_.get(), other.overflow_buffer_.get(), overflow_offset_);
            }
        }
    }

    ColumnVector& operator=(const ColumnVector& other) {
        if (this == &other) return *this;
        element_size_ = other.element_size_;
        capacity_ = other.capacity_;
        field_type_ = other.field_type_;
        data_ = std::unique_ptr<uint8_t[]>(new uint8_t[other.element_size_ * other.capacity_]);
        std::memcpy(data_.get(), other.data_.get(), other.element_size_ * other.capacity_);
        bitmask_ = other.bitmask_;
        overflow_buffer_capacity_ = other.overflow_buffer_capacity_;
        overflow_offset_ = other.overflow_offset_;
        if (other.overflow_buffer_) {
            overflow_buffer_ = std::unique_ptr<uint8_t[]>(
                                new uint8_t[other.overflow_buffer_capacity_]);
            std::memcpy(overflow_buffer_.get(), other.overflow_buffer_.get(), overflow_offset_);
        } else {
            overflow_buffer_.reset();
        }
        return *this;
    }

    ~ColumnVector() = default;

    void SetAllNull() { bitmask_.SetAllNull(); }
    void SetAllNonNull() { bitmask_.SetAllNonNull(); }
    bool HasNoNullsGuarantee() const { return bitmask_.HasNoNullsGuarantee(); }

    void SetNullRange(uint32_t start, uint32_t len, bool value) {
        bitmask_.SetNullFromRange(start, len, value);
    }

    void SetNull(uint32_t pos, bool is_null) { bitmask_.SetBit(pos, is_null); }

    bool IsNull(uint32_t pos) const { return bitmask_.IsBitSet(pos); }

    uint8_t* data() const { return data_.get(); }

    uint32_t GetElementSize() const { return element_size_; }

    uint32_t GetCapacity() const { return capacity_; }

    lgraph_api::FieldType GetFieldType() const { return field_type_; }

    template<typename T>
    const T& GetValue(uint32_t pos) const {
        if (pos >= capacity_) {
            throw std::out_of_range("Index out of range in GetValue");
        }
        return reinterpret_cast<const T*>(data_.get())[pos];
    }

    template<typename T>
    T& GetValue(uint32_t pos) {
        if (pos >= capacity_) {
            throw std::out_of_range("Index out of range in GetValue");
        }
        return reinterpret_cast<T*>(data_.get())[pos];
    }

    template<typename T>
    void SetValue(uint32_t pos, T val) {
        if (pos >= capacity_) {
            throw std::out_of_range("Index out of range in GetValue");
        }
        reinterpret_cast<T*>(data_.get())[pos] = val;
    }

    void* AllocateOverflow(uint64_t size) const {
        if (!overflow_buffer_) {
            overflow_buffer_capacity_ = std::max(size, static_cast<uint64_t>(1024));
            overflow_buffer_ = std::make_unique<uint8_t[]>(overflow_buffer_capacity_);
            overflow_offset_ = 0;
        } else if (overflow_offset_ + size > overflow_buffer_capacity_) {
            uint64_t new_capacity = overflow_offset_ + size;
            new_capacity = ((new_capacity + 1023) / 1024) * 1024;
            ResizeOverflowBuffer(new_capacity);
        }
        void* ptr = overflow_buffer_.get() + overflow_offset_;
        overflow_offset_ += size;
        return ptr;
    }

    // fetch field size
    static size_t GetFieldSize(lgraph_api::FieldType type) {
        switch (type) {
            case lgraph_api::FieldType::BOOL:
                return sizeof(bool);
            case lgraph_api::FieldType::INT8:
                return sizeof(int8_t);
            case lgraph_api::FieldType::INT16:
                return sizeof(int16_t);
            case lgraph_api::FieldType::INT32:
                return sizeof(int32_t);
            case lgraph_api::FieldType::INT64:
                return sizeof(int64_t);
            case lgraph_api::FieldType::FLOAT:
                return sizeof(float);
            case lgraph_api::FieldType::DOUBLE:
                return sizeof(double);
            default:
                throw std::runtime_error("Unsupported field type");
        }
    }

    // insert data into column vector
    static void InsertIntoColumnVector(ColumnVector* column_vector,
                                       const lgraph_api::FieldData& field,
                                       uint32_t pos) {
        switch (field.type) {
            case lgraph_api::FieldType::BOOL:
                column_vector->SetValue(pos, field.AsBool());
                break;
            case lgraph_api::FieldType::INT8:
                column_vector->SetValue(pos, field.AsInt8());
                break;
            case lgraph_api::FieldType::INT16:
                column_vector->SetValue(pos, field.AsInt16());
                break;
            case lgraph_api::FieldType::INT32:
                column_vector->SetValue(pos, field.AsInt32());
                break;
            case lgraph_api::FieldType::INT64:
                column_vector->SetValue(pos, field.AsInt64());
                break;
            case lgraph_api::FieldType::FLOAT:
                column_vector->SetValue(pos, field.AsFloat());
                break;
            case lgraph_api::FieldType::DOUBLE:
                column_vector->SetValue(pos, field.AsDouble());
                break;
            default:
                throw std::runtime_error("Unsupported field type");
        }
    }

 private:
    void ResizeOverflowBuffer(uint64_t new_capacity) const {
        if (new_capacity <= overflow_buffer_capacity_) return;
        auto new_buffer = std::make_unique<uint8_t[]>(new_capacity);
        if (overflow_buffer_) {
            std::memcpy(new_buffer.get(), overflow_buffer_.get(), overflow_offset_);
        }
        overflow_buffer_ = std::move(new_buffer);
        overflow_buffer_capacity_ = new_capacity;
    }

 private:
    uint32_t element_size_;  // size of each element in bytes
    uint32_t capacity_;  // number of elements
    lgraph_api::FieldType field_type_;
    std::unique_ptr<uint8_t[]> data_;
    BitMask bitmask_;
    mutable uint64_t overflow_buffer_capacity_;
    mutable std::unique_ptr<uint8_t[]> overflow_buffer_ = nullptr;
    mutable uint64_t overflow_offset_;
};


class StringColumn {
 public:
    // add string to vector
    static void AddString(ColumnVector* vector, uint32_t vectorPos, cypher_string_t& srcStr) {
        auto& dstStr = vector->GetValue<cypher_string_t>(vectorPos);
        if (cypher_string_t::IsShortString(srcStr.len)) {
            dstStr.SetShortString(reinterpret_cast<const char*>(srcStr.prefix), srcStr.len);
        } else {
            dstStr.overflowPtr = reinterpret_cast<uint64_t>(vector->AllocateOverflow(srcStr.len));
            dstStr.SetLongString(reinterpret_cast<const char*>(srcStr.prefix), srcStr.len);
        }
    }

    static void AddString(ColumnVector* vector, uint32_t vectorPos, const char* srcStr,
                          uint64_t length) {
        auto& dstStr = vector->GetValue<cypher_string_t>(vectorPos);
        if (cypher_string_t::IsShortString(length)) {
            dstStr.SetShortString(srcStr, length);
        } else {
            dstStr.overflowPtr = reinterpret_cast<uint64_t>(vector->AllocateOverflow(length));
            dstStr.SetLongString(srcStr, length);
        }
    }

    static void AddString(ColumnVector* vector, uint32_t vectorPos, const std::string& srcStr) {
        AddString(vector, vectorPos, srcStr.data(), srcStr.length());
    }

    static cypher_string_t& ReserveString(ColumnVector* vector, uint32_t vectorPos,
                                          uint64_t length) {
        auto& dstStr = vector->GetValue<cypher_string_t>(vectorPos);
        dstStr.len = length;
        if (!cypher_string_t::IsShortString(length)) {
            dstStr.overflowPtr = reinterpret_cast<uint64_t>(vector->AllocateOverflow(length));
        }
        return dstStr;
    }

    static void ReserveString(ColumnVector* vector, cypher_string_t& dstStr, uint64_t length) {
        dstStr.len = length;
        if (!cypher_string_t::IsShortString(length)) {
            dstStr.overflowPtr = reinterpret_cast<uint64_t>(vector->AllocateOverflow(length));
        }
    }

    static void AddString(ColumnVector* vector, cypher_string_t& dstStr, cypher_string_t& srcStr) {
        if (cypher_string_t::IsShortString(srcStr.len)) {
            dstStr.SetShortString(reinterpret_cast<const char*>(srcStr.prefix), srcStr.len);
        } else {
            dstStr.overflowPtr = reinterpret_cast<uint64_t>(vector->AllocateOverflow(srcStr.len));
            dstStr.SetLongString(reinterpret_cast<const char*>(srcStr.prefix), srcStr.len);
        }
    }

    static void AddString(ColumnVector* vector, cypher_string_t& dstStr, const char* srcStr,
                          uint64_t length) {
        if (cypher_string_t::IsShortString(length)) {
            dstStr.SetShortString(srcStr, length);
        } else {
            dstStr.overflowPtr = reinterpret_cast<uint64_t>(vector->AllocateOverflow(length));
            dstStr.SetLongString(srcStr, length);
        }
    }

    static void AddString(ColumnVector* vector, cypher_string_t& dstStr,
                          const std::string& srcStr) {
        AddString(vector, dstStr, srcStr.data(), srcStr.length());
    }
};
}  // namespace cypher
