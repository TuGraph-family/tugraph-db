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

#include "fma-common/logger.h"
#include "./unit_test_utils.h"
#include "fma-common/logging.h"

FMA_SET_TEST_PARAMS(LevelLog, "");

FMA_UNIT_TEST(LevelLog) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    // test err info in different log level

    using namespace fma_common;
    fma_common::Logger::Get().SetFormatter(std::make_shared<TimedLogFormatter>());
    fma_common::Logger::Get().SetLevel(LogLevel::LL_ERROR);
    FMA_LOG() << "This is Error level" << "log format";
    ::fma_common::LoggerStream(::fma_common::Logger::Get(), LogLevel::LL_ERROR)
    << "This is Error level" << "log format";
    fma_common::Logger::Get().SetLevel(LogLevel::LL_INFO);
    FMA_LOG() << "This is Info level" << "log format";
    ::fma_common::LoggerStream(::fma_common::Logger::Get(), LogLevel::LL_ERROR)
    << "This is Info level" << "output format";
    fma_common::Logger::Get().SetLevel(LogLevel::LL_DEBUG);
    FMA_LOG() << "This is Debug level" << "log format";
    ::fma_common::LoggerStream(::fma_common::Logger::Get(), LogLevel::LL_ERROR)
    << "This is Debug level" << "log format";

    std::ostringstream header;
    header << "\n"
    << "**********************************************************************" << "\n"
        << "*                  TuGraph Graph Database v"
        << "*" << "\n"
        << "*                                                                    *" << "\n"
        << "*    Copyright(C) 2018-2021 Ant Group. All rights reserved.          *" << "\n"
        << "*                                                                    *" << "\n"
        << "**********************************************************************" << "\n"
        << "Server is configured with the following parameters:\n";
    FMA_LOG() << header.str();

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
