//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\env.h.
 *
 * \brief   Declares the environment class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/12.
 */

#pragma once

#include "fma-common/string_util.h"

#ifdef _WIN32
#pragma warning(disable : 4996)  // disable security warnings for getenv
#endif

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include <unistd.h>
#endif

namespace fma_common {
/*!
 * \class   Env
 *
 * \brief   Utility class to get environmental variables.
 */
class Env {
 public:
    /*!
     * \fn  static bool Env::MpiDebug()
     *
     * \brief   Determines if MPI_DEBUG is set
     *
     * \return  True if MPI_DEBUG is set to true
     */
    static bool MpiDebug() {
        const char* cs = getenv("MPI_DEBUG");
        if (cs == nullptr) {
            return false;
        } else {
            bool d = false;
            ParseString(cs, d);
            return d;
        }
    }

    /*!
     * \fn  static bool Env::MpiLog()
     *
     * \brief   Determines if MPI_LOG is set.
     *
     * \return  True if MPI_LOG is set to true
     */
    static bool MpiLog() {
        const char* cs = getenv("MPI_LOG");
        if (cs == nullptr) {
            return false;
        } else {
            bool d = false;
            ParseString(cs, d);
            return d;
        }
    }

    /*!
     * \fn    template<typename T> static bool Env::GetVariable(const std::string& name, T& value)
     *
     * \brief    Gets an enviormental variable.
     *
     * \tparam    T    Generic type parameter.
     *
     * \param               name     The name of the variable.
     * \param [in,out]    value    The value to store the variable value, if exists.
     *
     * \return    True if it the value exists and can be successfully parsed
     *             as T, false otherwise.
     */
    template <typename T>
    static bool GetVariable(const std::string& name, T& value) {
        const char* cs = getenv("MPI_LOG");
        if (cs == nullptr) {
            return false;
        } else {
            return ParseString<T>(cs, value);
        }
    }
};
}  // namespace fma_common
