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

#include <string>
#include <vector>

#include "fma-common/configuration.h"
#include "fma-common/local_file_stream.h"
#include "fma-common/file_stream.h"
#include "fma-common/file_system.h"
#include "fma-common/logging.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

FMA_SET_TEST_PARAMS(FileStream, "-f tmpfile");

FMA_UNIT_TEST(FileStream) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    std::string path;
    size_t buffer_size = 16384;
    size_t block_size = 4 << 20;
    size_t n_blocks = 13;
    Configuration config;
    config.Add(path, "file,f", false).Comment("Temp file to write to");
    config.Add(buffer_size, "buffer,b", true).Comment("Buffer size to use, in B");
    config.Add(block_size, "blockSize", true).Comment("Block size to use per each read, in B");
    config.Add(n_blocks, "nBlocks", true).Comment("Number of blocks to write");
    config.Parse(argc, argv);
    config.Finalize();

    LOG() << "Using file " << path << "\n"
          << "\tBuffer size: " << buffer_size << " B\n"
          << "\tBlock size: " << block_size << " B\n"
          << "\tNumber of blocks: " << n_blocks;

    std::string block(block_size, 1);
    double t1 = GetTime();
    {
        file_system::RemoveFile(path);
        OutputLocalFileStream out(path, buffer_size);
        for (size_t i = 0; i < n_blocks; i++) {
            out.Write(&block[0], block_size);
        }
    }
    double t2 = GetTime();
    double mb = (double)block_size * n_blocks / 1024 / 1024;
    LOG() << "Wrote " << mb << "MB, at " << mb / (t2 - t1) << "MB/s";

    std::fill(block.begin(), block.end(), 0);
    double sum = 0;
    double compute_time = 0;
    t1 = GetTime();
    {
        InputLocalFileStream in(path, buffer_size);
        FMA_UT_CHECK_EQ(in.Size(), block_size * n_blocks)
            << "Written bytes does not equal file size";
        uint64_t total_bytes = 0;
        while (true) {
            uint64_t bytes = in.Read(&block[0], block_size);
            total_bytes += bytes;
            if (bytes != block_size) break;
            double tt1 = GetTime();
            for (auto c : block) sum += c;
            std::fill(block.begin(), block.end(), 0);
            double tt2 = GetTime();
            compute_time += tt2 - tt1;
        }
        FMA_UT_CHECK_EQ(total_bytes, in.Size()) << "Read bytes does not equal file size";
    }
    t2 = GetTime();
    LOG() << "Read " << mb << "MB, at " << mb / (t2 - t1 - compute_time) << "MB/s";
    FMA_UT_CHECK_EQ(sum, (double)n_blocks * block_size) << "Data is probably corruptted";
    file_system::RemoveFile(path);

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
