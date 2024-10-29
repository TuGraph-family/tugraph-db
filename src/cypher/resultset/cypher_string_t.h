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

    static bool IsShortString(uint32_t len) { return len <= SHORT_STR_LENGTH; }

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
};

}  // namespace cypher
