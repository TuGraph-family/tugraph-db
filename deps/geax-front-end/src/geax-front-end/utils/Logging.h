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

#ifndef FRONTEND_UTILS_LOGGING_H_
#define FRONTEND_UTILS_LOGGING_H_

#include <unistd.h>
#include <iostream>
#ifdef GEAX_ENABLE_GLOG
#include <glog/logging.h>
#include <gflags/gflags.h>
#endif
#include "geax-front-end/utils/Copilot.h"

#ifdef GEAX_ENABLE_GLOG
#ifdef LOG
#undef LOG
#endif
#define __LOG_INFO \
    if (FLAGS_minloglevel <= google::GLOG_INFO) /* NOLINT */ \
        COMPACT_GOOGLE_LOG_INFO.stream()
#define __LOG_WARNING \
    if (FLAGS_minloglevel <= google::GLOG_WARNING) /* NOLINT */ \
        COMPACT_GOOGLE_LOG_WARNING.stream()
#define __LOG_ERROR \
    if (FLAGS_minloglevel <= google::GLOG_ERROR) /* NOLINT */ \
        COMPACT_GOOGLE_LOG_ERROR.stream()
#define __LOG_FATAL \
    COMPACT_GOOGLE_LOG_FATAL.stream() /* NOLINT */
#ifdef NDEBUG
#define __LOG_DFATAL __LOG_ERROR
#elif GOOGLE_STRIP_LOG <= 3
#define __LOG_DFATAL COMPACT_GOOGLE_LOG_DFATAL.stream()
#endif

#define LOG(severity) __LOG_ ## severity

#else
#define LOG(severity) std::cout
#endif


// Key-value format printers are prefixed with K*
//
// K(v) will expand v to "v=value of v" format, for instance,
// auto ret = GEAXErrorCode::GEAX_SUCCEED;
// K(ret) will be expanded to "ret=GEAX_SUCCEED"
#ifndef K
#define K(v) K_I(v)
#define K_I(v) #v << "=" << v
#endif
// KV(k, v) indicates a KV format but with specified k
#ifndef KV
#define KV(k, v) KV_I(k, v)
#define KV_I(k, v) k << "=" << v
#endif
// KP(v) indicates Kv format of Pointer address (KP),
// which will expand v to "v(address of pointer v)"
#ifndef KP
#define KP(v) KP_I(v)
#define KP_I(v) #v << "(" << reinterpret_cast<const void*>(v) << ")"
#endif
// Multiple KVs
// Maximumn number of args is 16.
#ifndef KM
#define GEAX_PP_KV_DELIMETER GEAX_PP_KV_DELIMETER_I
#define GEAX_PP_KV_DELIMETER_I << ", " <<
#define KM(...) GEAX_PP_EXPR_ARG(K, GEAX_PP_KV_DELIMETER, __VA_ARGS__)
#endif
// TODO(boyao.zby) implement these two macros
// Multiple KVs with same key
// #ifndef KMV
// #endif
// Multiple KVs with different keys
// #ifndef MKV
// #endif
// Multiple pointers
#ifndef KMP
#define GEAX_PP_KP_DELIMETER GEAX_PP_KP_DELIMETER_I
#define GEAX_PP_KP_DELIMETER_I << ", " <<
#define KMP(...) GEAX_PP_EXPR_ARG(KP, GEAX_PP_KP_DELIMETER, __VA_ARGS__)
#endif
// Range pointers
#ifndef KRP
#define KRP(begin, end) KRP_I(begin, end)
#define KRP_I(begin, end) RangeToString((begin), (end))
#endif

// Key-value format printers that are prefixed with D* are prefixed with
// a delimiter ", ".
//
// DK(v) puts a delimiter before K(v)
#ifndef DK
#define DK(v) DK_I(v)
#define DK_I(v) ", " << K(v)
#endif
// DKV(k, v) puts a delim
#ifndef DKV
#define DKV(k, v) DKV_I(k, v)
#define DKV_I(k, v) ", " << KV(k, v)
#endif
// DKP(v) puts a delimiter before KP(v)
#ifndef DKP
#define DKP(v) DKP_I(v)
#define DKP_I(v) ", " << KP(v)
#endif
// DKM(...) puts a delimiter before KM(...)
#ifndef DKM
#define DKM(...) DKM_I(__VA_ARGS__)
#define DKM_I(...) ", " << KM(__VA_ARGS__)
#endif
// DKMP(...) puts a delimiter before KMP(...)
#ifndef DKMP
#define DKMP(...) DKMP_I(__VA_ARGS__)
#define DKMP_I(...) ", " << KMP(__VA_ARGS__)
#endif

// Key-value format printers that are suffixed with D* are suffixed with
// a delimeter ", ".
//
// KD(v) puts a delimiter after K(v)
#ifndef KD
#define KD(v) KD_I(v)
#define KD_I(v) K(v) << ", "
#endif


template <typename T>
std::ostream&
operator<<(std::enable_if_t<std::is_enum<T>::value,
                            std::ostream>& strm,
           const T& val) {
    return strm << ToString(val);
}

#endif  // FRONTEND_UTILS_LOGGING_H_
