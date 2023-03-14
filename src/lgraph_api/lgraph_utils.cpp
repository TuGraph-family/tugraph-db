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

#include <assert.h>
#include <string>
#include <vector>
#include <stdexcept>
#include "fma-common/utils.h"
#include "lgraph/lgraph_utils.h"

namespace lgraph_api {

double get_time() { return fma_common::GetTime(); }

void split_string(std::string origin_string, std::vector<std::string>& sub_strings,
                            std::string string_delimiter = ",") {
    auto start = 0U;
    auto end = origin_string.find(string_delimiter);
    while (end != std::string::npos) {
        sub_strings.push_back(origin_string.substr(start, end-start));
        start = end + string_delimiter.length();
        end = origin_string.find(string_delimiter, start);
    }
    sub_strings.push_back(origin_string.substr(start, end-start));
}

std::string rc4(std::string& input, std::string key, std::string mode = "encrypt") {
    assert(mode == "encrypt" || mode == "decrypt");
    size_t key_length = key.size();
    assert(key_length != 0);

    // inititalize box
    int box[256];
    int i = 0;
    for (i = 0; i < 256; i++) {
        box[i] = i;
    }
    int j = 0;
    for (i = 0; i < 256; i++) {
        j = (j + box[i] + key[i % key_length]) % 256;
        std::swap(box[i], box[j]);
    }

    i = 0;
    j = 0;
    size_t input_length = input.size();
    std::string output = "";
    for (size_t k = 0; k < input_length; k++) {
        i = (i + 1) % 256;
        j = (j + box[i]) % 256;
        std::swap(box[i], box[j]);

        int r = (box[i] + box[j]) % 256;

        output.push_back(input[k] ^ box[r]);
    }
    return output;
}

std::string encode_base64(const std::string input) {
    char encode_table[] = {
            'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',
            'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
            'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',
            'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
            'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',
            'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
            'w', 'x', 'y', 'z', '0', '1', '2', '3',
            '4', '5', '6', '7', '8', '9', '+', '/'
    };

    size_t input_length = input.size();
    size_t output_length = ((input_length + 2) / 3) * 4;
    std::string output;
    output.resize(output_length);
    char * p = const_cast<char*>(output.c_str());

    size_t i = 0;
    for (; i < input_length - 2; i += 3) {
        *p++ = encode_table[(input[i] >> 2) & 0x3F];
        *p++ = encode_table[((input[i] & 0x03) << 4) | ((int)(input[i+1] & 0xF0) >> 4)];
        *p++ = encode_table[((input[i+1] & 0x0F) << 2) | ((int)(input[i+2] & 0xC0) >> 6)];
        *p++ = encode_table[input[i+2] & 0x3F];
    }
    if (i < input_length) {
        *p++ = encode_table[(input[i] >> 2) & 0x3F];
        if (i == (input_length - 1)) {
            *p++ = encode_table[((input[i] & 0x03) << 4)];
            *p++ = '=';
        } else {
            *p++ = encode_table[((input[i] & 0x03) << 4) | ((int)(input[i+1] & 0xF0) >> 4)];
            *p++ = encode_table[((input[i+1] & 0x0F) << 2)];
        }
        *p++ = '=';
    }

    return output;
}

std::string decode_base64(const std::string input) {
    char decode_table[] = {
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 62, 64, 64, 64, 63,
            52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 64, 64, 64, 64, 64, 64,
            64,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
            15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 64, 64, 64, 64, 64,
            64, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
            41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64,
            64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64, 64
    };

    size_t input_length = input.size();
    assert(input_length % 4 == 0);
    size_t output_length = input_length / 4 * 3;
    size_t orig_output_length = output_length;

    if (input[input_length-2] == '=') {
        output_length -= 2;
    } else if (input[input_length-1] == '=') {
        output_length--;
    }
    std::string output;
    output.resize(orig_output_length);
    for (size_t i = 0; i < input_length; i += 4) {
        uint32_t a = input[i] == '=' ? 0 : decode_table[static_cast<int>(input[i])];
        uint32_t b = input[i+1] == '=' ? 0 : decode_table[static_cast<int>(input[i+1])];
        uint32_t c = input[i+2] == '=' ? 0 : decode_table[static_cast<int>(input[i+2])];
        uint32_t d = input[i+3] == '=' ? 0 : decode_table[static_cast<int>(input[i+3])];

        uint32_t triple = (a << 18) + (b << 12) + (c << 6) + d;
        size_t j = i / 4 * 3;
        output[j] = (triple >> 16) & 0xFF;
        output[j+1] = (triple >> 8) & 0xFF;
        output[j+2] = triple & 0xFF;
    }
    output.resize(output_length);
    return output;
}

void* alloc_buffer(size_t bytes) {
    void* buffer;
#if USE_VALGRIND
    buffer = malloc(bytes);
    if (!buffer) {
        throw std::bad_alloc();
    }
#else
    buffer = mmap(nullptr, bytes, PROT_READ | PROT_WRITE,
                  MAP_PRIVATE | MAP_ANONYMOUS | MAP_NORESERVE, -1, 0);
    if (buffer == MAP_FAILED) {
        throw std::runtime_error("memory allocation failed");
    }
#endif
    return buffer;
}

void dealloc_buffer(void* buffer, size_t bytes) {
#if USE_VALGRIND
    free(buffer);
#else
    int error = munmap(buffer, bytes);
    if (error != 0) {
        fprintf(stderr, "warning: potential memory leak!\n");
    }
#endif
}
}  // namespace lgraph_api
