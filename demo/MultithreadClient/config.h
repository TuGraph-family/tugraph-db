#pragma once
#include <string>
#include <atomic>

namespace multithread_client {

struct Config {
    std::string host;
    std::string user;
    std::string password;
    std::string mode;
    std::string input;
    std::string output;
    bool continue_on_error;
    uint32_t thread_num;
};

struct PerformanceIndicator {
    PerformanceIndicator() : total_query(0), success_query(0), time_used(0) {}
    uint32_t total_query;
    uint32_t success_query;
    uint64_t time_used;
    std::string result;
};

}  // end of namespace multithread_client
