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
#include <string>
#include <string_view>

namespace cypher {

struct cypher_string_t {
    static constexpr uint64_t PREFIX_LENGTH = 4;
    static constexpr uint64_t INLINED_SUFFIX_LENGTH = 8;
    static constexpr uint64_t SHORT_STR_LENGTH = PREFIX_LENGTH + INLINED_SUFFIX_LENGTH;

    uint32_t len;
    uint8_t prefix[PREFIX_LENGTH];
    union {
        uint8_t data[INLINED_SUFFIX_LENGTH];
        uint64_t overflowPtr;
    };

    cypher_string_t() : len{0}, overflowPtr{0} {}
    cypher_string_t(const char* value, uint64_t length) { set(value, length); }

    static bool IsShortString(uint32_t len) { return len <= SHORT_STR_LENGTH; }

    const uint8_t* GetData() const {
        return IsShortString(len) ? prefix : reinterpret_cast<uint8_t*>(overflowPtr);
    }

    uint8_t* GetDataUnsafe() {
        return IsShortString(len) ? prefix : reinterpret_cast<uint8_t*>(overflowPtr);
    }

    void set(const std::string& value) { set(value.data(), value.length()); }
    void set(const char* value, uint64_t length) {
        len = length;
        if (IsShortString(length)) {
            std::memcpy(prefix, value, length);
        } else {
            std::memcpy(prefix, value, PREFIX_LENGTH);
            overflowPtr = reinterpret_cast<uint64_t>(value + PREFIX_LENGTH);
        }
    }

    void SetShortString(const char* value, uint64_t length) {
        len = length;
        std::memcpy(prefix, value, length);
    }

    void SetLongString(const char* value, uint64_t length) {
        len = length;
        std::memcpy(prefix, value, PREFIX_LENGTH);
        std::memcpy(reinterpret_cast<char*>(overflowPtr), value + PREFIX_LENGTH,
                    length - PREFIX_LENGTH);
    }

    std::string GetAsShortString() const {
        return std::string(reinterpret_cast<const char*>(prefix), len);
    }

    std::string GetAsString() const {
        if (IsShortString(len)) {
            return std::string(reinterpret_cast<const char*>(prefix), len);
        } else {
            return std::string(reinterpret_cast<const char*>(prefix), PREFIX_LENGTH) +
                   std::string(reinterpret_cast<const char*>(overflowPtr), len - PREFIX_LENGTH);
        }
    }

    bool operator==(const cypher_string_t& rhs) const {
        return len == rhs.len && std::memcmp(GetData(), rhs.GetData(), len) == 0;
    }

    bool operator!=(const cypher_string_t& rhs) const { return !(*this == rhs); }

    bool operator>(const cypher_string_t& rhs) const {
        return std::lexicographical_compare(rhs.GetData(), rhs.GetData() + rhs.len,
                                            GetData(), GetData() + len);
    }

    bool operator>=(const cypher_string_t& rhs) const { return (*this > rhs) || (*this == rhs); }
    bool operator<(const cypher_string_t& rhs) const { return !(*this >= rhs); }
    bool operator<=(const cypher_string_t& rhs) const { return !(*this > rhs); }
};

}  // namespace cypher
