/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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

    void Print() const { std::cerr << buf_.GetBuf(); }
};

void TuGraphTest::setup() {
    if (_ut_buffer_log) {
        static std::shared_ptr<MemoryBufferDevice> device(new MemoryBufferDevice());
        device->Reset();
        fma_common::Logger::Get().SetDevice(device);
    }
}

void TuGraphTest::teardown() {
    if (_ut_buffer_log) {
        const ::testing::TestInfo* const test_info =
            ::testing::UnitTest::GetInstance()->current_test_info();
        if (test_info->result()->Failed()) {
            dynamic_cast<MemoryBufferDevice*>(fma_common::Logger::Get().GetDevice().get())->Print();
        }
    }
}

void TuGraphTest::SetUp() { setup(); }

void TuGraphTest::TearDown() { teardown(); }
