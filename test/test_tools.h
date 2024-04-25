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

#pragma once
#include <set>

#include "fma-common/configuration.h"
#include "fma-common/file_system.h"
#include "fma-common/type_traits.h"
#include "./ut_utils.h"
#include "tiny-process-library/process.hpp"

#include "core/global_config.h"

namespace lgraph {
inline std::unique_ptr<SubProcess> StartLGraphServer(const GlobalConfig& conf,
                                                     bool wait_till_up = true,
                                                     bool skip_license_check = true) {
    std::string server_cmd = fma_common::StringFormatter::Format(
        "./lgraph_server -c lgraph_standalone.json"
        " --host {}"
        " --port {}"
        " --enable_rpc {}"
        " --use_pthread {}"
        " --rpc_port {}"
        " --enable_backup_log {}"
        " --enable_ip_check {}"
        " --verbose {}"
        " --directory {}"
        " --ssl_auth {}"
        " --server_cert {}"
        " --server_key {}"
        " --use_pthread {}"
        " --bolt_port {}",
        conf.bind_host, conf.http_port, conf.enable_rpc, conf.use_pthread, conf.rpc_port,
        conf.enable_backup_log, conf.enable_ip_check,
        conf.verbose, conf.db_dir, conf.enable_ssl, conf.server_cert_file, conf.server_key_file,
        conf.use_pthread, conf.bolt_port);
    UT_LOG() << "Starting lgraph_server with command: " << server_cmd;
    auto server = std::unique_ptr<SubProcess>(new SubProcess(server_cmd));
    if (wait_till_up && !server->ExpectOutput("Server started.", 10000)) {
        UT_WARN() << "Server failed to start, stderr:\n" << server->Stderr();
    }
    return server;
}

inline void WriteCertFiles(const std::string& cert_path, const std::string& key_path) {
    static const std::string key = R"(-----BEGIN RSA PRIVATE KEY-----
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
-----END RSA PRIVATE KEY-----
)";
    static const std::string cert = R"(-----BEGIN CERTIFICATE-----
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
rw==
-----END CERTIFICATE-----
)";
    fma_common::OutputFmaStream key_file(key_path);
    key_file.Write(key.data(), key.size());
    fma_common::OutputFmaStream cert_file(cert_path);
    cert_file.Write(cert.data(), cert.size());
}

#define UT_FMT fma_common::StringFormatter::Format

#define FMA_FMT fma_common::StringFormatter::Format

class TestMarker {
    static int& GetLevel() {
        static int level = 0;
        return level;
    }

    std::string test_name_;
    int level_ = 0;

 public:
    explicit TestMarker(const std::string& test_name) {
        test_name_ = test_name;
        level_ = GetLevel()++;
        UT_LOG() << "\n" << std::string((level_ + 1) * 2, '*') << " BEGIN Testing " << test_name_;
    }

    ~TestMarker() {
        GetLevel()--;
        UT_LOG() << "\n" << std::string((level_ + 1) * 2, '*') << " END Testing " << test_name_;
    }
};
}  // namespace lgraph

inline std::set<std::string>& _Tests() {
    static std::set<std::string> t;
    return t;
}

inline std::set<std::string>& _Skips() {
    static std::set<std::string> s;
    return s;
}

inline void _ParseTests(int* argc, char*** argv) {
    std::string tests = "all";
    std::string skips;
    fma_common::Configuration conf;
    conf.Add(tests, "tests", true);
    conf.Add(skips, "skip", true);
    conf.ParseAndRemove(argc, argv);
    conf.Finalize();
    auto ts = fma_common::Split(tests, ",");
    _Tests().insert(ts.begin(), ts.end());
    auto ss = fma_common::Split(skips, ",");
    _Skips().insert(ss.begin(), ss.end());
    for (auto& s : _Skips()) _Tests().erase(s);
}

#define ParseTestAndSkips(argc, argv) _ParseTests(&argc, &argv)

#define MarkTestBegin(name) auto __t_marker__ = lgraph::TestMarker(name)
#define DefineTest(name)                                                                     \
    if ((_Tests().find("all") != _Tests().end() || _Tests().find(name) != _Tests().end()) && \
        _Skips().find(name) == _Skips().end())

class Barrier {
    size_t n_threads_;
    size_t waiting_threads_ = 0;
    size_t released_threads_ = 0;
    std::mutex mu_;
    std::condition_variable cond_;

 public:
    explicit Barrier(size_t n_threads) :n_threads_(n_threads), waiting_threads_(0) {}

    void Wait() {
        std::unique_lock<std::mutex> lock(mu_);
        waiting_threads_++;
        if (waiting_threads_ == n_threads_) {
            released_threads_ = 1;
            cond_.notify_all();
            while (released_threads_ != n_threads_) cond_.wait(lock);
            // all released, reset lock
            waiting_threads_ = 0;
        } else {
            while (waiting_threads_ != n_threads_) cond_.wait(lock);
            released_threads_++;
            cond_.notify_one();
        }
    }
};
