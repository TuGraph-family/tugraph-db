#pragma once

#include "config.h"
#include "client_thread_pool.h"

namespace multithread_client {

class PluginSender {
 public:
    PluginSender(const Config& conf);

    void process();

 private:
    void file_reader();
    bool parse_line(std::string& line, uint32_t& times);
    void calculate(uint64_t time_used);

    const Config& config;
    ClientThreadPool pool;
};

}  // end of namespace multithread_client