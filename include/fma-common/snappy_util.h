//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

/*!
 * \file    fma-common\snappy_util.h.
 *
 * \brief   Declares the snappy utility class.
 *
 * \author  Chuntao Hong
 *
 * \last_modified   2017/5/15.
 */
#pragma once
#include <string>
#include <vector>
#ifdef ENABLE_SNAPPY
#include "snappy/snappy-c.h"

namespace fma_common {
/*!
 * \fn  template<typename T> inline void SnappyCompress(const std::vector<T>& in, std::string& out)
 *
 * \brief   Compress a vector of T and store the result in out.
 *
 * \tparam  T   Element type
 * \param           in  The input vector.
 * \param [in,out]  out The output bytes.
 */
template <typename T>
inline void SnappyCompress(const std::vector<T>& in, std::string& out) {
    size_t max_len = snappy_max_compressed_length(in.size() * sizeof(T));
    out.resize(max_len);
    size_t len = max_len;
    snappy_status s = snappy_compress((const char*)(&in[0]), in.size() * sizeof(T), &out[0], &len);
    FMA_CHECK_EQ(s, SNAPPY_OK) << "Failed to compress data";
    out.resize(len);
}

/*!
 * \fn  template<typename T> inline void SnappyDecompress(const std::string& in, std::vector<T>&
 * out)
 *
 * \brief   Decompress the bytes buffer into a vector of T.
 *
 * \tparam  T   Element type
 * \param           in  The input compressed bytes.
 * \param [in,out]  out The output vector.
 */
template <typename T>
inline void SnappyDecompress(const std::string& in, std::vector<T>& out) {
    size_t len;
    snappy_status st = snappy_uncompressed_length(&in[0], in.size(), &len);
    FMA_CHECK_EQ(st, SNAPPY_OK) << "Failed to read the compressed data";
    FMA_CHECK_EQ(len % sizeof(T), 0) << "Size is not a multiple of sizeof(T)";
    out.resize(len / sizeof(T));
    st = snappy_uncompress(&in[0], in.size(), (char*)(&out[0]), &len);
    FMA_CHECK_EQ(st, SNAPPY_OK);
}
}  // namespace fma_common
#endif
