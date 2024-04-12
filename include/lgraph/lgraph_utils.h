//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

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

/**
 * \brief   Parse vid from the node passed in by cypher. For V2 procedure.
 *
 * \param[in]   node_string  node
 * \return      vid
 */
size_t GetVidFromNodeString(const std::string& node_string);

}  // namespace lgraph_api
