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

#include "geax-front-end/GEAXErrorCode.h"
#include <unordered_map>

namespace geax {
namespace frontend {

const std::string& ToString(const GEAXErrorCode code) {
    static const std::string ret = "UNKNOWN_ERROR_CODE";

    static std::unordered_map<GEAXErrorCode, std::string> dict = {
        {GEAXErrorCode::GEAX_SUCCEED, "GEAX_SUCCEED"},
        {GEAXErrorCode::GEAX_ERROR, "GEAX_ERROR"},
        {GEAXErrorCode::GEAX_OOPS, "GEAX_OOPS"},
        {GEAXErrorCode::GEAX_COMMON_INVALID_ARGUMENT, "GEAX_COMMON_INVALID_ARGUMENT"},
        {GEAXErrorCode::GEAX_COMMON_NULLPTR, "GEAX_COMMON_NULLPTR"},
        {GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT, "GEAX_COMMON_NOT_SUPPORT"},
        {GEAXErrorCode::GEAX_COMMON_PARSE_ERROR, "GEAX_COMMON_PARSE_ERROR"},
        {GEAXErrorCode::GEAX_COMMON_KEY_NOT_FOUND, "GEAX_COMMON_KEY_NOT_FOUND"},
        {GEAXErrorCode::GEAX_COMMON_SYNTAX_ERROR, "GEAX_COMMON_SYNTAX_ERROR"},
    };

    auto it = dict.find(code);
    if (it != dict.end()) {
        return it->second;
    }
    return ret;
}

}  // namespace frontend
}  // namespace geax
