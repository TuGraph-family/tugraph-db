#pragma once
#include <algorithm>
#include <cctype>
#include <thread>
#include "config.h"

namespace lgraph {
class RpcClient;
}  // namespace lgraph

namespace multithread_client {

class Client {
 public:
    void call_cypher();

    void load_plugin();

    void call_plugin();

    void delete_plugin();

    void process();

    std::string get_host() { return config.host; }

    void set_host(const std::string &host) { config.host = host; }

    std::string get_user() { return config.user; }

    void set_user(const std::string &user) { config.user = user; }

    std::string get_password() { return config.password; }

    void set_password(const std::string &password) { config.password = password; }

    std::string get_mode() { return config.mode; }

    void set_mode(const std::string &mode) { config.mode = mode; }

    int32_t get_thread_num() { return config.thread_num; }

    void set_thread_num(const std::string &thread_num) {
        for (char c : thread_num) {
            if (!std::isdigit(c)) {
                config.thread_num = 1;
                return;
            }
        }
        int count = std::stoi(thread_num);
        int hard_threads =
            std::thread::hardware_concurrency() == 0 ? count : std::thread::hardware_concurrency();
        config.thread_num = count <= hard_threads ? count : hard_threads;
    }

    std::string get_input() { return config.input; }

    void set_input(const std::string &input) { config.input = input; }

    std::string get_output() { return config.output; }

    void set_output(const std::string &output) { config.output = output; }

    bool get_continue_on_error() { return config.continue_on_error; }

    void set_continue_on_error(const std::string &continue_on_error) {
        std::string tmp;
        std::transform(continue_on_error.begin(), continue_on_error.end(), tmp.begin(), tolower);
        if (tmp == "1" || tmp == "true") {
            config.continue_on_error = true;
        } else {
            config.continue_on_error = false;
        }
    }

 private:
    Config config;
};

}  // end of namespace multithread_client