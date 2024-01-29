//  Copyright 2024 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\predefs.h.
 *
 * \brief   Declares the pre-defined variables.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once

#include <string>
#include <vector>

#include "fma-common/utils.h"

namespace fma_common {
/*!
 * \fn  inline const std::string& HDFS_CMD()
 *
 * \brief   Hdfs command to use. It must be directly callable. So if hdfs
 *          is not in PATH, you must specify the full path to it.
 *
 * \return  A reference to a const std::string.
 */
inline const std::string& HDFS_CMD() {
    static const std::string hdfs = "hdfs dfs ";
    return hdfs;
}

#ifdef _WIN32
inline const std::string& SUPRESS_OUTPUT() {
    static const std::string r = " >NUL 2>&1";
    // static const std::string r = " >NUL";
    return r;
}
#else
static inline const std::string& SUPRESS_OUTPUT() {
    static const std::string r = " >/dev/null 2>&1";
    return r;
}
#endif
}  // namespace fma_common
