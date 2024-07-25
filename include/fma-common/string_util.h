//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\string_util.h.
 *
 * \brief   Declares the string utility class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <algorithm>
#include <cmath>
#include <iterator>
#include <list>
#include <locale>
#include <map>
#include <queue>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <boost/algorithm/hex.hpp>

#include "fma-common/type_traits.h"

namespace fma_common {
/*!
 * \fn  inline std::string Strip(const std::string & str, char c)
 *
 * \brief   Strips off all occurrences of character c from the begining and
 *          the end of string.
 *
 * \param   str The string.
 * \param   c   The character to be stripped off.
 *
 * \return  The result string.
 */
inline std::string Strip(const std::string& str, char c) {
    size_t b, e;
    for (b = 0; b < str.size(); b++) {
        if (str[b] != c) break;
    }
    for (e = str.size(); e > b; e--) {
        if (str[e - 1] != c) break;
    }
    return str.substr(b, e - b);
}

/*!
 * \fn  inline std::string Strip(const std::string & str, const std::string& chars)
 *
 * \brief   Strips off all occurrences of characters in chars from the
 *          begining and the end of string.
 *
 * \param   str     The string.
 * \param   chars   The characters to be stripped off.
 *
 * \return  The result string.
 */
inline std::string Strip(const std::string& str, const std::string& chars) {
    size_t i = 0;
    while (i < str.size() && chars.find(str[i]) != chars.npos) i++;
    size_t j = str.size();
    while (j > i && chars.find(str[j - 1]) != chars.npos) j--;
    return str.substr(i, j - i);
}

/*!
 * \fn  inline std::string ToLower(const std::string & str)
 *
 * \brief   Converts a str to lower case.
 *
 * \param   str The string.
 *
 * \return  Lower case string.
 */
inline std::string ToLower(const std::string& str) {
    std::string ret(str);
    for (auto& c : ret) {
        c = tolower(c);
    }
    return ret;
}

/**
 * Converts a str to upper case.
 *
 * \param str   The string.
 *
 * \return  Upper case string.
 */
inline std::string ToUpper(const std::string& str) {
    std::string ret(str);
    for (auto& c : ret) {
        c = toupper(c);
    }
    return ret;
}

/**
 * String equal compare with case sensitivity
 *
 * \param left              The left.
 * \param right             The right.
 * \param case_sensitive    True to case sensitive.
 *
 * \return  True if the strings compares equal, false otherwise.
 */
inline bool StringEqual(const std::string& left, const std::string& right, bool case_sensitive) {
    if (left.size() != right.size()) return false;
    if (case_sensitive) {
        for (size_t i = 0; i < left.size(); i++) {
            if (left[i] != right[i]) return false;
        }
    } else {
        for (size_t i = 0; i < left.size(); i++) {
            if (tolower(left[i]) != tolower(right[i])) return false;
        }
    }
    return true;
}

/*!
 * \fn  inline std::vector<std::string> Split(const std::string & str)
 *
 * \brief   Splits the given string into vector of substrings. All
 *          blankspaces are removed.
 *
 * \param   str The string.
 *
 * \return  A std::vector&lt;std::string&gt;
 */
inline std::vector<std::string> Split(const std::string& str) {
    using namespace std;
    istringstream iss(str);
    vector<std::string> substrings;
    copy(std::istream_iterator<std::string>(iss), std::istream_iterator<std::string>(),
         std::back_inserter<std::vector<std::string>>(substrings));
    return substrings;
}

/*!
 * \fn  inline bool IsIn(char c, const std::string& str)
 *
 * \brief   Query if 'c' is in str.
 *
 * \param   c   The character.
 * \param   str The string.
 *
 * \return  True if in, false if not.
 */
inline bool IsIn(char c, const std::string& str) { return str.find(c) != str.npos; }

/*!
 * \fn  inline std::vector<std::string> Split(const std::string & str, const std::string & breakers)
 *
 * \brief   Splits the given string str into vector of substrings, using
 *          characters in breakers as delimiters. All delimiters are removed.
 *
 * \param   str         The string.
 * \param   breakers    Delimiter characters.
 *
 * \return  A std::vector&lt;std::string&gt;
 */
inline std::vector<std::string> Split(const std::string& str, const std::string& breakers) {
    using namespace std;
    vector<std::string> substrings;
    string::size_type start_pos = 0;
    while (start_pos < str.size()) {
        while (start_pos != str.npos && start_pos < str.size() && IsIn(str[start_pos], breakers))
            start_pos++;
        string::size_type end = str.find_first_of(breakers, start_pos);
        if (end == str.npos) end = str.size();
        if (end > start_pos) {
            substrings.push_back(str.substr(start_pos, end - start_pos));
        }
        start_pos = end;
    }
    return substrings;
}

namespace _detail {
template <class T>
class String2Type {
 public:
    static bool Get(const std::string& str, T& n) {
        std::istringstream iss(str);
        iss >> n;
        return !iss.fail();
    }
};

template <>
class String2Type<uint64_t> {
 public:
    static bool Get(const std::string& str, uint64_t& n) {
#ifdef _WIN32
        return sscanf_s(str.c_str(), "%llu", &n) == 1;
#else
        return sscanf(str.c_str(), "%lu", &n) == 1;
#endif
    }
};

template <>
class String2Type<int> {
 public:
    static bool Get(const std::string& str, int& n) {
#ifdef _WIN32
        return sscanf_s(str.c_str(), "%d", &n) == 1;
#else
        return sscanf(str.c_str(), "%d", &n) == 1;
#endif
    }
};

template <typename T>
class String2Type<std::optional<T>> {
 public:
    static bool Get(const std::string& str, std::optional<T>& opt) {
        T t;
        if (!String2Type<uint64_t>::Get(str, t)) return false;
        opt = t;
        return true;
    }
};
}  // namespace _detail

/*!
 * \fn  template<class T> inline bool ParseString(const std::string & str, T & n)
 *
 * \brief   Parse string into type T.
 *
 * \tparam  T   Result type.
 * \param        str The string.
 * \param [out]  n   Space where the result is stored.
 *
 * \return  True if it succeeds, false if it fails.
 */
template <class T>
inline bool ParseString(const std::string& str, T& n) {
    return _detail::String2Type<T>::Get(str, n);
}

/*!
 * \fn  template<> inline bool ParseString<bool>(const std::string& str, bool& d)
 *
 * \brief   Parse string into a bool.
 *          All variants of True/TRUE,... evaluates to true.
 *          All variants of False/FALSE,... evaluates to false.
 *          1 evaluates to true.
 *          0 evaluates to false.
 *          Other value results in parsing error.
 *
 * \param        str The string.
 * \param [out]  d   Variable to hold the result.
 *
 * \return  True if it succeeds, false if it fails.
 */
template <>
inline bool ParseString<bool>(const std::string& str, bool& d) {
    std::string s = ToLower(str);
    if (s == "true" || s == "t" || s == "1") {
        d = true;
        return true;
    } else if (s == "false" || s == "f" || s == "0") {
        d = false;
        return true;
    }
    return false;
}

/*!
 * \fn  template<> inline bool ParseString<std::string>(const std::string& str, std::string& d)
 *
 * \brief   Parse a string into a string. Specialization for string, just
 *          copy the original string.
 *
 * \param           str The string.
 * \param [in,out]  d   The result string.
 *
 * \return  True if it succeeds, false if it fails.
 */
template <>
inline bool ParseString<std::string>(const std::string& str, std::string& d) {
    d = str;
    return true;
}

/**
 * A template to determine whether class T has a std::string T::ToString() const member.
 *
 * \tparam T    Generic type parameter.
 */
template <typename T>
class _has_to_string {
    template <typename U, typename _ = decltype(std::declval<U>().ToString())>
    static typename std::enable_if<std::is_same<_, std::string>::value, std::true_type>::type test(
        int);

    template <typename U>
    static std::false_type test(...);

 public:
    static bool const value = decltype(test<T>(0))::value;
};

template <class T>
inline std::string ToString(const T& d);

namespace _detail {
template <typename T>
struct Type2String {
    template <typename T2 = T>
    static inline typename std::enable_if<_has_to_string<T2>::value, std::string>::type Get(
        const T2& d) {
        return d.ToString();
    }

    template <typename T2 = T>
    static inline
        typename std::enable_if<!_has_to_string<T2>::value && _has_std_to_string_<T2>::value,
                                std::string>::type
        Get(const T2& d) {
        return std::to_string(d);
    }

    template <typename T2 = T>
    static inline
        typename std::enable_if<!_has_to_string<T2>::value && !_has_std_to_string_<T2>::value,
                                std::string>::type
        Get(const T2& d) {
        static_assert(
            _user_friendly_static_assert_<_has_std_to_string_<T2>::value, T2>::value,
            "fma_common::ToString() is not defined for this type.\n"
            "You can define it by defining one of the following functions:\n"
            "    1. std::string fma_common::ToString(const T&)\n"
            "    2. std::string T::ToString() const\n"
            "    3. static std::string fma_common::_detail::Type2String<T>::Get(const T&)");
        return "";
    }
};

template <typename K, typename V>
struct Type2String<std::pair<K, V>> {
    static std::string Get(const std::pair<K, V>& p) {
        return ToString(p.first) + ":" + ToString(p.second);
    }
};

template <typename K>
struct Type2String<std::optional<K>> {
    static std::string Get(const std::optional<K>& p) {
        return p.has_value() ? ToString(p.value()) : "nullopt";
    }
};

template <>
struct Type2String<std::string> {
    static inline std::string Get(const std::string& c) { return c; }
};

template <>
struct Type2String<const char*> {
    static inline std::string Get(const char* c) { return std::string(c); }
};

template <>
struct Type2String<char> {
    static inline std::string Get(const char& c) {
        std::string ret(3, '\'');
        ret[1] = c;
        return ret;
    }
};

template <typename ContainerT, typename ElementT>
struct ContainerPrinter {
    static std::string Get(const ContainerT& c) {
        std::string ret = "{<" + Type2String<size_t>::Get(c.size()) + ">: ";
        for (auto it = c.cbegin(); it != c.cend(); ++it) {
            if (it != c.cbegin()) ret += ", ";
            ret += ToString(*it);
        }
        ret += "}";
        return ret;
    }
};

template <size_t N>
struct PrintTuple {
    template <typename... Ts>
    static inline std::string Print(const std::tuple<Ts...>& tup) {
        auto& e = std::get<std::tuple_size<std::tuple<Ts...>>::value - N>(tup);
        return Type2String<typename std::decay<decltype(e)>::type>::Get(e) + ", " +
               PrintTuple<N - 1>::Print(tup);
    }
};

template <>
struct PrintTuple<1> {
    template <typename... Ts>
    static inline std::string Print(const std::tuple<Ts...>& tup) {
        auto& e = std::get<std::tuple_size<std::tuple<Ts...>>::value - 1>(tup);
        return Type2String<typename std::decay<decltype(e)>::type>::Get(e);
    }
};

template <typename... Ts>
struct Type2String<std::tuple<Ts...>> {
    static std::string Get(const std::tuple<Ts...>& tup) {
        using T = std::tuple<Ts...>;
        size_t size = std::tuple_size<T>::value;
        std::string ret = "{<" + Type2String<size_t>::Get(size) + ">: ";
        ret += _detail::PrintTuple<std::tuple_size<T>::value>::Print(tup);
        ret += "}";
        return ret;
    }
};

template <typename T>
struct Type2String<std::set<T>> : public ContainerPrinter<std::set<T>, T> {};

template <typename K, typename V, typename CompareFunc, typename Allocator>
struct Type2String<std::map<K, V, CompareFunc, Allocator>>
    : public ContainerPrinter<std::map<K, V, CompareFunc, Allocator>, std::pair<K, V>> {};

template <typename T>
struct Type2String<std::unordered_set<T>> : public ContainerPrinter<std::unordered_set<T>, T> {};

template <typename K, typename V>
struct Type2String<std::unordered_map<K, V>>
    : public ContainerPrinter<std::unordered_map<K, V>, std::pair<K, V>> {};

template <typename T>
struct Type2String<std::vector<T>> : public ContainerPrinter<std::vector<T>, T> {};

template <typename T>
struct Type2String<std::deque<T>> : public ContainerPrinter<std::deque<T>, T> {};

template <typename T>
struct Type2String<std::list<T>> : public ContainerPrinter<std::list<T>, T> {};

template <typename UNSIGNED>
inline char* PrintUintToBuffer(char* buf, UNSIGNED number) {
    char* p = buf;
    if (number == 0) {
        *p++ = '0';
    } else {
        char* p_first = p;
        while (number != 0) {
            *p++ = '0' + number % 10;
            number /= 10;
        }
        std::reverse(p_first, p);
    }
    return p;
}

template <typename SIGNED>
inline char* PrintIntToBuffer(char* buf, SIGNED number) {
    char* p = buf;
    if (number < 0) {
        *p++ = '-';
        number = -number;
    }
    return PrintUintToBuffer(p, number);
}

template <typename UNSIGNED>
inline std::string PrintUnsigned(UNSIGNED number) {
    char buf[32];
    char* p = PrintUintToBuffer(buf, number);
    return std::string(buf, p);
}

template <typename SIGNED>
inline std::string PrintSigned(SIGNED number) {
    char buf[32];
    char* p = PrintIntToBuffer(buf, number);
    return std::string(buf, p);
}

inline char* PrintIntReversed(char* buf, int64_t number, bool no_end_zero = false,
                              int* ndigits = nullptr) {
    int nd = 0;
    char* p = buf;
    char* digit_start = buf;
    if (number < 0) {
        *p++ = '-';
        number = -number;
        digit_start++;
    }
    if (number == 0) {
        *p++ = '0';
    } else {
        if (no_end_zero) {
            while (number % 10 == 0) {
                number /= 10;
                nd++;
            }
        }
        while (number != 0) {
            *p++ = '0' + number % 10;
            number /= 10;
        }
    }
    if (ndigits) *ndigits = nd + (int)(p - digit_start);
    return p;
}

inline double MyPow(int e) {
    // static const double POS_POW[10] = {
    //    1, 10, 100, 1000, 10000,
    //    100000, 1000000, 10000000, 100000000, 1000000000
    //};

    // static const double NEG_POW[10] = {
    //    1, 0.1, 0.01, 0.001, 0.0001,
    //    0.00001, 0.000001, 0.0000001, 0.00000001, 0.000000001
    //};

    // if (e >= 0 && e < 10) return POS_POW[e];
    // if (e <= 0 && e > -10) return NEG_POW[-e];
    return pow(10, e);
}

template <typename FLOAT>
inline std::string PrintFloat(FLOAT num) {
    if (num == 0) {
        return std::string("0");
    }
    char buf[32];
    double ipart, fpart;
    fpart = modf(num, &ipart);
    int exp = 0;
    char* p = buf;
    if (ipart == 0) {
        int e;
        frexp(fpart, &e);
        // raise fpart to integer
        e = -e * 3 / 10 + 9;
        int64_t fasi = (int64_t)(fpart * MyPow(e));
        int nd;
        p = PrintIntReversed(buf, fasi, true, &nd);
        char* pf = fasi < 0 ? buf + 1 : buf;
        exp = (int)(nd - 1 - e);
        if (fasi != 0 && p - pf > 1) {
            *p = *(p - 1);
            *(p - 1) = '.';
            p++;
        }
        std::reverse(pf, p);
    } else {
        if (ipart > std::numeric_limits<int32_t>::min() &&
            ipart < std::numeric_limits<int32_t>::max()) {
            int64_t ip = (int64_t)ipart;
            int nd;
            p = PrintIntReversed(buf, ip, fpart == 0, &nd);
            char* pf = ip < 0 ? buf + 1 : buf;
            if (p - pf > 1) {
                *p = *(p - 1);
                *(p - 1) = '.';
                p++;
            }
            std::reverse(pf, p);
            exp = nd - 1;
            if (nd == 1 && fpart != 0) {
                *p++ = '.';
            }
            fpart = fpart > 0 ? fpart : -fpart;
            for (int i = nd; i < 8 && fpart != 0; i++) {
                // print fractional part
                fpart *= 10;
                int t = (int)fpart;
                *p++ = '0' + t;
                fpart -= t;
            }
        } else {
            int e;
            frexp(ipart, &e);
            e = -e * 3 / 10 + 8;
            int64_t asi = (int64_t)(ipart * MyPow(e));
            int nd;
            p = PrintIntReversed(buf, asi, true, &nd);
            char* pf = asi < 0 ? buf + 1 : buf;
            exp = (int)(nd - 1 - e);
            if (asi != 0 && p - pf > 1) {
                *p = *(p - 1);
                *(p - 1) = '.';
                p++;
            }
            std::reverse(pf, p);
        }
    }
    if (exp != 0) {
        *p++ = 'e';
        p = PrintIntToBuffer(p, exp);
    }
    return std::string(buf, p);
}
}  // namespace _detail

/*!
 * \fn  template<class T> inline std::string ToString(const T& d)
 *
 * \brief   Convert this object into a string representation.
 *
 * \tparam  T   Type of data
 *
 * \param   d   data
 *
 * \return  std::string representation of data.
 */
template <class T>
inline std::string ToString(const T& d) {
    return _detail::Type2String<T>::Get(d);
}

/*!
 * \fn  inline std::string& ToString(const T& d)
 *
 * \brief   String specialization for ToString(std::string). Just returns
 *          a const reference to the same string.
 *
 * \param   d   data
 *
 * \return  const reference to the original string.
 */
inline const std::string& ToString(const std::string& d) { return d; }

inline std::string ToString(const char* str) { return std::string(str); }

template <int N>
inline std::string ToString(char str[N]) {
    return std::string(str, str + N);
}

inline std::string ToString(uint8_t n) { return _detail::PrintUnsigned(n); }

inline std::string ToString(int8_t n) { return _detail::PrintSigned(n); }

inline std::string ToString(uint16_t n) { return _detail::PrintUnsigned(n); }

inline std::string ToString(int16_t n) { return _detail::PrintSigned(n); }

inline std::string ToString(uint32_t n) { return _detail::PrintUnsigned(n); }

inline std::string ToString(int32_t n) { return _detail::PrintSigned(n); }

inline std::string ToString(uint64_t n) { return _detail::PrintUnsigned(n); }

inline std::string ToString(int64_t n) { return _detail::PrintSigned(n); }

inline std::string ToString(void* p) {
    char buf[24];
    int r = snprintf(buf, sizeof(buf), "%p", p);
    return std::string(buf, r);
}

inline std::string ToString(const std::vector<float>& fv) {
    std::string vec_str;
    for (size_t i = 0; i < fv.size(); i++) {
        if (fv.at(i) > 999999) {
            vec_str += std::to_string(fv.at(i)).substr(0, 7);
        } else {
            vec_str += std::to_string(fv.at(i)).substr(0, 8);
        }
        vec_str += ',';
    }
    if (!vec_str.empty()) {
        vec_str.pop_back();
    }
    return vec_str;
}

inline std::string ToString(float n) { return _detail::PrintFloat(n); }

inline std::string ToString(double n) { return _detail::PrintFloat(n); }

/*!
 * \fn  template<> inline std::string ToString<const char*>(const char* const & d)
 *
 * \brief   Const char* specialization for ToString(). Just constructs a
 *          a new string from the string literal.
 *
 * \param   d   A string literal.
 *
 * \return  std::string(d).
 */
template <>
inline std::string ToString<const char*>(const char* const& d) {
    return std::string(d);
}

/*!
 * \fn  inline bool StartsWith(const std::string & str, const std::string & pattern, bool
 * case_sensitive = true)
 *
 * \brief   Whether string str starts with substring pattern.
 *
 * \param   str             The string.
 * \param   pattern         Substring.
 * \param   case_sensitive  (Optional) True if case sensitive.
 *
 * \return  True if string str starts with pattern, otherwise false.
 */
inline bool StartsWith(const std::string& str, const std::string& pattern,
                       bool case_sensitive = true) {
    if (str.size() < pattern.size()) return false;
    for (size_t i = 0; i < pattern.size(); i++) {
        char p = pattern[i];
        char s = str[i];
        if (case_sensitive) {
            if (p != s) return false;
        } else {
            if (tolower(p) != tolower(s)) return false;
        }
    }
    return true;
}

/*!
 * \fn  inline bool EndsWith(const std::string& str, const std::string& pattern, bool case_sensitive
 * = true)
 *
 * \brief   Whether string str ends with substring pattern.
 *
 * \param   str             The string.
 * \param   pattern         Substring.
 * \param   case_sensitive  (Optional) True if case sensitive.
 *
 * \return  True if string str ends with pattern, otherwise false.
 */
inline bool EndsWith(const std::string& str, const std::string& pattern,
                     bool case_sensitive = true) {
    if (str.size() < pattern.size()) return false;
    const char* pp = &pattern[0];
    const char* sp = &str[str.size() - pattern.size()];
    for (size_t i = 0; i < pattern.size(); i++) {
        char p = pp[i];
        char s = sp[i];
        if (case_sensitive) {
            if (p != s) return false;
        } else {
            if (tolower(p) != tolower(s)) return false;
        }
    }
    return true;
}

inline std::string FilterString(const std::string& str, const std::string& to_be_removed) {
    std::string ret = str;
    auto e = std::remove_if(ret.begin(), ret.end(),
                            [=](char c) -> bool { return IsIn(c, to_be_removed); });
    ret.resize(e - ret.begin());
    return ret;
}

template <typename Iterator>
inline std::string JoinString(const Iterator& begin, const Iterator& end, const std::string& glue) {
    Iterator it = begin;
    std::string ret = *it++;
    for (; it != end; it++) {
        ret += glue + *it;
    }
    return ret;
}

template <typename T>
inline std::string ToHexString(const T& v) {
    std::string ret;
    boost::algorithm::hex(v.begin(), v.end(), std::back_inserter(ret));
    return ret;
}

}  // namespace fma_common
