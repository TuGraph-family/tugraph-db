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

#include <sys/time.h>
#include <sys/mman.h>
#include <math.h>
#include <string>
#include <vector>
#include "tools/json.hpp"
using json = nlohmann::json;

namespace lgraph_api {

/**
 * \brief   Get current time.
 *
 * \return  Digit value of current time.
 */
double get_time();

/**
 * \brief   Split the original string by format.
 *
 * \param   origin_string   original string to be split.
 * \param   sub_strings     split substring.
 * \param   string_delimiter    Split format.
 */
void split_string(std::string origin_string, std::vector<std::string>& sub_strings,
                  std::string string_delimiter);

/**
 * \brief   Encrypt the input string in ac4 format.
 *
 * \param   input   input string.
 * \param   key     encryption key.
 * \param   mode    encryption mode
 * \return  Encrypted string
 */
std::string rc4(std::string& input, std::string key, std::string mode);

/**
 * \brief   Encode the input string in encode_base64 format.
 *
 * \param   input   input string.
 * \return  Encrypted string
 */
std::string encode_base64(const std::string input);

/**
 * \brief   Decode the input string in decode_base64 format.
 *
 * \param   input   input string.
 * \return  Decrypted string.
 */
std::string decode_base64(const std::string input);

/**
 * \brief   Allocate memory with size in byte.
 *
 * \param   bytes   Size in byte of request memory.
 * \return  Pointer of allocated memory.
 */
void* alloc_buffer(size_t bytes);

/**
 * \brief   Free memory with size in byte.
 *
 * \param   buffer  Size in bytes of request memory
 * \param   bytes   Pointer of memory to free.
 */
void dealloc_buffer(void* buffer, size_t bytes);

/*!
 * \fn  inline bool IsDigits(char c)
 *
 * \brief   Query if 'c' is digits (0-9).
 *
 * \param   c   The character.
 *
 * \return  True if is digits, false if not.
 */
inline bool IsDigits(char c) {
#ifdef _WIN32
    static const bool is_digit[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_digit[(uint8_t)c];
#else
    return c >= '0' && c <= '9';
#endif
}

/*!
 * \fn  static inline size_t ParseInt64(const char* b, const char* e, int64_t& d)
 *
 * \brief   Parse an int64_t from string, returning number of bytes parsed.
 *
 * \param        b   Pointer to begining of string.
 * \param        e   Pointer to one past end of string.
 * \param [out]  d   Variable to store the result.
 *
 * \return  Number of bytes parsed.
 */
inline size_t ParseInt64(const char* b, const char* e, int64_t& d) {
    const char* orig = b;
    if (b == e) return 0;
    d = 0;
    bool neg = false;
    if (*b == '-') {
        neg = true;
        b++;
    } else if (*b == '+') {
        neg = false;
        b++;
    }
    if (b == e || !IsDigits(*b)) return 0;
    while (b != e && IsDigits(*b)) d = d * 10 + (*b++ - '0');
    if (neg) d = -d;
    return b - orig;
}

/*!
 * \fn  static inline size_t ParseDouble(const char* b, const char* e, double& d)
 *
 * \brief   Parse a double from string, returning number of bytes parsed.
 *
 * \param       b   Pointer to begining of string.
 * \param       e   Pointer to one past end of string.
 * \param [out] d   Variable to store the result.
 *
 * \return  Number of bytes parsed.
 */
inline size_t ParseDouble(const char* b, const char* e, double& d) {
    const char* orig = b;
    if (b == e) return 0;
    d = 0;
    bool neg = false;
    if (*b == '-') {
        neg = true;
        b++;
    } else if (*b == '+') {
        neg = false;
        b++;
    }
    if (b == e || !IsDigits(*b)) return 0;
    while (b != e && IsDigits(*b)) d = d * 10 + (*b++ - '0');
    if (b != e && *b == '.') {
        b++;
        double tail = 0;
        double mult = 0.1;
        while (b != e && IsDigits(*b)) {
            tail = tail + mult * (*b - '0');
            mult *= 0.1;
            b++;
        }
        d += tail;
    }
    if (b != e && (*b == 'e' || *b == 'E')) {
        // parse scientific presentation
        b++;
        int64_t ord = 0;
        b += ParseInt64(b, e, ord);
        d = d * pow(10, ord);
    }
    if (neg) d = -d;
    return b - orig;
}

/**
 * \brief   Parse parameter from nlohmann::json.
 *
 * \param[out]   value  value to store parameter
 * \param        key    key of the parameter in the input
 * \param        input  input json
 */
template <class DataType>
void parse_from_json(DataType& value, const char* key, json& input) {
    if (!input[key].is_null()) {
        try {
            value = input[key].get<DataType>();
        } catch (std::exception& e) {
            std::string type_name;
            if (std::is_same<DataType, int>::value) {
                type_name = "int";
            } else if (std::is_same<DataType, double>::value) {
                type_name = "double";
            } else if (std::is_same<DataType, std::string>::value) {
                type_name = "string";
            } else if (std::is_same<DataType, int64_t>::value) {
                type_name = "int64";
            } else if (std::is_same<DataType, bool>::value) {
                type_name = "bool";
            } else {
                type_name = typeid(DataType).name();
            }
            throw std::runtime_error(std::string(key) + " need to be " + type_name);
        }
    }
}

/**
 * \brief   Parse vector parameter from nlohmann::json.
 *
 * \param[out]   value  value to store parameter
 * \param        key    key of the parameter in the input
 * \param        input  input json
 */
template <class DataType>
void parse_from_json(std::vector<DataType>& value, const char* key, json& input) {
    if (!input[key].is_null()) {
        try {
            value = input[key].get<std::vector<DataType>>();
        } catch (std::exception& e) {
            std::string type_name;
            if (std::is_same<DataType, int>::value) {
                type_name = "vector<int>";
            } else if (std::is_same<DataType, double>::value) {
                type_name = "vector<double>";
            } else if (std::is_same<DataType, std::string>::value) {
                type_name = "vector<string>";
            } else if (std::is_same<DataType, int64_t>::value) {
                type_name = "vector<int64>";
            } else if (std::is_same<DataType, bool>::value) {
                type_name = "vector<bool>";
            } else {
                type_name = "vector<" + std::string(typeid(DataType).name()) + ">";
            }
            throw std::runtime_error(std::string(key) + " need to be " + type_name);
        }
    }
}

}  // namespace lgraph_api
