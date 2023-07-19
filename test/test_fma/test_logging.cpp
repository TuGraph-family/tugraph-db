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
#include "fma-common/unit_test_utils.h"

void Bar(int y) {
    int *p = nullptr;
    *p = 1024;
}

void Foo(int x) { Bar(x + 10); }

class MyFormatter : public fma_common::LogFormatter {
    std::string header_;

 public:
    explicit MyFormatter(const std::string &header) : header_(header) {}

    void WriteLine(const char *p, size_t s, fma_common::LogDevice &device, const std::string &,
                   fma_common::LogLevel) override {
        char buf[1024];
        memcpy(buf, header_.data(), header_.size());
        memcpy(buf + header_.size(), p, s);
        device.WriteLine(buf, s + header_.size(), fma_common::LogLevel::LL_INFO);
    }
};

FMA_UNIT_TEST(Logging) {
    LOG() << "This is log without any header";

    FMA_CHECK_NEQ(0, 1);

    using namespace fma_common;
    Logger &root = Logger::Get("");
    root.SetFormatter(std::make_shared<TimedLogFormatter>());
    LOG() << "This is log with timed header";
    Logger &child = Logger::Get("child");
    LoggerStream(child, LL_INFO) << "Child inherits parent logger settings";
    child.SetFormatter(std::make_shared<LogFormatter>());
    LoggerStream(child, LL_INFO) << "Now child is back to unformatted.";
    LOG() << "But root logger still has header";
    Logger &grandson = Logger::Get("child.child");
    {
        LoggerStream glog(grandson, LL_DEBUG);
        glog << "This is invisible since the current log level is less than logger's log level.\n";
        glog.Flush();
    }
    grandson.SetLevel(LL_DEBUG);
    LoggerStream(grandson, LL_DEBUG) << "Now this is visible, and has no header\n";
    grandson.SetFormatter(std::make_shared<MyFormatter>("[Grandson]: "));
    LoggerStream(grandson, LL_DEBUG) << "Now we have customed header";
    // this should trigger the assert failure and print stacktrace
    // CHECK_EQ(1, 2) << "Impossible!";

    // this should trigger seg fault and print stacktrace
    /*LOG() << "Testing seg fault handler";
    Foo(10);*/

    FMA_LOG() << "INFO message";
    FMA_DBG_CHECK(true) << "DBG_CHECK";
    root.SetLevel(LL_ERROR);
    FMA_LOG() << "This should not appear";

    /*DBG_CHECK should trigger error in debug mode, and should compile to no-op in release mode*/
    // FMA_DBG_CHECK(false) << "This should trigger error.";

    // test thread safety of Logger::Get
    std::vector<std::thread> threads;
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([]() {
            for (int i = 0; i < 1000; ++i) {
                Logger::Get(std::to_string(i));
            }
        });
    }
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
