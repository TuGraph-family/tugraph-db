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
    std::string log_general = "general.log";
    std::string log_debug = "debug.log";
    std::string history_log_dir = log_dir + "history_logs/";
    std::string history_log_general_dir = history_log_dir + "general_logs/";
    std::string history_log_debug_dir = history_log_dir + "debug_logs/";
    std::string class_name = "Galaxy";
    std::string general_log_info_msg = "This is a info level general log message.";
    std::string general_log_stream_debug_msg =
        "This is a debug level general log message with class name Galaxy.";
    std::string debug_log_error_msg = "This is a error level debug log message.";
    std::string debug_log_stream_fatal_msg =
        "This is a fatal level debug log message with class name Galaxy.";
    int log_records_to_write = 70000;
    size_t file_size = 5 * 1024 * 1024;
    int file_size_error = 200;

    lgraph::AutoCleanDir cleaner(log_dir);
    // Init LoggerManager, set log directory, default log level is info.
    lgraph_log::LoggerManager::GetInstance().Init(log_dir, lgraph_log::severity_level::INFO,
                                                  file_size);

    // Test general log
    {
        // Write general log message to file.
        // This will write a log in the format of
        // "[TimeStamp] [SeverityLevel] - Message".
        GENERAL_LOG(INFO) << general_log_info_msg;
        // This will write a log in the format of
        // "[TimeStamp] [SeverityLevel] [ClassName] - Message".
        GENERAL_LOG_STREAM(DEBUG, class_name) << general_log_stream_debug_msg;

        // Check general log file's existence.
        std::string line;
        std::vector<std::string> lines;
        std::ifstream general_log_file(log_dir + log_general);
        UT_EXPECT_TRUE(general_log_file.is_open());
        while (getline(general_log_file, line)) {
            lines.push_back(line);
        }

        // Check log message's correctness in the suffix of each log line.
        UT_EXPECT_EQ(lines.size(), 2);
        int len = general_log_info_msg.size();
        std::string log_msg = lines[0].substr(lines[0].size() - len, len);
        UT_EXPECT_EQ(general_log_info_msg, log_msg);
        len = general_log_stream_debug_msg.size();
        log_msg = lines[1].substr(lines[1].size() - len, len);
        UT_EXPECT_EQ(general_log_stream_debug_msg, log_msg);

        // Close log file.
        general_log_file.close();
    }

    // Test debug log
    {
        // Set log level, filter all log message below current level,
        // this level does not affect general log.
        // under debug level, log line will show info of
        // [file : function : line] to help locate the source of log message.
        // above debug level, log line will be simpler.
        lgraph_log::LoggerManager::GetInstance().SetLevel(lgraph_log::severity_level::DEBUG);

        // Write debug log messages to file.
        // Under debug level, this will write a log in the format of
        // "[TimeStamp] [ThreadID] [SeverityLevel] [File : Function : Line] - Message".
        DEBUG_LOG(ERROR) << debug_log_error_msg;
        // Under debug level, this will write a log in the format of
        // "[TimeStamp] [ThreadID] [SeverityLevel] [ClassName] [File : Function : Line] - Message".
        DEBUG_LOG_STREAM(FATAL, class_name) << debug_log_stream_fatal_msg;

        // Check debug log file's existence.
        std::string line;
        std::vector<std::string> lines;
        std::ifstream debug_log_file(log_dir + log_debug);
        UT_EXPECT_TRUE(debug_log_file.is_open());
        while (getline(debug_log_file, line)) {
            lines.push_back(line);
        }

        // Check log message's correctness in the suffix of each log line.
        UT_EXPECT_EQ(lines.size(), 2);
        int len = debug_log_error_msg.size();
        std::string log_msg = lines[0].substr(lines[0].size() - len, len);
        UT_EXPECT_EQ(debug_log_error_msg, log_msg);
        len = debug_log_stream_fatal_msg.size();
        log_msg = lines[1].substr(lines[1].size() - len, len);
        UT_EXPECT_EQ(debug_log_stream_fatal_msg, log_msg);

        // Close log file
        debug_log_file.close();
    }

    // Test file rotation
    {
        // Write a large number of log messages to trigger file rotation.
        for (int i = 0; i < log_records_to_write; i++) {
            GENERAL_LOG(INFO) << general_log_info_msg;
            DEBUG_LOG(ERROR) << debug_log_error_msg;
        }

        // Check rotated file's existence and size in history_logs folder.
        std::ifstream history_general_log_file(history_log_general_dir + log_general);
        UT_EXPECT_TRUE(history_general_log_file.is_open());
        UT_EXPECT_LE(std::filesystem::file_size(history_log_general_dir + log_general),
            file_size + file_size_error);
        UT_EXPECT_GE(std::filesystem::file_size(history_log_general_dir + log_general),
            file_size - file_size_error);
        std::ifstream history_debug_log_file(history_log_debug_dir + log_debug);
        UT_EXPECT_TRUE(history_debug_log_file.is_open());
        UT_EXPECT_LE(std::filesystem::file_size(history_log_debug_dir + log_debug),
            file_size + file_size_error);
        UT_EXPECT_GE(std::filesystem::file_size(history_log_debug_dir + log_debug),
            file_size - file_size_error);
    }

    lgraph_log::LoggerManager::GetInstance().Init("", lgraph_log::severity_level::INFO);
}
