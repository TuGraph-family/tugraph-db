//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once
#include "fma-common/logger.h"

namespace fma_common {
#define DBG() FMA_DBG()
#define LOG() FMA_LOG()
#define WARN() FMA_WARN()
#define ERR() FMA_ERR()
#define FATAL() FMA_FATAL()
#define CHECK(pred) FMA_CHECK(pred)
#define CHECK_EQ(a, b) FMA_CHECK_EQ(a, b)
#define CHECK_NEQ(a, b) FMA_CHECK_NEQ(a, b)
#define CHECK_LT(a, b) FMA_CHECK_LT(a, b)
#define CHECK_LE(a, b) FMA_CHECK_LE(a, b)
#define CHECK_GT(a, b) FMA_CHECK_GT(a, b)
#define CHECK_GE(a, b) FMA_CHECK_GE(a, b)
#define ASSERT(pred) FMA_ASSERT(pred)
#define DBG_ASSERT(pred) FMA_DBG_ASSERT(pred)
#define DBG_CHECK(pred) FMA_DBG_CHECK(pred)
#define DBG_CHECK_EQ(a, b) FMA_DBG_CHECK_EQ(a, b)
#define DBG_CHECK_NEQ(a, b) FMA_DBG_CHECK_NEQ(a, b)
#define DBG_CHECK_LT(a, b) FMA_DBG_CHECK_LT(a, b)
#define DBG_CHECK_LE(a, b) FMA_DBG_CHECK_LE(a, b)
#define DBG_CHECK_GT(a, b) FMA_DBG_CHECK_GT(a, b)
#define DBG_CHECK_GE(a, b) FMA_DBG_CHECK_GE(a, b)
#define EXIT() FMA_EXIT()
}  // namespace fma_common
