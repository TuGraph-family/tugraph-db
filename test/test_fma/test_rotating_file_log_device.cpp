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

// fma_log is no longer used after the implementation of lgraph_log
// this test is no longer useful since it's for fma_log's file rotation
// lgraph_log's file rotation test is in test_lgraph_log.cpp

#include "fma-common/configuration.h"
#include "fma-common/fma_stream.h"
#include "fma-common/file_system.h"
#include "fma-common/rotating_file_log_device.h"
#include "./unit_test_utils.h"

FMA_SET_TEST_PARAMS(RotatingFileLogDevice, "");

FMA_UNIT_TEST(RotatingFileLogDevice) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    using namespace fma_common;
    const std::string dir = "./tmp";
    const std::string prefix = "log_file";
    auto check_files = [&](int n, int last_idx) {
        auto files = RotatingFileLogDevice::ListLogFiles(dir, prefix);
        if (n != -1) {
            FMA_UT_CHECK_EQ(files.size(), (size_t)n);
        }
        FMA_UT_CHECK_EQ(files.back(),
                     (_STD_FS::path(dir) / (FMA_FMT("log_file.{}", last_idx))).string());
    };
    fma_common::file_system::RemoveDir(dir);
    fma_common::file_system::MkDir(dir);
    {
        FMA_LOG() << "Testing rotate";
        {
            RotatingFileLogDevice device(dir, prefix, 1, 3);
            for (size_t i = 0; i < 4; i++) {
                const std::string s = std::to_string(i);
                device.WriteLine(s.data(), s.size(), LogLevel::LL_INFO);
            }
        }
        check_files(3, 4);

        FMA_LOG() << "Testing reopen";
        {
            // reopen should not change file idx
            RotatingFileLogDevice device(dir, "log_file", 1, 3);
            check_files(3, 4);

            // write again and get a new log file
            device.WriteLine(dir.data(), dir.size(), LogLevel::LL_INFO);
            check_files(3, 5);
        }

        FMA_LOG() << "Testing bigger files";
        {
            RotatingFileLogDevice device(dir, "log_file", 1024, 3);
            const std::string s(10, 'T');
            for (size_t i = 0; i < 102; i++)
                device.WriteLine(s.data(), s.size(), LogLevel::LL_INFO);
            check_files(3, 5);

            // this should trigger a rotate
            device.WriteLine(s.data(), s.size(), LogLevel::LL_INFO);
            check_files(3, 6);
        }
    }
    {
        FMA_LOG() << "Testing with fma_common::Logger";
        {
            // now test with logger
            auto &logger = fma_common::Logger::Get("my_logger");
            logger.SetDevice(std::make_shared<RotatingFileLogDevice>(dir, "log_file", 102, 0));
            for (size_t i = 0; i < 10; i++) {
                FMA_INFO_STREAM(logger) << "012345678";  // actually writes 10 bytes, including \n
            }
            check_files(1, 6);

            // now trigger a rotate
            FMA_INFO_STREAM(logger) << "012345678";  // actually writes 10 bytes
            check_files(1, 7);
            FMA_INFO_STREAM(logger) << "012345678";  // actually writes 10 bytes
        }
        // now check file content
        fma_common::InputFmaStream in((_STD_FS::path(dir) / ("log_file.7")).string());
        fma_common::StreamLineReader reader(in);
        std::vector<std::string> content = reader.ReadAllLines(true);
        FMA_UT_CHECK_EQ(content.size(), 1);
        FMA_UT_CHECK_EQ(content.back(), "012345678");
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
