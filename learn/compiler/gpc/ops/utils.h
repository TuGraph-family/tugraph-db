/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#ifndef GPC_OPS_UTILS
#define GPC_OPS_UTILS

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

inline std::string vector2string(std::vector<int> vec) {
    std::stringstream ss;
    ss << "{";
    int option_size = vec.size();
    for (int i = 0; i < option_size - 1; ++i)
        ss << std::to_string(vec[i]) << ",";
    if (option_size != 0)
        ss << std::to_string(vec[option_size - 1]);
    ss << "}";
    return ss.str();
}

template <bool HighResIsSteady = std::chrono::high_resolution_clock::is_steady>
struct SteadyClock {
    using type = std::chrono::high_resolution_clock;
};
// ...otherwise use steady_clock.
template <> struct SteadyClock<false> {
    using type = std::chrono::steady_clock;
};
using BenchmarkClock = SteadyClock<>::type;

class LogTimeScope {
  public:
    LogTimeScope(const char *tag, bool print = false) {
        print_ = print;
        itag = tag;
        start = BenchmarkClock::now();
    }
    ~LogTimeScope() {
        if (!print_)
            return;
        double ms = GetTimeMS();
        std::cout << itag << "'s run time is " << ms << "ms\n";
    }
    double GetTimeMS() {
        auto end = BenchmarkClock::now();
        double elapsed_ms =
            std::chrono::duration_cast<std::chrono::duration<double>>(end -
                                                                      start)
                .count() *
            1000;
        return elapsed_ms;
    }
    const char *itag;
    std::chrono::time_point<std::chrono::steady_clock> start;
    bool print_;
};

#endif // GPC_OPS_UTILS