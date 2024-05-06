//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/**
 *  @file lgraph_atomic.h
 *  @brief Implementation of atomic operations, used in lgraph_traversal.
 */

#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <atomic>

/* Tell gcc not to warn about aliasing (e.g., sockaddr_in) in this file.  */
#if (__GNUC__ == 4 && 3 <= __GNUC_MINOR__) || 4 < __GNUC__
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

namespace lgraph_api {
template <class T>
inline bool cas(T *ptr, T oldv, T newv) {
    if (sizeof(T) == 8) {
        return __sync_bool_compare_and_swap((int64_t *)ptr, *((int64_t *)&oldv),
                                            *((int64_t *)&newv));
    } else if (sizeof(T) == 4) {
        return __sync_bool_compare_and_swap((int *)ptr, *((int *)&oldv), *((int *)&newv));
    } else {
        fprintf(stderr, "length not supported by cas.\n");
        exit(-1);
    }
}

template <class T>
inline bool write_min(T *a, T b) {
    T c;
    bool r = 0;
    do {
        c = *a;
    } while (c > b && !(r = cas(a, c, b)));
    return r;
}

template <class T>
inline bool write_max(T *a, T b) {
    T c;
    bool r = 0;
    do {
        c = *a;
    } while (c < b && !(r = cas(a, c, b)));
    return r;
}

template <class T>
inline void write_add(T *a, T b) {
    volatile T newV, oldV;
    do {
        oldV = *a;
        newV = oldV + b;
    } while (!cas(a, oldV, newV));
}

inline void write_add(uint64_t *a, uint64_t b) { __sync_fetch_and_add(a, b); }

inline void write_add(uint32_t *a, uint32_t b) { __sync_fetch_and_add(a, b); }

inline void write_add(int64_t *a, int64_t b) { __sync_fetch_and_add(a, b); }

inline void write_add(int32_t *a, int32_t b) { __sync_fetch_and_add(a, b); }

template <class T>
inline void write_sub(T *a, T b) {
    volatile T newV, oldV;
    do {
        oldV = *a;
        newV = oldV - b;
    } while (!cas(a, oldV, newV));
}

inline void write_sub(uint64_t *a, uint64_t b) { __sync_fetch_and_sub(a, b); }

inline void write_sub(uint32_t *a, uint32_t b) { __sync_fetch_and_sub(a, b); }

inline void write_sub(int64_t *a, int64_t b) { __sync_fetch_and_sub(a, b); }

inline void write_sub(int32_t *a, int32_t b) { __sync_fetch_and_sub(a, b); }
}  // namespace lgraph_api
