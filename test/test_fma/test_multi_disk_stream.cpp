/**
 * Copyright 2022 AntGroup CO., Ltd.
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

#include <cstdlib>

#include "fma-common/configuration.h"
#include "fma-common/binary_read_write_helper.h"
#include "fma-common/file_system.h"
#include "fma-common/logging.h"
#include "fma-common/multi_disk_stream.h"
#include "fma-common/string_util.h"
#include "fma-common/utils.h"
#include "./unit_test_utils.h"

using namespace fma_common;

FMA_SET_TEST_PARAMS(MultiDiskStream, "-f tmpfile -p a,b,c -s 1023",
                    "-f tmpfile -p a,b,c,d,e -s 1027 -u 1025");

FMA_UNIT_TEST(MultiDiskStream) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    typedef size_t T;
    Configuration parser;
    parser.Add<std::string>("file,f").Comment("File to write to");
    parser.Add<size_t>("size,s").Comment("Size of each int vector").SetDefault(65536);
    parser.Add<size_t>("nIter,n").Comment("Number of vectors to write").SetDefault(1024);
    parser.Add<size_t>("bufSize,u").Comment("Buffer size to use").SetDefault(64 << 20);
    parser.Add<size_t>("nRead,r").Comment("Number of times to read the data").SetDefault(1);
    parser.Add<std::string>("parts,p").Comment(
        "Part files for the multi-disk file, seperated by comma");
    parser.Parse(argc, argv);
    parser.Finalize();

    std::string path = parser.GetValue<std::string>("file");
    size_t vector_size = parser.GetValue<size_t>("size");
    size_t n_vectors = parser.GetValue<size_t>("nIter");
    size_t buf_size = parser.GetValue<size_t>("bufSize");
    size_t n_read = parser.GetValue<size_t>("nRead");
    std::string parts_str = parser.GetValue<std::string>("parts");
    std::vector<std::string> parts = Split(parts_str, ",");

    T sum = 0;
    std::vector<T> v(vector_size);
    for (size_t i = 0; i < v.size(); i++) {
        v[i] = i;
        sum += v[i];
    }

    FileSystem::GetFileSystem(path).Remove(path);
    double t1 = GetTime();
    OutputMultiDiskStream out(path, parts, buf_size, std::ofstream::trunc);
    for (size_t i = 0; i < n_vectors; i++) {
        BinaryWrite(out, v);
    }
    out.Close();
    double t2 = GetTime();
    LOG() << "Write completed at "
          << (double)vector_size * sizeof(T) * n_vectors / 1024 / 1024 / (t2 - t1) << "MB/s";

    for (size_t r = 0; r < n_read; r++) {
        double t = 0;
        InputMultiDiskStream in(path, buf_size);
        in.Seek(sizeof(size_t) + sizeof(T) * vector_size);
        for (size_t i = 0; i < n_vectors - 1; i++) {
            std::fill(v.begin(), v.end(), 0);
            t1 = GetTime();
            BinaryRead(in, v);
            t2 = GetTime();
            t += (t2 - t1);
            uint64_t s = 0;
            for (auto d : v) s += d;
            FMA_UT_CHECK_EQ(s, sum) << "inconsistent content";
        }
        LOG() << "Read completed at "
              << (double)vector_size * sizeof(T) * n_vectors / 1024 / 1024 / t << "MB/s";
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
