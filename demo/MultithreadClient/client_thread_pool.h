#pragma once

#include "lgraph/lgraph_rpc_client.h"
#include "config.h"
#include <vector>
#include <queue>
#include <thread>
#include <mutex>
#include <functional>
#include <condition_variable>

namespace multithread_client {

class BlockQueue {
 public:
    BlockQueue() : flag(false) {}

    void push(std::string param) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!queue.empty()) {
            queue.push(param);
            return;
        }
        queue.push(param);
        cond.notify_one();
    }

    void stop() {
        std::unique_lock<std::mutex> lock(mutex);
        flag = true;
        cond.notify_one();
    }

    std::string get() {
        std::unique_lock<std::mutex> lock(mutex);
        while (queue.empty()) {
            if (flag) return "";
            cond.wait(lock);
        }
        std::string val = std::move(queue.front());
        queue.pop();
        return val;
    }

 private:
    std::mutex mutex;
    std::condition_variable cond;
    std::queue<std::string> queue;
    bool flag;
};

class ClientThread {
 public:
    ClientThread(const Config& conf) : config(conf) {
        channel = std::make_shared<lgraph::RpcClient>(config.host, config.user, config.password);
    }

    void assign(const std::string& param) { queue.push(param); }

    std::string fetch() { return queue.get(); }

    void start(const std::function<void(ClientThread*)>& func) {
        thread = std::make_shared<std::thread>(func, this);
    }

    void join() { thread->join(); }

    std::shared_ptr<lgraph::RpcClient> get_channel() { return channel; }

    void stop() { queue.stop(); }

    std::unordered_map<std::string, PerformanceIndicator>& get_indicator() { return indicator; }

 private:
    const Config& config;
    std::shared_ptr<std::thread> thread;
    BlockQueue queue;
    std::shared_ptr<lgraph::RpcClient> channel;
    std::unordered_map<std::string, PerformanceIndicator> indicator;
};

class ClientThreadPool {
 public:
    ClientThreadPool(const Config& conf) : config(conf) { threads.reserve(config.thread_num); }

    void start(const std::function<void(ClientThread*)>& func) {
        for (uint32_t idx = 0; idx < config.thread_num; ++idx) {
            std::shared_ptr<ClientThread> sp = std::make_shared<ClientThread>(config);
            sp->start(func);
            threads.push_back(sp);
        }
    }

    void join() {
        for (uint32_t idx = 0; idx < threads.size(); ++idx) {
            threads[idx]->join();
        }
    }

    void assign(const std::string& param, uint32_t times) {
        for (uint32_t idx = 0; idx < times; ++idx) {
            uint32_t pos = idx % config.thread_num;
            threads[pos]->assign(param);
        }
    }

    void stop() {
        for (uint32_t idx = 0; idx < threads.size(); ++idx) {
            threads[idx]->stop();
        }
    }

    std::vector<std::shared_ptr<ClientThread>>& get_threads() { return threads; }

 private:
    const Config& config;
    std::vector<std::shared_ptr<ClientThread>> threads;
};

}  // end of namespace multithread_client