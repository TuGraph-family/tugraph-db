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

#include <atomic>
#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/text_dir_stream.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

void ReaderThread(const std::string &dir, size_t n_readers, size_t id, size_t buf_size,
                  size_t n_parallel, bool dump_output, std::atomic<size_t> &total_lines,
                  std::atomic<size_t> &total_bytes) {
    InputTextDirStream in(dir, n_readers, id, buf_size, n_parallel);
    std::string buf(64 << 20, 0);
    size_t n_bytes = 0;
    size_t n_lines = 0;
    size_t read = 0;
    while ((read = in.Read(&buf[0], buf.size()))) {
        n_bytes += read;
        for (size_t i = 0; i < read; i++) {
            if (buf[i] == '\n') n_lines++;
        }
        if (dump_output) {
            LOG() << "[" << id << "] " << buf.substr(0, read);
        }
    }
    LOG() << "[" << id << "] bytes read = " << in.GetBytesRead();
    total_lines += n_lines;
    total_bytes += n_bytes;
}

#ifndef WIN32
FMA_SET_TEST_PARAMS(TextDirStream, "--dir ../../include/fma-common",
                    "--dir ../../include/fma-common --bufSize 333 --nParallel 3 --nReaders 3");
#endif

FMA_UNIT_TEST(TextDirStream) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    std::string dir;
    size_t buf_size = 64 << 20;
    size_t n_parallel = 1;
    bool write_out = false;
    size_t n_readers = 1;
    Configuration config;
    config.Add(dir, "dir").Comment("Dir to load");
    config.Add(buf_size, "bufSize", true).Comment("Buffer size");
    config.Add(n_parallel, "nParallel", true).Comment("Number of files to load in parallel");
    config.Add(n_readers, "nReaders", true).Comment("Number of reader threads to use");
    config.Add(write_out, "write", true).Comment("Whether to write content to stdout");
    config.Parse(argc, argv);
    config.Finalize();

    std::atomic<size_t> total_lines(0);
    std::atomic<size_t> total_bytes(0);
    double t1 = GetTime();
    std::vector<std::thread> workers;
    for (size_t i = 0; i < n_readers; i++) {
        workers.emplace_back(
            [dir, buf_size, n_parallel, n_readers, i, write_out, &total_lines, &total_bytes]() {
                ReaderThread(dir, n_readers, i, buf_size, n_parallel, write_out, total_lines,
                             total_bytes);
            });
    }
    for (auto &w : workers) w.join();
    double t2 = GetTime();
    LOG() << "Read " << total_bytes << " bytes at " << (double)total_bytes / 1024 / 1024 / (t2 - t1)
          << "MB/s";
    LOG() << "Number of lines: " << total_lines;

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
