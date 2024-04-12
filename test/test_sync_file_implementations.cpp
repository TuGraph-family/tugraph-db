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

#ifndef _WIN32

#include <fcntl.h>

#include <fstream>

#include "gtest/gtest.h"
#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "./ut_utils.h"

class TestSyncFileImpl : public TuGraphTest {
 protected:
    std::string path_ = "./test_file";
    size_t n_tests_ = 3;
    size_t n_commits_ = 100;
    size_t n_writes_ = 10;
    size_t write_size_ = 256;
    double start_time_ = 0;
    double end_time_ = 0;
    double total_time = 0;
    std::string write_buf_;

    TestSyncFileImpl() {
        fma_common::Configuration conf;
        conf.Add(path_, "path", true)
            .Comment("Path of the test file.");
        conf.Add(n_tests_, "niter", true)
            .Comment("Number of tests to run.");
        conf.Add(n_commits_, "ncommits", true)
            .Comment("Number of commits in each test.");
        conf.Add(n_writes_, "nwrites", true)
            .Comment("Number of writes to perform in each commit.");
        conf.Add(write_size_, "wsize", true)
            .Comment("Number of bytes to write in each write.");
        conf.ParseAndFinalize(_ut_argc, _ut_argv);
        write_buf_.resize(write_size_, 'a');
    }

    void SetUp() {
        TuGraphTest::SetUp();
        if (!_ut_run_benchmarks)
            GTEST_SKIP() << "--run_benchmarks not set skipping benchmarks.";
        total_time = 0;
    }

    void TearDown() {
        size_t n_bytes = n_tests_ * n_commits_ * n_writes_ * write_size_;
        UT_LOG() << "Average throughput: "
                 << (double)n_bytes / 1024 / 1024 / total_time
                 << " MB/s";
        TuGraphTest::TearDown();
    }

    void StartOneTest() {
        fma_common::file_system::RemoveFile(path_);
        start_time_ = fma_common::GetTime();
    }

    void EndOneTest() {
        end_time_ = fma_common::GetTime();
        total_time += (end_time_ - start_time_);
        size_t n_bytes = n_commits_ * n_writes_ * write_size_;
        UT_LOG() << "Finished writing at "
                 << (double) n_bytes/ 1024/1024/(end_time_ - start_time_)
                 << " MB/s";
        fma_common::file_system::RemoveFile(path_);
    }
};

TEST_F(TestSyncFileImpl, OSync) {
    UT_LOG() << "Testing with O_SYNC";
    std::string file_buf(1<<20, 0);
    size_t write_p = 0;
    for (size_t i = 0; i < n_tests_; i++) {
        double t1 = fma_common::GetTime();
        int file = open(path_.c_str(), O_SYNC | O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        UT_EXPECT_NE(file, -1);
        double t2 = fma_common::GetTime();
        UT_LOG() << "Open time: " << t2 - t1;
        StartOneTest();
        size_t written_bytes = 0;
        for (size_t c = 0; c < n_commits_; c++) {
            write_p = 0;
            for (size_t w = 0; w < n_writes_; w++) {
                if (write_p + write_buf_.size() > file_buf.size()) {
                    written_bytes += write(file, file_buf.data(), write_p);
                    write_p = 0;
                }
                if (write_size_ > file_buf.size()) {
                    // too large, write directly
                    written_bytes += write(file, write_buf_.data(), write_buf_.size());
                } else {
                    memcpy(&write_buf_[0] + write_p, write_buf_.data(), write_buf_.size());
                    write_p += write_size_;
                }
            }
            // commit
            written_bytes += write(file, file_buf.data(), write_p);
        }
        EndOneTest();
        t1 = fma_common::GetTime();
        close(file);
        t2 = fma_common::GetTime();
        UT_LOG() << "Close time: " << t2 - t1;
        fma_common::file_system::RemoveFile(path_);
        UT_EXPECT_EQ(written_bytes, n_commits_*n_writes_*write_size_);
    }
}

TEST_F(TestSyncFileImpl, FSync) {
    UT_LOG() << "Testing with fsync";
    for (size_t i = 0; i < n_tests_; i++) {
        double t1 = fma_common::GetTime();
        int file = open(path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        UT_EXPECT_NE(file, -1);
        double t2 = fma_common::GetTime();
        UT_LOG() << "Open time: " << t2 - t1;
        StartOneTest();
        size_t written_bytes = 0;
        for (size_t c = 0; c < n_commits_; c++) {
            for (size_t w = 0; w < n_writes_; w++) {
                written_bytes += write(file, write_buf_.data(), write_buf_.size());
            }
            // commit
            fsync(file);
        }
        EndOneTest();
        t1 = fma_common::GetTime();
        close(file);
        t2 = fma_common::GetTime();
        UT_LOG() << "Close time: " << t2 - t1;
        fma_common::file_system::RemoveFile(path_);
        UT_EXPECT_EQ(written_bytes, n_commits_*n_writes_*write_size_);
    }
}

TEST_F(TestSyncFileImpl, FDataSync) {
    UT_LOG() << "Testing with fsync";
    for (size_t i = 0; i < n_tests_; i++) {
        double t1 = fma_common::GetTime();
        int file = open(path_.c_str(), O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
        UT_EXPECT_NE(file, -1);
        double t2 = fma_common::GetTime();
        UT_LOG() << "Open time: " << t2 - t1;
        StartOneTest();
        size_t written_bytes = 0;
        for (size_t c = 0; c < n_commits_; c++) {
            for (size_t w = 0; w < n_writes_; w++) {
                written_bytes += write(file, write_buf_.data(), write_buf_.size());
            }
            // commit
            fdatasync(file);
        }
        EndOneTest();
        t1 = fma_common::GetTime();
        close(file);
        t2 = fma_common::GetTime();
        UT_LOG() << "Close time: " << t2 - t1;
        fma_common::file_system::RemoveFile(path_);
        UT_EXPECT_EQ(written_bytes, n_commits_*n_writes_*write_size_);
    }
}

TEST_F(TestSyncFileImpl, NoSync) {
    UT_LOG() << "Testing with fstream";
    for (size_t i = 0; i < n_tests_; i++) {
        double t1 = fma_common::GetTime();
        fma_common::file_system::RemoveFile(path_);
        std::ofstream file(path_, std::ios_base::binary | std::ios_base::trunc);
        double t2 = fma_common::GetTime();
        UT_LOG() << "Open time: " << t2 - t1;
        StartOneTest();
        for (size_t c = 0; c < n_commits_; c++) {
            for (size_t w = 0; w < n_writes_; w++) {
                file.write(write_buf_.data(), write_buf_.size());
            }
            // commit
            file.flush();
        }
        EndOneTest();
        t1 = fma_common::GetTime();
        file.close();
        t2 = fma_common::GetTime();
        UT_LOG() << "Close time: " << t2 - t1;
        fma_common::file_system::RemoveFile(path_);
    }
}
#endif
