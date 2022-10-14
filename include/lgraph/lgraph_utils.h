/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <sys/time.h>
#include <sys/mman.h>
#include <math.h>
#include <string>
#include <vector>

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
void split_string(std::string origin_string,
                std::vector<std::string>& sub_strings, std::string string_delimiter);

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
}  // namespace lgraph_api
