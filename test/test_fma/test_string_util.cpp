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

#include <algorithm>
#include <sstream>
#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"
#include "fma-common/string_util.h"
#include "./unit_test_utils.h"
#include "fma-common/utils.h"

inline size_t unsigned_to_decimal(uint64_t number, char *buffer) {
    char *orig = buffer;
    if (number == 0) {
        *buffer++ = '0';
    } else {
        char *p_first = buffer;
        while (number != 0) {
            *buffer++ = '0' + number % 10;
            number /= 10;
        }
        std::reverse(p_first, buffer);
    }
    *buffer = '\0';
    return buffer - orig;
}

void MyPrintInt(std::string &buf, int x) {
    buf.clear();
    // get number of bytes to reserve
    int n, m;
    bool neg = x < 0;
    x = x < 0 ? -x : x;
    if (x < 100000) {
        if (x < 1000) {
            if (x < 100) {
                if (x < 10) {
                    n = 1;
                    m = 1;  // < 10
                } else {
                    n = 2;
                    m = 10;  // 10 <= x < 100
                }
            } else {
                n = 3;
                m = 100;  // 100 <= x < 1000
            }
        } else {
            if (x < 10000) {
                n = 4;
                m = 1000;  // 1000 <= x < 10000
            } else {
                n = 5;
                m = 10000;  // 10000 <= x < 100000
            }
        }
    } else {
        if (x < 10000000) {
            if (x < 1000000) {
                n = 6;
                m = 100000;  // 10^5 <= x < 10^6
            } else {
                n = 7;
                m = 1000000;  // 10^6 <= x < 10^7
            }
        } else {
            if (x < 100000000) {
                n = 8;
                m = 10000000;  // 10^7 <= x < 10^8
            } else {
                if (x < 1000000000) {
                    n = 9;
                    m = 100000000;  // 10^8 <= x < 10^9
                } else {
                    n = 10;
                    m = 1000000000;  // 10^9 <= x
                }
            }
        }
    }
    int pos = 0;
    if (neg) {
        buf.resize(n + 1);
        buf[pos++] = '-';
    } else {
        buf.resize(n);
    }
    while (m > 0) {
        int y = x / m;
        buf[pos++] = '0' + (char)y;
        x -= y * m;
        m /= 10;
    }
}

static constexpr size_t GetNumFields(const char *p, bool skip) {
    return (p == nullptr || *p == '\0') ? 0 : 1;
    // (skip ? GetNumFields(p + 1, false) :
    // (*p == '{' ? 1 + GetNumFields(p + 1, false) :
    //    *p == '\\' ? GetNumFields(p + 1, true) : GetNumFields(p + 1, false)));
}

template <size_t Size, typename... Ts>
static std::string SafeFormat(char const (&format)[Size], const Ts &...data) {
    static_assert(static_cast<size_t>(GetNumFields(format, false)) == 0,
                  "Malformed format string: number of fields does not match");
    std::string ret;
    MyPrintf(ret, format, data...);
    return ret;
}

class Type1 {
 public:
    std::string ToString() const { return "This is an instance of type1"; }
};

class Type2 {
    int y;

 public:
    void ToString() const {}
};

class Type3 {
    int y;

 public:
};

class BaseWithToString {
 public:
    virtual std::string ToString() const = 0;
};

class DerivedWithToString : public BaseWithToString {
 public:
    std::string ToString() const override {
        return "This is a derived class with ToString() member";
    }
};

using namespace fma_common;

FMA_SET_TEST_PARAMS(StringUtil, "--nIter 100000", "--nIter 10");

FMA_UNIT_TEST(StringUtil) {
    lgraph_log::LoggerManager::GetInstance().EnableBufferMode();

    // LOG() << _detail::PrintFloat(-1.000233);
    // LOG() << _detail::PrintFloat(100.000233);
    // LOG() << _detail::PrintFloat(0.000233);
    // LOG() << _detail::PrintFloat(-0.000233);
    // LOG() << _detail::PrintFloat(123456789e-19);
    // LOG() << _detail::PrintFloat(987654321e19);
    // LOG() << _detail::PrintFloat(987654321e64);
    // LOG() << _detail::PrintFloat(1e6);
    // LOG() << _detail::PrintFloat(1e-6);
    // LOG() << ToString(1002334);
    // LOG() << ToString(-1002334);
    //   LOG() << ToString(0.29429);
    // return 0;

    int n_iter = 10000000;

    Configuration config;
    config.Add(n_iter, "nIter", true).Comment("Number of iterations to run");
    config.Parse(argc, argv);
    config.Finalize();

    // double tt1 = GetTime();
    // double d = 0;
    // double pi = 3.14159;
    // double ipart, fpart = 0;
    // double fpart = 0;
    // int e;
    // for (int i = 0; i < n_iter; i++) {
    //    fpart += (i % 10) * 1000000000;
    // }
    // double tt2 = GetTime();
    // LOG() << "fpart=" << fpart;
    // LOG() << (double)n_iter / (1 << 20) / (tt2 - tt1) << " Mtps";
    // return 0;

    double t1, t2;
    double x = 987654321;
    size_t total_bytes = 0;
    LOG_INFO() << "Using snprintf";
    t1 = GetTime();
    for (int i = 0; i < n_iter; i++) {
        char buf[32];
        total_bytes += snprintf(buf, sizeof(buf), "%f", x);
    }
    t2 = GetTime();
    LOG_INFO() << "Printed " << n_iter << " integers at " << (double)n_iter / (t2 - t1) << " tps";
    LOG_INFO() << "Number of bytes: " << total_bytes << " at "
          << (double)total_bytes / 1024 / 1024 / (t2 - t1) << "MB/s";

    SleepS(1);

    total_bytes = 0;
    LOG_INFO() << "Using to_string";
    std::string buf(16, 0);
    t1 = GetTime();
    std::ios_base::sync_with_stdio(0);
    for (int i = 0; i < n_iter; i++) {
        // MyPrintInt(buf, x);
        // buf = std::to_string(x);
        // total_bytes += buf.size();
        total_bytes += ToString(x).size();
        // total_bytes += std::to_string(x).size();
        // total_bytes += StringFormatter::Format(buf, "{}", x).size();
        // total_bytes += unsigned_to_decimal(x, &buf[0]);
    }
    t2 = GetTime();
    LOG_INFO() << "x=" << ToString(x);
    LOG_INFO() << "Printed " << n_iter << " integers at " << (double)n_iter / (t2 - t1) << " tps";
    LOG_INFO() << "Number of bytes: " << total_bytes << " at "
          << (double)total_bytes / 1024 / 1024 / (t2 - t1) << "MB/s";

    //// this should trigger a ToString()-not-defined error
    // LOG() << ToString(Type3());
    //// this should trigger a ToString()-return-type error
    // LOG() << ToString(Type2());
    LOG_INFO() << ToString(DerivedWithToString());
    LOG_INFO() << ToString(Type1());

    lgraph_log::LoggerManager::GetInstance().DisableBufferMode();
    return 0;
}
