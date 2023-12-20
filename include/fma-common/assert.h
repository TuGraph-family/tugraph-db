//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include "fma-common/env.h"
#include "tools/lgraph_log.h"

namespace fma_common {
#define FMA_CHECK(pred)     if (!(pred))       LOG_FATAL() << "CHECK("#pred")      failed"
#define FMA_CHECK_EQ(a, b)  if (!((a) == (b))) LOG_FATAL() << "CHECK("#a" == "#b") failed"
#define FMA_CHECK_NEQ(a, b) if (!((a) != (b))) LOG_FATAL() << "CHECK("#a" != "#b") failed"
#define FMA_CHECK_LT(a, b)  if (!((a) < (b)))  LOG_FATAL() << "CHECK("#a" < "#b")  failed"
#define FMA_CHECK_LE(a, b)  if (!((a) <= (b))) LOG_FATAL() << "CHECK("#a" <= "#b") failed"
#define FMA_CHECK_GT(a, b)  if (!((a) > (b)))  LOG_FATAL() << "CHECK("#a" > "#b")  failed"
#define FMA_CHECK_GE(a, b)  if (!((a) >= (b))) LOG_FATAL() << "CHECK("#a" >= "#b") failed"
#define FMA_ASSERT(pred)    if (!(pred))       LOG_FATAL() << "ASSERT("#pred")     failed"

#ifndef NDEBUG
#define FMA_DBG_CHECK(pred)     FMA_CHECK(pred)
#define FMA_DBG_CHECK_EQ(a, b)  FMA_CHECK_EQ(a, b)
#define FMA_DBG_CHECK_NEQ(a, b) FMA_CHECK_NEQ(a, b)
#define FMA_DBG_CHECK_LT(a, b)  FMA_CHECK_LT(a, b)
#define FMA_DBG_CHECK_LE(a, b)  FMA_CHECK_LE(a, b)
#define FMA_DBG_CHECK_GT(a, b)  FMA_CHECK_GT(a, b)
#define FMA_DBG_CHECK_GE(a, b)  FMA_CHECK_GE(a, b)
#define FMA_DBG_ASSERT(pred)    FMA_ASSERT(pred)
#else
#define FMA_DBG_CHECK(pred)     (void)(true || (pred))
#define FMA_DBG_CHECK_EQ(a, b)  (void)(true || ((a) == (b)))
#define FMA_DBG_CHECK_NEQ(a, b) (void)(true || ((a) != (b)))
#define FMA_DBG_CHECK_LT(a, b)  (void)(true || ((a) < (b)))
#define FMA_DBG_CHECK_LE(a, b)  (void)(true || ((a) <= (b)))
#define FMA_DBG_CHECK_GT(a, b)  (void)(true || ((a) > (b)))
#define FMA_DBG_CHECK_GE(a, b)  (void)(true || ((a) >= (b)))
#define FMA_DBG_ASSERT(pred)    (void)(true || (pred))
#endif
}  // namespace fma_common
