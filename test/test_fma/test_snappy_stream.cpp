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

#include <vector>

#include "fma-common/configuration.h"
#include "fma-common/local_file_stream.h"
#include "fma-common/logging.h"
#include "fma-common/snappy_stream.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"
#include "./rand_r.h"

using namespace fma_common;

FMA_UNIT_TEST(SnappyStream) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    ArgParser parser;
    parser.Add<int>("blockSize,b")
        .Comment("snappy block size to use")
        .SetDefault(1024 * 1024 * 128);
    parser.Add<int>("nBlocks,n").Comment("number of blocks to write").SetDefault(1);
    parser.Add<int>("nBuffers,f").Comment("number of buffers to use").SetDefault(1);
    parser.Add("file").SetDefault("snappy.out").Comment("file to use");
    parser.Parse(argc, argv);
    parser.Finalize();

    int block_size = parser.GetValue<int>("blockSize");
    block_size = block_size / 4 * 4;
    int n_blocks = parser.GetValue<int>("nBlocks");
    // int n_buffers = parser.GetValue<int>("nBuffers");
    std::string path = parser.GetValue("file");

    int64_t sum = 0;
    std::vector<char> block(block_size);
    int *p = (int *)(&block[0]);
    for (int i = 0; i < block_size / 4; i++) {
        int d = myrand() % 255;
        p[i] = d;
        sum += d;
    }
    sum *= n_blocks;

    double t1 = GetTime();
#if ENABLE_SNAPPY
    SnappyOutputStream out(path, n_buffers, block_size, std::ofstream::trunc);
#else
    OutputLocalFileStream out(path, std::ofstream::trunc);
#endif
    for (int i = 0; i < n_blocks; i++) {
        out.Write(&block[0], block_size);
    }
    out.Close();
    double t2 = GetTime();
    LOG() << "write: " << (double)n_blocks * block_size / 1024 / 1024 / (t2 - t1) << "MB/s";

    t1 = GetTime();
#if ENABLE_SNAPPY
    SnappyInputStream in(path, n_buffers);
#else
    InputLocalFileStream in(path);
#endif
    std::string buf;
    int64_t sum2 = 0;
    buf.resize(block_size);
    while (in.Read(&buf[0], block_size)) {
        size_t n_elements = buf.size() / 4;
        int *p = (int *)buf.data();
        for (size_t i = 0; i < n_elements; i++) {
            sum2 += p[i];
        }
    }
    FMA_UT_CHECK_EQ(sum, sum2);
    t2 = GetTime();
    LOG() << "read: " << (double)n_blocks * block_size / 1024 / 1024 / (t2 - t1) << "MB/s";

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
