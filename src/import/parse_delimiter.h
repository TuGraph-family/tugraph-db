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

#include "core/data_type.h"
#include "fma-common/text_parser.h"

namespace lgraph {
// parse delimiter and process strings like \r \n \002 \0xA
inline std::string ParseDelimiter(const std::string& delimiter) {
    std::string ret;
    const char* b = delimiter.data();
    const char* e = delimiter.data() + delimiter.size();
    const char* p = b;
    while (p < e) {
        if (*p == '\\') {
            const char* start_p = p;
            p++;
            if (p >= e) {
                THROW_CODE(InputError, "Illegal escape sequence, do you mean \\\\?");
            }
            switch (*p) {
            case ('\\'):
                {
                    ret.push_back('\\');
                    p++;
                    break;
                }
            case ('a'):
                {
                    ret.push_back('\a');
                    p++;
                    break;
                }
            case ('f'):
                {
                    ret.push_back('\f');
                    p++;
                    break;
                }
            case ('n'):
                {
                    ret.push_back('\n');
                    p++;
                    break;
                }
            case ('r'):
                {
                    ret.push_back('\r');
                    p++;
                    break;
                }
            case ('t'):
                {
                    ret.push_back('\t');
                    p++;
                    break;
                }
            case ('v'):
                {
                    ret.push_back('\v');
                    p++;
                    break;
                }
            case ('x'):
                {
                    // \xnn hex numbers
                    p++;
                    uint8_t c = 0;
                    for (int i = 0; i < 2; i++) {
                        if (p >= e)
                            THROW_CODE(InputError,
                                       "Illegal escape sequence: " + std::string(start_p, p));
                        if (*p >= '0' && *p <= '9')
                            c = c * 16 + *p - '0';
                        else if (*p >= 'a' && *p <= 'f')
                            c = c * 16 + (*p - 'a' + 10);
                        else if (*p >= 'A' && *p <= 'F')
                            c = c * 16 + (*p - 'A' + 10);
                        else
                            THROW_CODE(InputError, "Illegal escape sequence: " +
                                             std::string(start_p, p + 1));
                        p++;
                    }
                    ret.push_back((char)c);
                    break;
                }
            default:
                {
                    if (*p >= '0' && *p <= '9') {
                        // \nnn octal numbers
                        uint16_t c = 0;
                        for (int i = 0; i < 3; i++) {
                            if (p >= e)
                                THROW_CODE(InputError, "Illegal escape sequence: " +
                                                 std::string(start_p, p));
                            if (*p < '0' || *p > '7')
                                THROW_CODE(InputError, "Illegal escape sequence: " +
                                                 std::string(start_p, p + 1));
                            c = c * 8 + *p - '0';
                            p++;
                        }
                        if (c >= 256)
                            THROW_CODE(InputError,
                                       "Illegal escape sequence: " + std::string(start_p, p));
                        ret.push_back((char)c);
                        break;
                    } else {
                        THROW_CODE(InputError,
                                   "Illegal escape sequence: " + std::string(start_p, p + 1));
                    }
                }
            }
        } else {
            ret.push_back(*p);
            p++;
        }
    }
    return ret;
}
}  // namespace lgraph
