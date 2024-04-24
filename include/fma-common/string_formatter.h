//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include <exception>

#include "tools/lgraph_log.h"
#include "fma-common/string_util.h"

namespace fma_common {
/*!
 * \class   StringFormatter
 *
 * \brief   A string formatter which can be used like Python formatters.
 *          Example:
 *              std::string buf;
 *              buf.reserve(1024);
 *              StringFormatter.format(buf, "{} {} {}", a, b, c);
 */
class StringFormatter {
 private:
    // TODO(any): check number of fields in compile time
    // TODO(any): support \{ and \}
    template <typename T, typename... Ts>
    static bool MyPrintf(std::string& buf, const char* format, const T& d, const Ts&... ds) {
        const char* p = format;
        while (*p != '\0') {
            if (*p == '\\' && (*(p + 1) == '{' || *(p + 1) == '}')) {
                buf.push_back(*(p + 1));
                p += 2;
            } else {
                if (*p != '{') {
                    buf.push_back(*p++);
                } else {
                    p++;
                    break;
                }
            }
        }
        buf.append(ToString(d));
        while (*p != '\0') {
            if (*p == '\\' && (*(p + 1) == '{' || *(p + 1) == '}')) {
                p += 2;
            } else {
                if (*p == '}') break;
                p++;
            }
        }
        if (*p == '}')
            return MyPrintf(buf, ++p, ds...);
        else
            return false;
    }

    static bool MyPrintf(std::string& buf, const char* format) {
        const char* p = format;
        while (*p != '\0') {
            if (*p == '\\' && (*(p + 1) == '{' || *(p + 1) == '}')) {
                buf.push_back(*(p + 1));
                p += 2;
            } else {
                if (*p != '{' && *p != '}') {
                    buf.push_back(*p++);
                } else {
                    return false;
                }
            }
        }
        return true;
    }

    static constexpr size_t GetNumFields(const char* p, bool skip) {
        return (p == nullptr || *p == '\0') ? 0
                                            : (skip ? GetNumFields(p + 1, false)
                                                    : (*p == '{'    ? 1 + GetNumFields(p + 1, false)
                                                       : *p == '\\' ? GetNumFields(p + 1, true)
                                                                    : GetNumFields(p + 1, false)));
    }

    static constexpr bool CheckNumFields(const char* p, size_t n_fields) {
        return GetNumFields(p, false) == n_fields;
    }

 public:
    template <typename... Ts>
    static const std::string& Append(std::string& buf, const char* format, const Ts&... data) {
        if (!MyPrintf(buf, format, data...)) {
            LOG_ERROR() << "Error formatting string with format string \"" << format
                      << "\". Number of fields given: " << sizeof...(Ts);
        }
        return buf;
    }

    template <typename... Ts>
    static const std::string& Format(std::string& buf, const char* format, const Ts&... data) {
        buf.clear();
        if (!MyPrintf(buf, format, data...)) {
            LOG_ERROR() << "Error formatting string with format string \"" << format
                      << "\". Number of fields given: " << sizeof...(Ts);
        }
        return buf;
    }

    template <typename... Ts>
    static std::string Format(const char* format, const Ts&... data) {
        // static_assert(CheckNumFields(format, 0), "Number of fields does not match");
        std::string ret;
        if (!MyPrintf(ret, format, data...)) {
            LOG_ERROR() << "Error formatting string with format string \"" << format
                      << "\". Number of fields given: " << sizeof...(Ts);
        }
        return ret;
    }

#define FMA_FMT fma_common::StringFormatter::Format
};
}  // namespace fma_common
