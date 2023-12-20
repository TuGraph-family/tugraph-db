/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include <iostream>
#include <type_traits>
#include <vector>
#include <utility>
#include <string>

namespace geax::utils {

template <typename T>
class _has_std_to_string_ {
    template <typename U,
              typename _ = decltype(std::to_string(std::declval<U>()))>
    static std::true_type test(int);

    template <typename U>
    static std::false_type test(...);

 public:
    static bool const value = decltype(test<T>(0))::value;
};

template <typename T>
typename std::enable_if<_has_std_to_string_<T>::value, std::string>::type
ToString(const T& obj) {
    return std::to_string(obj);
}

inline std::string ToString(const std::string& obj) { return obj; }

template <typename T1, typename T2>
static inline std::string ToString(const std::pair<T1, T2>& pair) {
    return "(" + ToString(pair.first) + ", " + ToString(pair.second) + ")";
}

template <typename T>
static std::string ToString(const std::vector<T>& vec) {
    std::string str = "[";
    for (auto it = vec.cbegin(); it != vec.cend(); ++it) {
        if (it != vec.cbegin()) str += ", ";
        str += ToString(*it);
    }
    str += "]";
    return str;
}

}  // namespace geax::utils
