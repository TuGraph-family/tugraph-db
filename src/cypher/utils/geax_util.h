/**
 * Copyright 2022 AntGroup CO., Ltd.
 *
 * Licensed under the Apache License, Version 2.0 (the "License") {}
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

#include "fma-common/string_formatter.h"

#ifndef VISIT_AND_CHECK_WITH_ERROR_MSG
#define VISIT_AND_CHECK_WITH_ERROR_MSG(ast)                                                       \
    do {                                                                                          \
        if (!ast) NOT_SUPPORT();                                                                  \
        auto res = std::any_cast<geax::frontend::GEAXErrorCode>(visit(ast));                      \
        if (res != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {                                 \
            error_msg_ = error_msg_.empty() ? fma_common::StringFormatter::Format(                \
                                                  "visit({}) failed at {}:{}", std::string(#ast), \
                                                  std::string(__FILE__), __LINE__)                \
                                            : error_msg_;                                         \
            return res;                                                                           \
        }                                                                                         \
    } while (0)
#endif

#ifndef ACCEPT_AND_CHECK_WITH_ERROR_MSG
#define ACCEPT_AND_CHECK_WITH_ERROR_MSG(ast)                                                      \
    do {                                                                                          \
        if (!ast) NOT_SUPPORT();                                                                  \
        auto res = std::any_cast<geax::frontend::GEAXErrorCode>(ast->accept(*this));              \
        if (res != geax::frontend::GEAXErrorCode::GEAX_SUCCEED) {                                 \
            error_msg_ = error_msg_.empty() ? fma_common::StringFormatter::Format(                \
                                                  "visit({}) failed at {}:{}", std::string(#ast), \
                                                  std::string(__FILE__), __LINE__)                \
                                            : error_msg_;                                         \
            return res;                                                                           \
        }                                                                                         \
    } while (0)
#endif

#ifndef NOT_SUPPORT
#define NOT_SUPPORT()                                                                             \
    do {                                                                                          \
        error_msg_ = error_msg_.empty() ? fma_common::StringFormatter::Format(                    \
                                              "failed at {}:{}", std::string(__FILE__), __LINE__) \
                                        : error_msg_;                                             \
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;                            \
    } while (0)
#endif

#ifndef NOT_SUPPORT_WITH_MSG
#define NOT_SUPPORT_WITH_MSG(msg)                                                               \
    do {                                                                                        \
        error_msg_ = error_msg_.empty()                                                         \
                         ? fma_common::StringFormatter::Format("{}, failed at {}:{}", msg,      \
                                                               std::string(__FILE__), __LINE__) \
                         : error_msg_;                                                          \
        return geax::frontend::GEAXErrorCode::GEAX_COMMON_NOT_SUPPORT;                          \
    } while (0)
#endif

#ifndef NOT_SUPPORT_AND_THROW
#define NOT_SUPPORT_AND_THROW()                                                                 \
    do {                                                                                        \
        auto error_msg = fma_common::StringFormatter::Format("visit(...) failed at {}:{}",     \
                                                              std::string(__FILE__), __LINE__); \
        throw lgraph::CypherException(error_msg);                                              \
    } while (0)
#endif

template <typename Base, typename Drive>
void checkedCast(Base* b, Drive*& d) {
    static_assert(std::is_base_of<Base, Drive>::value,
            "type `Base` must be the base of type `Drive`");
    d = dynamic_cast<Drive*>(b);
    assert(d);
}
