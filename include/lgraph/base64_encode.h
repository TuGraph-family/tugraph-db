//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/**
 *  @file base64_encode.h
 *  @brief Base64 encode and decode. Simple implementation, to be used with BLOB fields.
 *         If performance is important, look for an optimized implementation.
 */

#pragma once

#include <stdexcept>
#include <string>

#include "lgraph/lgraph_exceptions.h"

namespace lgraph_api {
namespace base64 {

/**
 * @brief   Encodes a string to Base64.
 *
 * @param   p   The string to encode.
 * @param   s   Size of the string.
 *
 * @returns The encoded string.
 */
inline std::string Encode(const char* p, size_t s) {
    static const char* tbl =
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz"
        "0123456789+/";
    size_t ngroup = (s + 2) / 3;
    std::string ret(ngroup * 4, '=');
    for (size_t g = 0; g < s / 3; g++) {
        // handle a full group
        const uint8_t* sp = (const uint8_t*)p + g * 3;
        char* dp = &ret[g * 4];
        dp[0] = tbl[sp[0] >> 2];
        dp[1] = tbl[((sp[0] & 0x3) << 4) + ((sp[1] & 0xF0) >> 4)];
        dp[2] = tbl[((sp[1] & 0x0F) << 2) + (sp[2] >> 6)];
        dp[3] = tbl[sp[2] & 0x3F];
    }
    // handle left-overs
    size_t ns = s % 3;
    if (ns != 0) {
        char* dp = &ret[(ngroup - 1) * 4];
        const uint8_t* sp = (const uint8_t*)p + (s - ns);
        dp[0] = tbl[sp[0] >> 2];
        if (ns == 1) {
            dp[1] = tbl[(sp[0] & 0x3) << 4];
            return ret;
        }
        // ns == 2
        dp[1] = tbl[((sp[0] & 0x3) << 4) + ((sp[1] & 0xF0) >> 4)];
        dp[2] = tbl[(sp[1] & 0x0F) << 2];
    }
    return ret;
}

/**
 * @brief   Encodes a string to Base64.
 *
 * @param   str The string to decode.
 *
 * @returns The decoded string.
 */
inline std::string Encode(const std::string& str) { return Encode(str.data(), str.size()); }

/**
 * @brief   Tries to decode a Base64 string.
 *
 * @param       p   The string to decode.
 * @param       s   Size of the string.
 * @param [out] ret The decoded string.
 *
 * @returns True if the string was decoded successfully, false otherwise.
 */
inline bool TryDecode(const char* p, size_t s, std::string& ret) {
    static const uint8_t tbl[256] = {
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 62,  128, 128, 128,  63,
         52,  53,  54,  55,  56,  57,  58,  59,  60,  61, 128, 128, 128,   0, 128, 128,
        128,   0,   1,   2,   3,   4,   5,   6,   7,   8,   9,  10,  11,  12,  13,  14,
         15,  16,  17,  18,  19,  20,  21,  22,  23,  24,  25, 128, 128, 128, 128, 128,
        128,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,  36,  37,  38,  39,  40,
         41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
        128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128, 128,
    };

    if (s % 4 != 0) return false;
    ret.resize((s / 4) * 3);
    if (s == 0) return true;
    size_t npad = 0;
    if (p[s - 1] == '=') {
        npad = (p[s - 2] == '=') ? 2 : 1;
    }
    for (size_t g = 0; g < s / 4; g++) {
        const uint8_t* sp = (uint8_t*)p + g * 4;
        uint8_t* dp = (uint8_t*)&ret[0] + g * 3;
        uint8_t s0 = tbl[sp[0]];
        uint8_t s1 = tbl[sp[1]];
        uint8_t s2 = tbl[sp[2]];
        uint8_t s3 = tbl[sp[3]];
        if ((s0 | s1 | s2 | s3) >= 128) {
            return false;  // check that all are valid
        }
        dp[0] = (s0 << 2) + ((s1 & 0x30) >> 4);
        dp[1] = ((s1 & 0x0F) << 4) + ((s2 & 0x3C) >> 2);
        dp[2] = ((s2 & 0x03) << 6) + s3;
    }
    ret.resize(ret.size() - npad);  // remove padding
    return true;
}

/**
 * @brief   Tries to decode a Base64 string.
 *
 * @param       str The string to decode.
 * @param [out] out The decoded string.
 *
 * @returns True if the string was decoded successfully, false otherwise.
 */
inline bool TryDecode(const std::string& str, std::string& out) {
    return TryDecode(str.data(), str.size(), out);
}

/**
 * @brief   Decode a Base64 string and throw exception if fails.
 *
 * @exception   InputError  Thrown if the string is not a valid Base64 string.
 *
 * @param   p   The string to decode.
 * @param   s   Size of the string.
 *
 * @returns The decoded string.
 */
inline std::string Decode(const char* p, size_t s) {
    std::string ret;
    if (!TryDecode(p, s, ret)) THROW_CODE(InputError, "Failed to decode Base64 string.");
    return ret;
}

/**
 * @brief   Decode a Base64 string and throw exception if fails.
 *
 * @exception   InputError  Thrown if the string is not a valid Base64 string.
 *
 * @param   str The string to decode.
 *
 * @returns The decoded string.
 */
inline std::string Decode(const std::string& str) { return Decode(str.data(), str.size()); }
}  // namespace base64
}  // namespace lgraph_api
