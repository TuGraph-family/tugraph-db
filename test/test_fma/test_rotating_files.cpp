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

#include "fma-common/configuration.h"
#include "fma-common/rotating_files.h"
#include "./unit_test_utils.h"

FMA_SET_TEST_PARAMS(RotatingFiles, "");

FMA_UNIT_TEST(RotatingFiles) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    using namespace fma_common;

    size_t n_writes = 128;
    size_t byte_per_write = 64;
    size_t max_file_size = 1024;
    std::string dir = "binlogs";
    {
        file_system::RemoveDir(dir);
        size_t write_per_file = (max_file_size + byte_per_write - 1) / byte_per_write;
        std::unique_ptr<IndexedFileNameGenerator> fgen(new IndexedFileNameGenerator("bing_log_"));
        RotatingFiles rf(dir, std::move(fgen), max_file_size);
        {
            for (size_t i = 0; i < n_writes; i++) {
                rf.Write(std::string(byte_per_write, (char)(i % 128)));
            }
            auto files = rf.ListFiles();
            FMA_UT_CHECK_EQ(files.size(), (n_writes + write_per_file - 1) / write_per_file + 1);
            FMA_LOG() << ToString(files);
            size_t write = 0;
            for (size_t i = 0; i < files.size(); i++) {
                InputFmaStream in(files[i]);
                FMA_UT_ASSERT(in.Good());
                size_t buf_size = (i == files.size() - 1)
                                      ? (n_writes % write_per_file) * byte_per_write
                                      : write_per_file * byte_per_write;
                FMA_UT_CHECK_EQ(in.Size(), buf_size);
                std::string buf(in.Size(), 0);
                FMA_UT_CHECK_EQ(in.Read(&buf[0], in.Size()), in.Size());
                for (size_t w = 0; w < write_per_file; w++) {
                    FMA_UT_CHECK_EQ(buf.substr(w * byte_per_write, byte_per_write),
                                 std::string(byte_per_write, (char)(write % 128)));
                    write++;
                    if (write >= n_writes) break;
                }
                if (write >= n_writes) break;
            }
        }
        rf.DeleteExistingFiles();
        FMA_UT_CHECK_EQ(rf.ListFiles().size(), 1);
        {
            for (size_t i = 0; i < n_writes; i++) {
                rf.Write(std::string(byte_per_write, (char)(i % 128)));
            }
            auto files = rf.ListFiles();
            FMA_UT_CHECK_EQ(files.size(), (n_writes + write_per_file - 1) / write_per_file + 1);
            FMA_LOG() << ToString(files);
            size_t write = 0;
            for (size_t i = 0; i < files.size(); i++) {
                InputFmaStream in(files[i]);
                FMA_UT_ASSERT(in.Good());
                size_t buf_size = (i == files.size() - 1)
                                      ? (n_writes % write_per_file) * byte_per_write
                                      : write_per_file * byte_per_write;
                FMA_UT_CHECK_EQ(in.Size(), buf_size);
                std::string buf(in.Size(), 0);
                FMA_UT_CHECK_EQ(in.Read(&buf[0], in.Size()), in.Size());
                for (size_t w = 0; w < write_per_file; w++) {
                    FMA_UT_CHECK_EQ(buf.substr(w * byte_per_write, byte_per_write),
                                 std::string(byte_per_write, (char)(write % 128)));
                    write++;
                    if (write >= n_writes) break;
                }
                if (write >= n_writes) break;
            }
        }
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
