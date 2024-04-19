//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\utils.h.
 *
 * \brief   Declares the utilities class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <stdio.h>
#ifdef _WIN32
#define popen _popen
#define pclose _pclose
#endif

#include <chrono>
#include <cstdlib>
#include <thread>

#include "fma-common/env.h"
#include "fma-common/assert.h"
#include "fma-common/predefs.h"
#include "fma-common/string_util.h"
#include "fma-common/type_traits.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif

namespace fma_common {
namespace _detail {
inline const std::string& EMPTY_STRING() {
    static std::string s = "";
    return s;
}
}  // namespace _detail

static inline double GetTime() {
    using namespace std::chrono;
    high_resolution_clock::duration tp = high_resolution_clock::now().time_since_epoch();
    return (double)tp.count() * high_resolution_clock::period::num /
           high_resolution_clock::period::den;
}

#ifdef _WIN32
/**
 * Compare and swap
 *
 * @tparam T Generic type parameter.
 * @param [in,out] orig     Pointer to original.
 * @param          expected The expected value of the *orig.
 * @param          newv     The newv.
 *
 * @return The old value of *orig. If success, it should equal to expected.
 */
template <typename T>
inline typename std::enable_if<sizeof(T) == 8, T>::type CompareAndSwap(T* orig, T expected,
                                                                       T newv) {
    int64_t r =
        InterlockedCompareExchange64((int64_t*)orig, *(int64_t*)&newv, *(int64_t*)&expected);
    return *(T*)&r;
}

template <typename T>
inline typename std::enable_if<sizeof(T) == 4, T>::type CompareAndSwap(T* orig, T expected,
                                                                       T newv) {
    uint32_t r =
        InterlockedCompareExchange((uint32_t*)orig, *(uint32_t*)&newv, *(uint32_t*)&expected);
    return *(T*)&r;
}

/**
 * Atomic fetch add
 *
 * @tparam T Int types, such as int, long, short,...
 * @param [in,out] p Pointer to the value we want to fetch and add
 * @param          v Increment value
 *
 * @return Original value before add.
 */
template <typename T>
inline typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 4, T>::type
AtomicFetchAdd(T* p, T v) {
    return InterlockedExchangeAdd((uint32_t*)p, (uint32_t)v);
}

template <typename T>
inline typename std::enable_if<std::is_integral<T>::value && sizeof(T) == 8, T>::type
AtomicFetchAdd(T* p, T v) {
    return InterlockedExchangeAdd64((int64_t*)p, (int64_t)v);
}
#else
/* Tell gcc not to warn about aliasing (e.g., sockaddr_in) in this file.  */
#if (__GNUC__ == 4 && 3 <= __GNUC_MINOR__) || 4 < __GNUC__
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

/**
 * Compare and swap
 *
 * @tparam T Generic type parameter.
 * @param [in,out] orig     Pointer to original.
 * @param          expected The expected value of the *orig.
 * @param          newv     The newv.
 *
 * @return The old value of *orig. If success, it should equal to expected.
 */
template <typename T>
inline typename std::enable_if<sizeof(T) == 8, T>::type CompareAndSwap(T* orig, T expected,
                                                                       T newv) {
    int64_t r = __sync_val_compare_and_swap((int64_t*)orig, *(int64_t*)&expected, *(int64_t*)&newv);
    return *(T*)&r;
}

template <typename T>
inline typename std::enable_if<sizeof(T) == 4, T>::type CompareAndSwap(T* orig, T expected,
                                                                       T newv) {
    int32_t r = __sync_val_compare_and_swap((int32_t*)orig, *(int32_t*)&expected, *(int32_t*)&newv);
    return *(T*)&r;
}

/**
 * Atomic fetch add
 *
 * @tparam T Generic type parameter.
 * @param [in,out] p Pointer to the value to be added into
 * @param          v Value to add
 *
 * @return Original value of *p
 */
template <typename T>
inline typename std::enable_if<std::is_integral<T>::value, T>::type AtomicFetchAdd(T* p, T v) {
    return __atomic_fetch_add(p, v, __ATOMIC_SEQ_CST);
}
#endif

/*!
 * \fn  inline void AtomicAdd(double* orig, double added)
 *
 * \brief   Atomic add a double value.
 *
 * \param [in,out]  orig    Pointer to the double we want to add on.
 * \param           added   The added value.
 */
inline void AtomicAdd(double* orig, double added) {
    double orig_value = *orig;
    double expected;
    do {
        expected = orig_value;
        double new_value = expected + added;
        orig_value = CompareAndSwap(orig, expected, new_value);
    } while (expected != orig_value);
}

inline void AtomicAdd(float* orig, float added) {
    float orig_value = *orig;
    float expected;
    do {
        expected = orig_value;
        float new_value = expected + added;
        orig_value = CompareAndSwap(orig, expected, new_value);
    } while (expected != orig_value);
}

/*!
 * \fn  inline void SleepUs(size_t us)
 *
 * \brief   Sleep for some microseconds.
 *
 * \param   us  The number of microseconds.
 */
inline void SleepUs(size_t us) { std::this_thread::sleep_for(std::chrono::microseconds(us)); }

/*!
 * \fn  inline void SleepS(double second)
 *
 * \brief   Sleep for some seconds.
 *
 * \param   second  The number of seconds.
 */
inline void SleepS(double second) { SleepUs((size_t)(second * 1000000)); }

/*!
 * \fn  inline int ExecCmd(const std::string& cmd, bool supress_output = true)
 *
 * \brief   Executes the command line, supressing all outputs.
 *
 * \param   cmd             The command.
 * \param   supress_output  (Optional) True to supress output.
 *
 * \return  0 on success.
 */
inline int ExecCmd(const std::string& cmd, bool supress_output = true) {
    if (supress_output) {
        return system((cmd + SUPRESS_OUTPUT()).c_str());
    } else {
        return system(cmd.c_str());
    }
}

inline int ExecCmdThroughPopen(const std::string& cmd, std::ostream& os) {
    FILE* pipe = popen(cmd.c_str(), "r");
    if (pipe == NULL) {
        return -1;
    }
    char buffer[1024];
    for (;;) {
        size_t nr = fread(buffer, 1, sizeof(buffer), pipe);
        if (nr != 0) {
            os.write(buffer, nr);
        }
        if (nr != sizeof(buffer)) {
            if (feof(pipe)) {
                break;
            } else if (ferror(pipe)) {
                break;
            }
        }
    }
    const int wstatus = pclose(pipe);
#ifdef __GNUC__
    if (wstatus < 0) {
        return wstatus;
    }
    if (WIFEXITED(wstatus)) {
        return WEXITSTATUS(wstatus);
    }
    if (WIFSIGNALED(wstatus)) {
        os << "Child process was killed by signal " << WTERMSIG(wstatus);
    }
    errno = ECHILD;
    return -1;
#else
    return wstatus;
#endif
}

inline double DoubleDecimalPlaces(double d, int num) {
    auto pow = std::pow(10, num);
    auto i = std::round(d * pow);
    return (i / pow);
}

namespace _detail {
inline FILE* OpenPipe(const std::string& cmd, const char* mode, bool redirect_stderr = false) {
    std::string c = cmd;
    if (redirect_stderr) c += " 2>&1 ";
    FILE* f = popen(c.c_str(), mode);
    return f;
}
}  // namespace _detail
}  // namespace fma_common
