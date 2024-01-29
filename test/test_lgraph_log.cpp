/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include <iostream>
#include <fstream>
#include <filesystem>

#include "./ut_utils.h"
#include "./test_tools.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"
#include "tools/lgraph_log.h"

class TestLGraphLog : public TuGraphTest {};

TEST_F(TestLGraphLog, LGraphLog) {
    using namespace lgraph_api;

    // Set log directory and log message.
    std::string log_dir = "logs/";
    std::string info_msg = "This is a info level log message.";
    std::string error_msg = "This is a error level log message.";
    std::string warn_msg = "This is a warn level log message.";
    int log_records_to_write = 70000;
    size_t file_size = 5 * 1024 * 1024;

    lgraph::AutoCleanDir cleaner(log_dir);
    lgraph_log::LoggerManager::GetInstance().Init(
        log_dir, lgraph_log::severity_level::INFO, file_size);
    {
        lgraph_log::LoggerManager::GetInstance().SetLevel(lgraph_log::severity_level::DEBUG);
        LOG_ERROR() << error_msg;
        LOG_WARN() << warn_msg;

        std::string line;
        std::vector<std::string> lines;

        std::vector<std::string> log_files;
        for (const auto & entry : std::filesystem::directory_iterator(log_dir)) {
            log_files.push_back(entry.path().generic_string());
        }
        UT_EXPECT_EQ(log_files.size(), 1);
        std::ifstream log_file(log_files[0]);
        UT_EXPECT_TRUE(log_file.is_open());
        while (getline(log_file, line)) {
            lines.push_back(line);
        }

        // Check log message's correctness in the suffix of each log line.
        UT_EXPECT_EQ(lines.size(), 2);
        int len = error_msg.size();
        std::string log_msg = lines[0].substr(lines[0].size() - len, len);
        UT_EXPECT_EQ(error_msg, log_msg);
        len = warn_msg.size();
        log_msg = lines[1].substr(lines[1].size() - len, len);
        UT_EXPECT_EQ(warn_msg, log_msg);

        // Close log file
        log_file.close();
    }

    // Test file rotation
    {
        // Write a large number of log messages to trigger file rotation.
        for (int i = 0; i < log_records_to_write; i++) {
            LOG_INFO() << info_msg;
            LOG_ERROR() << error_msg;
        }
        std::vector<std::string> log_files;
        for (const auto & entry : std::filesystem::directory_iterator(log_dir)) {
            log_files.push_back(entry.path().generic_string());
        }
        UT_EXPECT_EQ(log_files.size(), 4);
    }

    lgraph_log::LoggerManager::GetInstance().Init("", lgraph_log::severity_level::INFO);
}
