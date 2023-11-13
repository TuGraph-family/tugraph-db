//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\text_parser.h.
 *
 * \brief   Declares the text parser class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <functional>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <tuple>

#include "fma-common/file_stream.h"
#include "fma-common/pipeline.h"
#include "fma-common/type_traits.h"

namespace fma_common {
struct ParseFieldException : public std::exception {
    const char* description;
    const char* line;
    const char* buf_end;
    size_t offset;
    size_t nth_field;
    const char* type_name;
    std::string what_;

    ParseFieldException(const char* description_, const char* line_beg, const char* buf_end_,
                        size_t char_offset, size_t nth, const char* type)
        : description(description_),
          line(line_beg),
          buf_end(buf_end_),
          offset(char_offset),
          nth_field(nth),
          type_name(type) {
        const char* le = line;
        while (le < buf_end && *le != '\n') ++le;
        what_.append("Error parsing line:\n")
            .append(line, le)
            .append("\n: ")
            .append(description)
            .append(" at position ")
            .append(std::to_string(offset))
            .append(" when parsing the ")
            .append(std::to_string(nth_field))
            .append("th field as type ")
            .append(type_name);
    }

    virtual const char* what() const throw() { return what_.c_str(); }
};

struct UncleanParseException : public std::exception {
    const char* beg;
    const char* end;
    const char* description;
    std::string what_;

    UncleanParseException(const char* beg_, const char* end_, const char* description_)
        : beg(beg_), end(end_), description(description_) {
        what_.append("UncleanParseException: ").append(description).append(":\n");
        size_t n = end - beg;
        if (n < 100) {
            what_.append(beg, end);
        } else {
            what_.append(beg, 100)
                .append(" ... [totally ")
                .append(std::to_string(n))
                .append(" bytes].");
        }
    }

    virtual const char* what() const throw() { return what_.c_str(); }
};

namespace _Detail {
/*!
 * \struct  DropField
 *
 * \brief   Type to use when you want to drop a field in text file.
 */
struct DropField {};

struct CsvString {
    std::string value;
};

struct GraphicalString {
    std::string value;
};

template <typename T, size_t i, bool CSV>
struct ParseNext {
    static inline size_t Parse(const char* b, const char* e, T& d, size_t s);
};

template <typename T, bool CSV>
struct ParseNext<T, 0, CSV> {
    static inline size_t Parse(const char* b, const char* e, T& d, size_t s);
};

template <bool CSV, typename... Types>
struct ParseTuple_ {
    typedef std::tuple<Types...> T;
    static inline size_t Parse(const char* b, const char* e, T& d) {
        return _Detail::ParseNext<T, std::tuple_size<T>::value, CSV>::Parse(b, e, d, 0);
    }
};

/*!
 * \fn  template<typename TBound, typename TValue> static inline bool CheckValueBound(T2 d, const
 * char* b)
 *
 * \brief   Check that integer d is within the min and max value of type T.
 *
 * \tparam  TBound  Type of the bound.
 * \tparam  TValue  Type of the value to check.
 *
 * \param   d   An int64_t to process.
 * \param   b   Buffer from which d is parsed.
 *
 * \return  True if d is within limit.
 */
template <typename TBound>
inline bool CheckValueBound(int64_t d) {
    if (d < std::numeric_limits<TBound>::min() || d > std::numeric_limits<TBound>::max()) {
        return false;
    }
    return true;
}

template <>
inline bool CheckValueBound<bool>(int64_t d) {
    return true;
}

template <>
inline bool CheckValueBound<int64_t>(int64_t d) {
    return true;
}
}  // namespace _Detail

/*!
 * \namespace  TextParserUtils
 *
 * \brief   Text parser utilities.
 */
namespace TextParserUtils {
typedef _Detail::CsvString CsvString;
typedef _Detail::GraphicalString GraphicalString;
typedef _Detail::DropField DropField;
typedef DropField _;

/**
 * Query if 'c' is control character. Same behavior with std::iscntrl(c) with default C locale.
 *
 * \param   c   The character.
 *
 * \return  True if control, false if not.
 */
inline bool IsControl(char c) {
#ifdef _WIN32
    static const bool is_control[256] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 0-31
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  // 127 = DEL
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_control[(uint8_t)c];
#else
    return (c >= 0 && c < 32) || c == 127;
#endif
}

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
 * \fn  static inline bool IsDigital(char c)
 *
 * \brief   Query if 'c' is digital characters (0-9, ., e, E, -).
 *
 * \param   c   The character.
 *
 * \return  True if digital, false if not.
 */
inline bool IsDigital(char c) {
    static const bool is_digital[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    return is_digital[(uint8_t)c];
}

/*!
 * \fn  static inline bool IsGraphical(char c)
 *
 * \brief   Query if 'c' is graphical. Graphical characters are those
 *          which can be printed, except for the space character.
 *
 * \param   c   The character.
 *
 * \return  True if c has graphical representation.
 */
inline bool IsGraphical(char c) {
#ifdef _WIN32
    static const bool is_graphical[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_graphical[(uint8_t)c];
#else
    return c > 32 && c < 127;
#endif
}

// Query if c is a trimmable character, i.e. non-printable character that are
// not \r \n or unicode
inline bool IsTrimable(char c) {
    static const bool is_graphical[256] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_graphical[(uint8_t)c];
}

/*!
 * \fn  static inline bool IsNewLine(char c)
 *
 * \brief   Query if 'c' is new line ('\r' and '\n').
 *
 * \param   c   The character.
 *
 * \return  True if new line, false if not.
 */
inline bool IsNewLine(char c) {
#ifdef _WIN32
    static const bool is_newline[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_newline[(uint8_t)c];
#else
    return c == '\r' || c == '\n';
#endif
}

/*!
 * \fn  static inline bool IsAlphabetNumberic(char c)
 *
 * \brief   Query if 'c' is alphabet numberic, that is, if 'c' is 0-9,
 *          a-z or A-Z.
 *
 * \param   c   The character.
 *
 * \return  True if alphabet numberic, false if not.
 */
inline bool IsAlphabetNumberic(char c) {
    static const bool is_alpha_numeric[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_alpha_numeric[(uint8_t)c];
}

/**
 * Query if 'c' is valid name character, that is 0-9, a-Z and _
 *
 * @param c The character.
 *
 * @return  True if it succeeds, false if it fails.
 */
inline bool IsValidNameCharacter(char c) {
    static const bool is_valid_name_char[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,
        1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_valid_name_char[(uint8_t)c];
}

/**
 * Query if 'c' is blank
 *
 * \param   c   A char to process.
 *
 * \return  True if blank, false if not.
 */
inline bool IsBlank(char c) {
#ifdef _WIN32
    static const bool is_blank[256] = {
        0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };
    return is_blank[(uint8_t)c];
#else
    return c == '\t' || c == ' ';
#endif
}

inline char ToLower(char c) {
    static const uint8_t to_lower_[256] = {
        0,   1,   2,   3,   4,   5,   6,   7,   8,   9,   10,  11,  12,  13,  14,  15,  16,  17,
        18,  19,  20,  21,  22,  23,  24,  25,  26,  27,  28,  29,  30,  31,  32,  33,  34,  35,
        36,  37,  38,  39,  40,  41,  42,  43,  44,  45,  46,  47,  48,  49,  50,  51,  52,  53,
        54,  55,  56,  57,  58,  59,  60,  61,  62,  63,  64,  97,  98,  99,  100, 101, 102, 103,
        104, 105, 106, 107, 108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121,
        122, 91,  92,  93,  94,  95,  96,  97,  98,  99,  100, 101, 102, 103, 104, 105, 106, 107,
        108, 109, 110, 111, 112, 113, 114, 115, 116, 117, 118, 119, 120, 121, 122, 123, 124, 125,
        126, 127, 128, 129, 130, 131, 132, 133, 134, 135, 136, 137, 138, 139, 140, 141, 142, 143,
        144, 145, 146, 147, 148, 149, 150, 151, 152, 153, 154, 155, 156, 157, 158, 159, 160, 161,
        162, 163, 164, 165, 166, 167, 168, 169, 170, 171, 172, 173, 174, 175, 176, 177, 178, 179,
        180, 181, 182, 183, 184, 185, 186, 187, 188, 189, 190, 191, 192, 193, 194, 195, 196, 197,
        198, 199, 200, 201, 202, 203, 204, 205, 206, 207, 208, 209, 210, 211, 212, 213, 214, 215,
        216, 217, 218, 219, 220, 221, 222, 223, 224, 225, 226, 227, 228, 229, 230, 231, 232, 233,
        234, 235, 236, 237, 238, 239, 240, 241, 242, 243, 244, 245, 246, 247, 248, 249, 250, 251,
        252, 253, 254, 255};
    return (char)to_lower_[(uint8_t)c];
}

// skip a whole line including '\r\n' or '\n\r' or '\n' or '\r'
inline const char* FindNextLine(const char* p, const char* e) {
    while (p != e) {
        if (*p == '\r') {
            p++;
            if (p == e) return p;
            return *p == '\n' ? p + 1 : p;
        } else if (*p == '\n') {
            p++;
            if (p == e) return p;
            return *p == '\r' ? p + 1 : p;
        }
        p++;
    }
    return e;
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

inline size_t ParseBool(const char* b, const char* e, bool& d) {
    if (b == e) return 0;
    int64_t i;
    size_t s = ParseInt64(b, e, i);
    if (s != 0) {
        d = (i != 0);
        return s;
    }
    if (*b == 'f' || *b == 'F') {
        static const char* str = "false";
        const char* p = str;
        while (b < e && p < str + 5) {
            if (ToLower(*b) == *p) {
                b++;
                p++;
            } else {
                return 0;
            }
        }
        if (p == str + 5) {
            d = false;
            return 5;
        } else {
            return 0;
        }
    } else if (*b == 't' || *b == 'T') {
        static const char* str = "true";
        const char* p = str;
        while (b < e && p < str + 4) {
            if (ToLower(*b) == *p) {
                b++;
                p++;
            } else {
                return 0;
            }
        }
        if (p == str + 4) {
            d = true;
            return 4;
        } else {
            return 0;
        }
    }
    return 0;
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

/*!
 * \fn  template<typename IntegerT> static inline size_t ParseDigit(const char* b, const char* e,
 * IntegerT& d)
 *
 * \brief   Parse digit from string.
 *
 * \tparam  IntegerT  Type of the teger t.
 *
 * \param       b     Begining of the string.
 * \param       e     One past the end of the string.
 * \param [out] d     Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
template <typename IntT>
inline typename std::enable_if<std::is_integral<IntT>::value, size_t>::type ParseDigit(
    const char* b, const char* e, IntT& d) {
    static_assert(!std::is_same<IntT, uint64_t>::value,
                  "ParseDigit<uint64_t> is not implemented, use ParseDigit<int64_t> instead");
    int64_t i;
    size_t r = ParseInt64(b, e, i);
    if (r == 0) return 0;
    if (!_Detail::CheckValueBound<IntT>(i)) return 0;
    d = (IntT)i;
    return r;
}

/*!
 * \fn  static inline size_t ParseDigit(const char* b, const char* e, float& d)
 *
 * \brief   Parse digit from string.
 *
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
inline size_t ParseDigit(const char* b, const char* e, float& d) {
    double i = 0;
    size_t r = ParseDouble(b, e, i);
    d = (float)i;
    return r;
}

/*!
 * \fn  static inline size_t ParseDigit(const char* b, const char* e, double& d)
 *
 * \brief   Parse digit from string.
 *
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
inline size_t ParseDigit(const char* b, const char* e, double& d) { return ParseDouble(b, e, d); }

inline size_t ParseDigit(const char* b, const char* e, bool& d) { return ParseBool(b, e, d); }

/*!
 * \fn  static inline size_t ParseAlphabetNumeric(const char*b, const char* e, std::string& s)
 *
 * \brief   Parse alphabet numeric string from string buffer. Alphabet
 *          numeric string contains characters in 0-9, a-z, A-Z.
 *
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
inline size_t ParseAlphabetNumeric(const char* b, const char* e, std::string& s) {
    s.clear();
    const char* orig = b;
    if (b == e) return 0;
    while (b != e && IsAlphabetNumberic(*b)) s.push_back(*b++);
    return b - orig;
}

/*!
 * \fn  static inline size_t ParseGraphical(const char*b, const char* e, std::string& s)
 *
 * \brief   Parse graphical string from string buffer. Graphical characters
 *          include all characters that has graphical representation,
 *          that is, any c > 32, including symbols, 0-9, a-z.
 *
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
inline size_t ParseGraphicalString(const char* b, const char* e, std::string& s) {
    s.clear();
    const char* orig = b;
    if (b == e) return 0;
    while (b != e && IsGraphical(*b)) s.push_back(*b++);
    return b - orig;
}

/*!
 * \fn  static inline size_t DropGraphicalField(const char*b, const char* e)
 *
 * \brief   Drop a graphical field. Graphical characters are characters
 *          that has graphical representations, that is, all c > 32,
 *          including symbols, 0-9, a-z;
 *
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 *
 * \return  Number of bytes dropped.
 */
static inline size_t DropGraphicalField(const char* b, const char* e) {
    const char* orig = b;
    if (b == e) return 0;
    while (b != e && IsGraphical(*b)) b++;
    return b - orig;
}

/*!
 * \fn  static inline size_t ParsePrintableString(const char*b, const char* e, std::string& s)
 *
 * \brief   Parse printable string. Printable string contains characters
 *          that are printable, that is, any character that has a graphical
 *          representation (\032 - \126, including Space, symbols,
 *          alphabets, and numeric characters).
 *
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
static inline size_t ParsePrintableString(const char* b, const char* e, std::string& s) {
    s.clear();
    const char* orig = b;
    while (b != e && *b < 32) b++;
    if (b == e) return 0;
    while (b != e && *b > 31 && *b < 127) s.push_back(*b++);
    return b - orig;
}

// we ignore characters that are non-printable and not '\t', ' ', '\r', '\n'
inline bool IsCsvIgnoreable(char c) {
    static const bool is_csv_ignorable[256] = {
        1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1,  // 0-31
        1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,  // 127 = DEL
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
        0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    };

    return is_csv_ignorable[(uint8_t)c];
}

/*!
 * \fn  static inline size_t ParseCsvString(const char*b, const char* e, std::string& s)
 *
 * \brief   Parse Csv string till we hit a comma. A Csv string is a string
 *          containing printable characters, that is, any c > 31, including
 *          all symbols, a-z, A-z, 0-9, ' ', and '\t'
 *
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
static inline size_t ParseCsvString(const char* b, const char* e, std::string& s) {
    s.clear();
    const char* orig = b;
    while (b != e && IsBlank(*b)) b++;
    if (b == e) return 0;
    if (*b == '"') {
        // quoted
        b++;
        while (b != e && !TextParserUtils::IsNewLine(*b)) {
            /* ignore unprintable characters */
            if (IsCsvIgnoreable(*b)) {
                b++;
                continue;
            }
            if (*b == '"') {
                b++;
                if (b == e || *b != '"') {
                    break;
                } else {
                    // *b == '"'
                    s.push_back(*b);
                    b++;
                }
            } else {
                s.push_back(*b);
                b++;
            }
        }
    } else {
        while (b != e && !TextParserUtils::IsNewLine(*b) && *b != ',') {
            /* ignore unprintable characters */
            if (!IsCsvIgnoreable(*b)) s.push_back(*b);
            b++;
        }
    }
    return b - orig;
}

static inline size_t ParseQuotedString(const char* b, const char* e, std::string& s) {
    const char* orig = b;
    assert(*b == '"');
    // quoted
    b++;
    while (b != e && !TextParserUtils::IsNewLine(*b)) {
        if (*b == '"') {
            b++;
            if (b == e || *b != '"') {
                break;
            } else {
                // *b == '"'
                s.push_back(*b);
                b++;
            }
        } else {
            s.push_back(*b);
            b++;
        }
    }
    return b - orig;
}

/*!
 * \fn  static inline size_t DropCsvField(const char*b, const char* e)
 *
 * \brief   Drop Csv string till we hit a comma. A Csv string is a string
 *          containing printable characters, that is, any c > 31, including
 *          all symbols, a-z, A-z, 0-9, ' ', and '\t'
 *
 * \param       b   Begining of the string.
 * \param       e   End of the string.
 *
 * \return  Number of bytes parsed.
 */
static inline size_t DropCsvField(const char* b, const char* e) {
    const char* orig = b;
    while (b != e && IsBlank(*b)) b++;
    if (b == e) return 0;
    if (*b == '"') {
        // quoted
        b++;
        while (b != e && !TextParserUtils::IsNewLine(*b)) {
            /* ignore unprintable characters */
            if (IsCsvIgnoreable(*b)) {
                b++;
                continue;
            }
            if (*b == '"') {
                b++;
                if (b == e || *b != '"') {
                    break;
                } else {
                    b++;
                }
            } else {
                b++;
            }
        }
    } else {
        while (b != e && !TextParserUtils::IsNewLine(*b) && *b != ',') {
            b++;
        }
    }
    return b - orig;
}

/*!
 * \fn  template<typename T> static inline size_t ParseT(const char* b, const char* e, T& d)
 *
 * \brief   Parse string into type T.
 *
 * \tparam  T   Generic type parameter.
 * \param        b   Begining of the string.
 * \param        e   End of the string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  Number of bytes parsed.
 */
template <typename T, bool IS_CSV = false>
inline size_t ParseT(const char* b, const char* e, T& d) {
    // return _Detail::ParseStr_<T, IS_CSV>::Parse(b, e, d);
    return ParseDigit(b, e, d);
}

template <typename T, bool IS_CSV = false>
inline size_t ParseT(const std::string& str, T& d) {
    return ParseT(str.data(), str.data() + str.size(), d);
}

template <>
inline size_t ParseT<DropField, true>(const char* b, const char* e, DropField&) {
    return DropCsvField(b, e);
}

template <>
inline size_t ParseT<DropField, false>(const char* b, const char* e, DropField&) {
    return DropGraphicalField(b, e);
}

template <>
inline size_t ParseT<GraphicalString, true>(const char* b, const char* e, GraphicalString& s) {
    return ParseGraphicalString(b, e, s.value);
}

template <>
inline size_t ParseT<GraphicalString, false>(const char* b, const char* e, GraphicalString& s) {
    return ParseGraphicalString(b, e, s.value);
}

template <>
inline size_t ParseT<std::string, false>(const char* b, const char* e, std::string& s) {
    return ParseGraphicalString(b, e, s);
}

template <>
inline size_t ParseT<std::string, true>(const char* b, const char* e, std::string& s) {
    return ParseCsvString(b, e, s);
}

template <>
inline size_t ParseT<char, true>(const char* b, const char* e, char& c) {
    const char* orig = b;
    if (b == e) return 0;
    c = *b++;
    return b - orig;
}

template <>
inline size_t ParseT<char, false>(const char* b, const char* e, char& c) {
    const char* orig = b;
    if (b == e) return 0;
    c = *b++;
    return b - orig;
}

template <>
inline size_t ParseT<CsvString, true>(const char* b, const char* e, CsvString& s) {
    return ParseCsvString(b, e, s.value);
}

template <>
inline size_t ParseT<CsvString, false>(const char* b, const char* e, CsvString& s) {
    return ParseCsvString(b, e, s.value);
}

/*!
 * \struct  TupleParser
 *
 * \brief   A tuple parser that parses a line into a tuple and returns
 *          the number of bytes parsed.
 *
 * \tparam  IS_CSV  Is the text seperated by comma (',') ?
 * \tparam  Ts      Type of the fields in each line.
 */
template <bool IS_CSV = false, typename... Ts>
struct TupleParser {
    typedef std::tuple<Ts...> T;
    inline std::tuple<size_t, bool> operator()(const char* b, const char* e, T& d) const {
        size_t s = _Detail::ParseTuple_<IS_CSV, Ts...>::Parse(b, e, d);
        const char* p = FindNextLine(b + s, e);
        return std::tuple<size_t, bool>(p - b, s != 0);
    }
};

template <bool IS_CSV = false, typename... Ts>
static inline std::tuple<size_t, bool> ParseAsTuple(const char* b, const char* e, Ts&&... data) {
    std::tuple<typename std::decay<Ts>::type...> tup;
    auto t = TupleParser<IS_CSV, typename std::decay<Ts>::type...>()(b, e, tup);
    std::tie(std::forward<Ts>(data)...) = tup;
    return t;
}
};  // namespace TextParserUtils

namespace _Detail {
template <typename T, size_t i, bool CSV>
size_t ParseNext<T, i, CSV>::Parse(const char* b, const char* e, T& d, size_t s) {
    constexpr size_t idx = std::tuple_size<T>::value - i;
    typedef typename std::tuple_element<idx, T>::type ElementT;
    ElementT& element = std::get<idx>(d);
    while (b + s != e && !TextParserUtils::IsGraphical(b[s])) s++;
    size_t r = TextParserUtils::ParseT<ElementT, CSV>(b + s, e, element);
    if (r == 0) {
        throw ParseFieldException("failed to parse data", b, e, s, std::tuple_size<T>::value - i,
                                  typeid(ElementT).name());
    }
    const char* p = b + s + r;
    if (i > 1) {
        if (CSV) {
            // in the middle of line, check comma
            while (p < e && *p != ',' && !TextParserUtils::IsNewLine(*p)) {
                p++, r++;
            }
            if (p == e || TextParserUtils::IsNewLine(*p)) {
                throw ParseFieldException("unexpected line break after data", b, e, s,
                                          std::tuple_size<T>::value - i, typeid(ElementT).name());
            }
            if (*p != ',') {
                throw ParseFieldException("no comma after data", b, e, s,
                                          std::tuple_size<T>::value - i, typeid(ElementT).name());
            }
        } else {
            while (p < e && !TextParserUtils::IsBlank(*p) && !TextParserUtils::IsNewLine(*p)) {
                p++, r++;
            }
            if (p == e || TextParserUtils::IsNewLine(*p)) {
                throw ParseFieldException("unexpected line break after data", b, e, s,
                                          std::tuple_size<T>::value - i, typeid(ElementT).name());
            }
            if (!TextParserUtils::IsBlank(*p)) {
                throw ParseFieldException("no blank space after data", b, e, s,
                                          std::tuple_size<T>::value - i, typeid(ElementT).name());
            }
        }
        r++;
    }
    return ParseNext<T, i - 1, CSV>::Parse(b, e, d, s + r);
}

template <typename T, bool CSV>
size_t ParseNext<T, 0, CSV>::Parse(const char*, const char*, T&, size_t s) {
    return s;
}
}  // namespace _Detail

class TextParserBase {
 public:
    static const size_t DEFAULT_BLOCK_SIZE = 1024 * 1024;
};

/*!
 * \class   TextParser
 *
 * \brief   A text parser for reading text from file and parsing into binary.
 *
 * \tparam  T   Data type, can be std::tuple<T...>.
 * \tparam  F   Type of the functor to parse one line.
 */
template <typename T, typename F>
class TextParser : public TextParserBase {
 public:
    DISABLE_COPY(TextParser);
    DISABLE_MOVE(TextParser);

    typedef T ElementType;

    /*
     * \brief Parse function type:
     *           size_t func(const char* beg, const char* end, T& d)
     *        This function is required to skip any meaningless headings like
     *        blankspace, tab or new line \r \n
     *
     * \param beg   pointer to the buffer to be parsed
     * \param end   end of the buffer
     * \param data  parse result of one line
     *
     * \return Number of bytes read by this operation, including the \r\n
     */
    typedef typename std::decay<F>::type ParseOneLine;

    /*!
     * \fn  TextParser::TextParser(InputFileStream& stream, const ParseOneLine& func, size_t
     * block_size = DEFAULT_BLOCK_SIZE, size_t n_threads = 1)
     *
     * \brief   Constructor.
     *
     * \param [in,out]  stream      The file stream to read from.
     * \param           func        The function to parse one line.
     * \param           block_size  (Optional) Size of each parse block (in bytes).
     * \param           n_threads  (Optional) The number of blocks to prefetch.
     */
    TextParser(InputFileStream& stream, const ParseOneLine& func,
               size_t block_size = DEFAULT_BLOCK_SIZE, size_t n_prefetch = 1,
               size_t n_header_lines = 0)
        : stream_(&stream), func_(func) {
        owns_stream_ = false;
        Start(func, block_size, n_prefetch, n_header_lines);
    }

    TextParser(std::unique_ptr<InputFileStream>&& stream, const ParseOneLine& func,
               size_t block_size = DEFAULT_BLOCK_SIZE, size_t n_prefetch = 1,
               size_t header_lines = 0)
        : stream_(std::move(stream)), func_(func) {
        owns_stream_ = true;
        Start(func, block_size, n_prefetch);
    }

    ~TextParser() { Stop(); }

    /*!
     * \fn  bool TextParser::Read(T& data)
     *
     * \brief   Reads a parsed data object.
     *
     * \param [out]  data    The data to read.
     *
     * \return  True if it succeeds, false if it fails.
     * \throw   std::runtime_error if there is parse error
     */
    bool Read(ElementType& data) {
        if (curr_buf_ == -1) {
            bool s = parsed_data_->Pop(curr_buf_);
            if (!s || curr_buf_ == -1) return false;
            Workspace& w = workspace_[curr_buf_];
            if (!w.error.empty()) throw std::runtime_error(w.error.c_str());
        }
        Workspace& w = workspace_[curr_buf_];
        if (w.data.size() <= w.head) return false;
        data = w.data[w.head++];
        if (w.head == w.data.size()) {
            read_stage_->Push(std::move(curr_buf_));
            curr_buf_ = -1;
        }
        return true;
    }

    /*!
     * \fn  bool TextParser::ReadBlock(std::vector<T>& block)
     *
     * \brief   Reads a block of data.
     *
     * \param [out]  block   The data block.
     *
     * \return  True if it succeeds, false if it fails.
     * \throw   std::runtime_error if there is parse error
     */
    bool ReadBlock(std::vector<ElementType>& block) {
        if (curr_buf_ == -1) {
            bool s = parsed_data_->Pop(curr_buf_);
            if (!s || curr_buf_ == -1) {
                parsed_data_->Push(-1);
                return false;
            }
            Workspace& w = workspace_[curr_buf_];
            if (!w.error.empty()) throw std::runtime_error(w.error.c_str());
        }
        Workspace& w = workspace_[curr_buf_];
        if (w.data.size() <= w.head) return false;
        if (w.head == 0) {
            // optimization: swap the memory so we can reuse it
            w.data.swap(block);
        } else {
            block.assign(w.data.begin() + w.head, w.data.end());
        }
        read_stage_->Push(curr_buf_);
        curr_buf_ = -1;
        return true;
    }

    void WaitEmpty() {
        if (read_stage_) read_stage_->WaitTillClear();
        if (parse_stage_) parse_stage_->WaitTillClear();
    }

    void Stop() {
        if (n_prefetch_ > 0 && parsed_data_) {
            read_stage_->Stop();
            parse_stage_->Stop();
            parsed_data_->EndQueue();
            read_stage_.reset();
            parse_stage_.reset();
            parsed_data_.reset();
        }
        workspace_.clear();
        curr_buf_ = -1;
        if (owns_stream_) {
            stream_.reset();
            owns_stream_ = false;
        } else {
            stream_.release();
        }
    }

 private:
    // returns 0 if this this the last line
    // otherwise, return the first character in the next line
    char SkipOneLine() {
        char c;
        while (stream_->Good()) {
            if (!stream_->Read(&c, 1)) return 0;
            if (TextParserUtils::IsNewLine(c)) {
                if (c == '\r') {
                    if (!stream_->Read(&c, 1)) return 0;
                    if (c == '\n') return 0;
                    return c;
                }
                if (c == '\n') {
                    if (!stream_->Read(&c, 1)) return 0;
                    if (c == '\r') return 0;
                    return c;
                }
            }
        }
        return 0;
    }

    void Start(const ParseOneLine& func, size_t block_size, size_t n_prefetch,
               size_t header_lines) {
        func_ = func;
        block_size_ = block_size;
        n_prefetch_ = n_prefetch == 0 ? 1 : n_prefetch;
        n_prefetch_ = std::min<size_t>(n_prefetch_, std::numeric_limits<int>::max());
        n_bytes_read_ = 0;
        curr_buf_ = -1;
        // skip header lines
        char first_char = 0;
        for (size_t i = 0; i < header_lines; i++) {
            first_char = SkipOneLine();
        }
        if (first_char != 0) {
            left_over_.push_back(first_char);
        }

        workspace_.resize(n_prefetch_);
        read_stage_ = std::make_unique<PipelineStage<int, int>>(
            [this](int i) -> int { return ReadStage(i); }, nullptr, 0, 1);
        parse_stage_ = std::make_unique<PipelineStage<int, int>>(
            [this](int i) -> int { return ParseStage(i); }, nullptr, 0, n_prefetch_, n_prefetch_);
        parsed_data_ = std::make_unique<BoundedQueue<int>>();
        read_stage_->SetNextStage(parse_stage_.get());
        parse_stage_->SetNextStage(parsed_data_.get());
        for (size_t i = 0; i < n_prefetch_; i++) {
            read_stage_->Push((int)i);
        }
    }

    int ReadStage(int i) {
        Workspace& w = workspace_[i];
        w.offset = stream_->Offset() - left_over_.size();
        std::string& buf = w.raw_buf;
        // move left over bytes
        buf.clear();
        buf.swap(left_over_);
        // read data
        while (true) {
            size_t last_size = buf.size();
            buf.resize(last_size + block_size_);
            size_t read_size = stream_->Read(&buf[last_size], block_size_);
            buf.resize(last_size + read_size);
            if (read_size == 0) return last_size == 0 ? -1 : i;  // nothing left to read
            size_t pos = buf.rfind('\n');
            if (pos == buf.npos) {
                // not a complete line, read more
                continue;
            } else if (!stream_->Good()) {
                // EOF, do not leave over the last line without '\n'
                // do nothing on buf, and let left_over_ be empty as-is
                return i;
            } else {
                pos++;
                left_over_.assign(buf.begin() + pos, buf.end());
                buf.resize(pos);
                return i;
            }
        }
    }

    int ParseStage(int i) {
        if (i == -1) return i;
        Workspace& w = workspace_[i];
        w.head = 0;
        std::string& buf = w.raw_buf;
        std::vector<T>& data = w.data;
        data.clear();
        // size_t n_data = 0;
        const char* bbeg = &buf[0];
        const char* bend = &buf[buf.size()];
        try {
            while (bbeg < bend) {
                size_t r;
                bool success = false;
                T d;
                std::tie(r, success) = func_(bbeg, bend, d);
                if (r == 0) break;
                if (success) data.push_back(std::move(d));
                bbeg += r;
                // remove trailing blank space
                while (bbeg < bend && TextParserUtils::IsNewLine(*bbeg)) bbeg++;
            }
            if (bbeg != bend) throw(UncleanParseException(bbeg, bend, ""));
        } catch (std::exception& e) {
            size_t offset = (bbeg - &buf[0]) + w.offset;
            w.error.append("Error parsing file ")
                .append(stream_->Path())
                .append("\n\tError occurred at offset ")
                .append(std::to_string(offset))
                .append(", exception detail:\n\t")
                .append(e.what());
            return i;
        }
        return i;
    }

 private:
    struct Workspace {
        size_t offset;        // offset of this buffer in the whole file
        std::string raw_buf;  // raw bytes
        std::vector<T> data;  // parsed data
        size_t head;          // the read position of the parsed data
        std::string error;    // error message if there is parsing error
    };

    bool owns_stream_ = false;
    std::unique_ptr<InputFileStream> stream_;
    ParseOneLine func_;
    size_t block_size_;
    size_t n_prefetch_;
    size_t n_bytes_read_;

    std::vector<Workspace> workspace_;
    std::string left_over_;
    std::unique_ptr<PipelineStage<int, int>> read_stage_;
    std::unique_ptr<PipelineStage<int, int>> parse_stage_;
    std::unique_ptr<BoundedQueue<int>> parsed_data_;
    int curr_buf_ = -1;
};

/*!
 * \fn  template<typename T, typename F> TextParser<T, F> MakeTextParser(InputFileStream& stream,
 * F&& func, size_t block_size = TextParser<T,F>::DEFAULT_BLOCK_SIZE, size_t n_threads = 1)
 *
 * \brief   Makes a text parser.
 *
 * \tparam  T   Data type to be read.
 * \tparam  F   Type of the functor to parse one line.
 *
 * \param [in,out]  stream      The input file stream.
 * \param [in,out]  func        Rvalue reference to the function.
 * \param           block_size  (Optional) Size of the parse block.
 * \param           n_threads  (Optional) Number of blocks to prefetch.
 *
 * \return  A TextParser&lt;T,F&gt;
 *          You can use ReadBlock(std::vector<T>&) to read blocks of data.
 */
template <typename T, typename F>
std::shared_ptr<TextParser<T, F>> MakeTextParser(
    InputFileStream& stream, const F& func, size_t block_size = TextParserBase::DEFAULT_BLOCK_SIZE,
    size_t n_threads = 1, size_t n_header_lines = 0) {
    return std::make_shared<TextParser<T, F>>(stream, func, block_size, n_threads, n_header_lines);
}

/*!
 * \fn  template<typename... Ts> TextParser<std::tuple<Ts...>, TextParserUtils::TupleParser<true,
 * Ts...>> MakeCsvParser(InputFileStream& stream, size_t block_size =
 * TextParserBase::DEFAULT_BLOCK_SIZE, size_t n_threads = 1)
 *
 * \brief   Makes a CSV parser. A CSV file is a file in which the fields
 *          in each line are seperated by a comma (',').
 *
 * \tparam  Ts  Types of the different fields in CSV file
 *
 * \param [in,out]  stream      The file stream from which to read.
 * \param           block_size  (Optional) Size of each text block to parse.
 *                              1MB text blocks seem to be optimal.
 * \param           n_threads  (Optional) The number of parallel threads to
 *                              use to parse the text. Threads will parse
 *                              different blocks in parallel, so make sure
 *                              your memory won't run out.
 *
 * \return  A TextParser&lt;std::tuple&lt;Ts...&gt;,TextParserUtils::TupleParser&lt;true,Ts...&gt;
 *          &gt;
 *          You can then use ReadBlock(std::vector<std::tuple<T1, T2...>>&)
 *          to read blocks of data.
 */
template <typename... Ts>
std::shared_ptr<TextParser<std::tuple<Ts...>, TextParserUtils::TupleParser<true, Ts...>>>
MakeCsvParser(InputFileStream& stream, size_t block_size = TextParserBase::DEFAULT_BLOCK_SIZE,
              size_t n_threads = 1, size_t n_header_lines = 0) {
    typedef std::tuple<Ts...> TUP;
    typedef TextParserUtils::TupleParser<true, Ts...> FUNC;
    return std::make_shared<TextParser<TUP, FUNC>>(stream, FUNC(), block_size, n_threads,
                                                   n_header_lines);
}

/**
 * Makes tsv parser.A TSV file is a text file in which the fields in each line are seperated
 * by a space (' ') or a tab ('\t')
 *
 * \tparam  Ts  Types of fields. Use TextParserUtils::DropField if you want to skip a field.
 * \param [in,out]  stream      The input file stream.
 * \param           block_size  (Optional) Size of each text block to parse.
 *                              1MB text blocks seem to be optimal.
 * \param           n_threads   (Optional) The number of parallel threads to
 *                              use to parse the text. Threads will parse
 *                              different blocks in parallel, so make sure
 *                              your memory won't run out.
 *
 * \return  A std::shared_ptr&lt;TextParser&lt;std::tuple&lt;Ts...&gt;,
 *          TextParserUtils::TupleParser&lt;false,Ts...&gt;&gt;&gt;
 */
template <typename... Ts>
std::shared_ptr<TextParser<std::tuple<Ts...>, TextParserUtils::TupleParser<false, Ts...>>>
MakeTsvParser(InputFileStream& stream, size_t block_size = TextParserBase::DEFAULT_BLOCK_SIZE,
              size_t n_threads = 1, size_t n_header_lines = 0) {
    typedef std::tuple<Ts...> TUP;
    typedef TextParserUtils::TupleParser<false, Ts...> FUNC;
    return std::make_shared<TextParser<TUP, FUNC>>(stream, FUNC(), block_size, n_threads,
                                                   n_header_lines);
}
}  // namespace fma_common
