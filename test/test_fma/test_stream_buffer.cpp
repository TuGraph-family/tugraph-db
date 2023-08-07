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
#include "fma-common/logging.h"
#include "fma-common/stream_buffer.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

using namespace fma_common;

class MockInputStream : public InputStreamBase {
    size_t size_ = 0;
    size_t offset_ = 0;
    double mbps_ = 1.0;
    double read_time_ = 0.0;

 public:
    ~MockInputStream() { LOG() << "Time used in Stream::Read(): " << read_time_; }

    MockInputStream(size_t size, double mbps) {
        size_ = size;
        mbps_ = mbps;
    }

    size_t Read(void *buf, size_t size) override {
        double t1 = GetTime();
        size_t s = std::min(size_ - offset_, size);
        if (s != 0) {
            SleepS((double)s / 1024 / 1024 / mbps_);
            memset(buf, 1, s);
            offset_ += s;
        }
        read_time_ += GetTime() - t1;
        return s;
    }

    virtual size_t Offset() const { return 0; }

    bool Good() const override { return true; }
};

class MockOutputStream : public OutputStreamBase {
    size_t size_ = 0;
    double mbps_ = 1.0;
    double read_time_ = 0.0;

 public:
    ~MockOutputStream() { LOG() << "Time used in Stream::Write(): " << read_time_; }

    explicit MockOutputStream(double mbps) { mbps_ = mbps; }

    void Write(const void *buf, size_t size) override {
        double t1 = GetTime();
        size_ += size;
        if (size != 0) {
            SleepS((double)size / 1024 / 1024 / mbps_);
        }
        read_time_ += GetTime() - t1;
    }

    bool Good() const override { return true; }
};

FMA_SET_TEST_PARAMS(StreamBuffer, "--mode read", "--mode write");

FMA_UNIT_TEST(StreamBuffer) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    size_t size = 4 << 20;
    size_t buf_size = 4 << 20;
    double mbps = 10.0;
    size_t block_size = 1 << 20;
    double process_mbps = 10.0;
    std::string mode;
    Configuration config;
    config.Add(size, "size", true).Comment("total size of mock file");
    config.Add(buf_size, "bufSize", true).Comment("size of the read buffer");
    config.Add(mbps, "ioSpeed", true).Comment("read/write speed in MB/s");
    config.Add(block_size, "blockSize", true).Comment("read block size");
    config.Add(process_mbps, "processSpeed", true).Comment("process speed in MB/s");
    config.Add(mode, "mode", false).Comment("read or write").SetPossibleValues({"read", "write"});
    config.Parse(argc, argv);
    config.Finalize();

    std::string buf(block_size, 0);
    double tt1, tt2;
    if (mode == "read") {
        LOG() << "Testing InputStreamBuffer";
        LOG() << "Directly using stream";
        tt1 = GetTime();
        {
            MockInputStream in(size, mbps);
            size_t total = 0;
            while (true) {
                size_t r = in.Read(&buf[0], block_size);
                if (r == 0) break;
                total += r;
                // mimic process
                SleepS((double)r / 1024 / 1024 / process_mbps);
            }
            FMA_UT_CHECK_EQ(total, size);
        }
        tt2 = GetTime();
        LOG() << "Finished at " << (double)size / 1024 / 1024 / (tt2 - tt1) << "MB/s";
        LOG() << "Now using buffered stream";
        tt1 = GetTime();
        {
            MockInputStream in(size, mbps);
            PrefetchingtStreamBuffer stream_buffer;
            stream_buffer.Open(&in, buf_size);
            size_t total = 0;
            while (true) {
                size_t r = stream_buffer.Read(&buf[0], block_size);
                if (r == 0) break;
                total += r;
                // mimic process
                SleepS((double)r / 1024 / 1024 / process_mbps);
            }
            FMA_UT_CHECK_EQ(total, size);
        }
        tt2 = GetTime();
        LOG() << "Finished at " << (double)size / 1024 / 1024 / (tt2 - tt1) << "MB/s";
    } else {
        LOG() << "Testing OutputStreamBuffer";
        LOG() << "Directly using stream";
        tt1 = GetTime();
        {
            MockOutputStream out(mbps);
            size_t total = 0;
            while (total < size) {
                out.Write(&buf[0], block_size);
                total += block_size;
                // mimic process
                SleepS((double)block_size / 1024 / 1024 / process_mbps);
            }
        }
        tt2 = GetTime();
        LOG() << "Finished at " << (double)size / 1024 / 1024 / (tt2 - tt1) << "MB/s";
        LOG() << "Now using buffered stream";
        tt1 = GetTime();
        {
            MockOutputStream stream(mbps);
            ThreadedOutputStreamBuffer buffer;
            buffer.Open(&stream, buf_size);
            size_t total = 0;
            while (total < size) {
                buffer.Write(&buf[0], block_size);
                total += block_size;
                // mimic process
                SleepS((double)block_size / 1024 / 1024 / process_mbps);
            }
        }
        tt2 = GetTime();
        LOG() << "Finished at " << (double)size / 1024 / 1024 / (tt2 - tt1) << "MB/s";
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
