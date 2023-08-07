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
#include "fma-common/file_system.h"
#include "fma-common/fma_stream.h"
#include "fma-common/leveled_log_device.h"
#include "fma-common/logger.h"
#include "./unit_test_utils.h"

FMA_UNIT_TEST(LeveledLogDevice) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    using namespace fma_common;
    const std::string info_path = "./_info_log_file";
    const std::string warn_path = "./_warn_log_file";
    file_system::RemoveFile(info_path);
    file_system::RemoveFile(warn_path);
    std::string fatal = "fatal ";
    std::string error = "error ";
    std::string warn = "warn ";
    std::string info = "info ";
    std::string debug = "debug";
    {
        auto info_log = std::make_shared<FileLogDevice>(info_path);
        auto warn_log = std::make_shared<FileLogDevice>(warn_path);
        std::map<LogLevel, std::shared_ptr<LogDevice>> devices = {{LogLevel::LL_WARNING, warn_log},
                                                                  {LogLevel::LL_INFO, info_log}};
        LeveledLogDevice level_device(devices);
        level_device.WriteLine(fatal.data(), fatal.size(), LogLevel::LL_FATAL);
        level_device.WriteLine(error.data(), error.size(), LogLevel::LL_ERROR);
        level_device.WriteLine(warn.data(), warn.size(), LogLevel::LL_WARNING);
        level_device.WriteLine(info.data(), info.size(), LogLevel::LL_INFO);
        level_device.WriteLine(debug.data(), debug.size(), LogLevel::LL_DEBUG);
        level_device.Flush();
    }
    {
        auto ReadAllLinesFromFile = [](const std::string &path) {
            InputFmaStream ins(path);
            StreamLineReader reader(ins);
            return reader.ReadAllLines(true);
        };

        std::vector<std::string> l1 = ReadAllLinesFromFile(info_path);
        FMA_UT_CHECK_EQ(l1.size(), 1);
        FMA_UT_CHECK_EQ(l1[0], info + debug);

        std::vector<std::string> l2 = ReadAllLinesFromFile(warn_path);
        FMA_UT_CHECK_EQ(l2.size(), 1);
        FMA_UT_CHECK_EQ(l2[0], fatal + error + warn);
    }

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
