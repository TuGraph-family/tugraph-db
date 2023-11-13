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

#include "fma-common/fma_stream.h"
#include "../test/ut_utils.h"

class MemoryBufferDevice : public fma_common::LogDevice {
    mutable fma_common::OutputMemoryFileStream buf_;

 public:
    void WriteLine(const char* p, size_t s, fma_common::LogLevel level) override {
        buf_.Write(p, s);
    }

    void Reset() {
        buf_.Close();
        buf_.Open("", 0, std::ofstream::app);
    }

    void Print() const {
        std::shared_lock<std::shared_mutex> lock(buf_.GetMutex());
        std::cerr << buf_.GetBuf();
    }
};

void TuGraphTest::setup() {
    if (_ut_buffer_log) {
        static std::shared_ptr<MemoryBufferDevice> device(new MemoryBufferDevice());
        device->Reset();
        fma_common::Logger::Get().SetDevice(device);
        lgraph_log::LoggerManager::GetInstance().EnableBufferMode();
    }
}

void TuGraphTest::teardown() {
    if (_ut_buffer_log) {
        const ::testing::TestInfo* const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        if (test_info->result()->Failed()) {
            dynamic_cast<MemoryBufferDevice*>(fma_common::Logger::Get().GetDevice().get())->Print();
            lgraph_log::LoggerManager::GetInstance().FlushBufferLog();
        }
        lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    }
}

void TuGraphTest::SetUp() { setup(); }

void TuGraphTest::TearDown() { teardown(); }
