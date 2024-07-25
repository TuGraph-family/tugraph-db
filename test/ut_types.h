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

#include "fma-common/string_util.h"

namespace lgraph::ut {

enum class QUERY_TYPE {
    CYPHER,
    GQL,
    NEWCYPHER
};

static std::string ToString(const QUERY_TYPE n) {
    static const std::unordered_map<QUERY_TYPE, std::string> map = {
        {QUERY_TYPE::CYPHER, "CYPHER"},
        {QUERY_TYPE::GQL, "GQL"},
        {QUERY_TYPE::NEWCYPHER, "NEWCYPHER"},
    };
    auto it = map.find(n);
    if (it == map.end()) {
        return "UNKNOWN";
    } else {
        return it->second;
    }
}

static bool ToType(const std::string& str, QUERY_TYPE& n) {
    static const std::unordered_map<std::string, QUERY_TYPE> map = {
        {"CYPHER", QUERY_TYPE::CYPHER},
        {"GQL", QUERY_TYPE::GQL},
        {"NEWCYPHER", QUERY_TYPE::NEWCYPHER},
    };
    auto it = map.find(fma_common::ToUpper(str));
    if (it == map.end()) {
        return false;
    } else {
        n = it->second;
        return true;
    }
}

}  // namespace lgraph::ut

namespace fma_common::_detail {
template <>
class String2Type<lgraph::ut::QUERY_TYPE> {
 public:
    static bool Get(const std::string& str, lgraph::ut::QUERY_TYPE& n) {
        return lgraph::ut::ToType(fma_common::ToUpper(str), n);
    }
};

template <>
class Type2String<lgraph::ut::QUERY_TYPE> {
 public:
    static std::string Get(const lgraph::ut::QUERY_TYPE& n) {
        return lgraph::ut::ToString(n);
    }
};

}  // namespace fma_common::_detail
