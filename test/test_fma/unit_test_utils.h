//  Copyright 2022 AntGroup CO., Ltd.
//  Licensed under the Apache License, Version 2.0 (the "License");
//  you may not use this file except in compliance with the License.
//  You may obtain a copy of the License at
//  http://www.apache.org/licenses/LICENSE-2.0
//  Unless required by applicable law or agreed to in writing, software
//  distributed under the License is distributed on an "AS IS" BASIS,
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.

#pragma once

#include <functional>
#include <map>
#include <mutex>
#include <regex>
#include <string>
#include <vector>

#include "fma-common/logger.h"
#include "tools/lgraph_log.h"

namespace fma_common {
class UtLogger {
 private:
    std::ostringstream ut_stream_;
    lgraph_log::severity_level level_;

 public:
    explicit UtLogger(lgraph_log::severity_level level) : level_(level) {}

    ~UtLogger() {
        FMA_UT_LOG(level_) << ut_stream_.str();
        if (level_ >= lgraph_log::severity_level::ERROR) {
            lgraph_log::LoggerManager::GetInstance().FlushBufferLog();
            EXIT_ON_FATAL(0);
        }
    }

    template <typename T>
    UtLogger & operator<<(const T& d) {
        ut_stream_ << d;
        return *this;
    }
};

#define FMA_UT_ERROR() ::fma_common::UtLogger(::lgraph_log::severity_level::ERROR)

#define FMA_UT_CHECK_EQ(a, b) \
    if (!::fma_common::_detail::CheckEq((a), (b), "CHECK_EQ", #a, #b, __FILE__, __LINE__)) \
    FMA_UT_ERROR()
#define FMA_UT_CHECK_EQ(a, b) \
    if (!::fma_common::_detail::CheckEq((a), (b), "CHECK_EQ", #a, #b, __FILE__, __LINE__)) \
    FMA_UT_ERROR()
#define FMA_UT_CHECK_NEQ(a, b)                                                                  \
    if (!::fma_common::_detail::CheckNeq((a), (b), "CHECK_NEQ", #a, #b, __FILE__, __LINE__)) \
    FMA_UT_ERROR()
#define FMA_UT_CHECK_LT(a, b) \
    if (!::fma_common::_detail::CheckLt((a), (b), "CHECK_LT", #a, #b, __FILE__, __LINE__)) \
    FMA_UT_ERROR()
#define FMA_UT_CHECK_LE(a, b) \
    if (!::fma_common::_detail::CheckLe((a), (b), "CHECK_LE", #a, #b, __FILE__, __LINE__)) \
    FMA_UT_ERROR()
#define FMA_UT_CHECK_GT(a, b) \
    if (!::fma_common::_detail::CheckGt((a), (b), "CHECK_GT", #a, #b, __FILE__, __LINE__)) \
    FMA_UT_ERROR()
#define FMA_UT_CHECK_GE(a, b) \
    if (!::fma_common::_detail::CheckGe((a), (b), "CHECK_GE", #a, #b, __FILE__, __LINE__)) \
    FMA_UT_ERROR()
#define FMA_UT_ASSERT(pred) \
    if (!(pred)) FMA_UT_ERROR() \
    << __FILE__ << ":" << __LINE__ << "\n\tASSERT(" #pred ") failed\n"

struct UnitTest {
    std::string name;
    std::vector<std::string> params;
    size_t num;
    std::function<int(int, char**)> func;
};

inline std::map<std::string, UnitTest>& GetUnitTests() {
    static std::map<std::string, UnitTest> unit_tests;
    return unit_tests;
}

inline void ValidateUnitTestSettings() {
    auto& tests = GetUnitTests();
    for (auto& kv : tests) {
        if (kv.second.func == nullptr) {
            FMA_UT_ERROR() << "Test [" << kv.first
                      << "] was registered but the test function was not defined. "
                      << "Did you misspelled the function?";
        }
    }
}

namespace _detail {
inline size_t RegisterUnitTest(const char* test_name, const std::function<int(int, char**)>& func) {
    auto& m = GetUnitTests();
    auto it = m.find(test_name);
    FMA_UT_ASSERT(it == m.end() || it->second.func == nullptr)
        << "Unit test " << test_name << " already registered";
    if (it == m.end()) {
        it = m.emplace_hint(it, test_name, UnitTest());
    }
    it->second.name = test_name;
    it->second.num = m.size() - 1;
    it->second.func = func;
    return it->second.num;
}

inline size_t RegisterTestParams(const char* test_name) { return 0; }

template <typename... Ts>
inline size_t RegisterTestParams(const char* test_name, const char* first, Ts... params) {
    auto& m = GetUnitTests();
    auto& t = m[test_name];
    t.params.push_back(first);
    RegisterTestParams(test_name, params...);
    return t.params.size() - 1;
}

inline std::string PrintNestedException(const std::exception& e, int level = 0) {
    std::string ret(level * 2, ' ');
    ret.append(e.what());
    try {
        std::rethrow_if_nested(e);
    } catch (const std::exception& e) {
        ret.append("\n" + PrintNestedException(e, level + 1));
    } catch (...) {
    }
    return ret;
}

inline void PrintExpectedException(const std::exception& e) {
    FMA_LOG() << "\n-----------------\nExpected exception: ";
    FMA_LOG() << ::fma_common::_detail::PrintNestedException(e);
}
}  // namespace _detail

#ifdef FMA_IN_UNIT_TEST
#define FMA_UNIT_TEST(name)                                                             \
    int Test##name(int, char**);                                                        \
    size_t _TestNum##name = ::fma_common::_detail::RegisterUnitTest(#name, Test##name); \
    int Test##name(int argc, char** argv)

#define FMA_SET_TEST_PARAMS(name, param1, ...) \
    size_t _TestParams##name =                 \
        ::fma_common::_detail::RegisterTestParams(#name, param1, ##__VA_ARGS__);
#else
#define FMA_UNIT_TEST(name) int main(int argc, char** argv)
#define FMA_SET_TEST_PARAMS(name, param1, ...)
#endif

#define FMA_EXPECT_EXCEPTION(stmt)                            \
    do {                                                      \
        try {                                                 \
            { stmt; }                                         \
            FMA_UT_ASSERT(false);                                \
        } catch (std::exception & e) {                        \
            ::fma_common::_detail::PrintExpectedException(e); \
        }                                                     \
    } while (0)

#define FMA_EXPECT_EXCEPTION_MSG(stmt, msg)                                        \
    do {                                                                           \
        try {                                                                      \
            { stmt; }                                                              \
            FMA_UT_ASSERT(false);                                                     \
        } catch (std::exception & e) {                                             \
            ::fma_common::_detail::PrintExpectedException(e);                      \
            std::string err = ::fma_common::_detail::PrintNestedException(e);      \
            FMA_UT_ASSERT(std::regex_search(err, std::regex(msg)))                    \
                << "Expecting msg=[" << (msg) << "], but got err=[" << err << "]"; \
        }                                                                          \
    } while (0)

#define FMA_EXPECT_EXCEPTION_TYPE(stmt, ExceptionType)                                       \
    do {                                                                                     \
        try {                                                                                \
            { stmt; }                                                                        \
            FMA_UT_ASSERT(false);                                                               \
        } catch (ExceptionType & e) {                                                        \
            ::fma_common::_detail::PrintExpectedException(e);                                \
        } catch (std::exception & e) {                                                       \
            FMA_UT_ASSERT(false) << "Expecting " << #ExceptionType << ", but instead got "      \
                              << typeid(e).name() << "\n\t Exception message: " << e.what(); \
        }                                                                                    \
    } while (0)
}  // namespace fma_common
