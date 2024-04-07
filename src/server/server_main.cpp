/**
 * Copyright 2024 AntGroup CO., Ltd.
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

#include <fma-common/configuration.h>
#include "server/lgraph_server.h"
#include "server/service.h"
namespace lgraph {
class LGraphDaemon : public Service {
    LGraphServer server_;

 public:
    explicit LGraphDaemon(std::shared_ptr<lgraph::GlobalConfig> config)
        : Service("lgraph", "./lgraph.pid"), server_(config) {}

    int Run() override {
        auto ret = server_.Start();
        if (ret) return ret;
        return server_.WaitTillKilled();
    }
};
}  // namespace lgraph

int main(int argc, char** argv) {
    // get config file path, if specified
    std::string config_file = "/usr/local/etc/lgraph.json";
    std::string cmd = "run";
    try {
        if (!lgraph::GlobalConfig::PrintVersion(config_file, cmd, &argc, &argv)) {
            return 0;
        }
    } catch (std::exception& e) {
        LOG_ERROR() << e.what();
        return -1;
    }
    // try to read the config file
    std::string json;
    {
        fma_common::InputFmaStream f(config_file);
        if (!f.Good()) {
            LOG_WARN() << "Error opening config file " << config_file;
            return -1;
        }
        json.resize(f.Size());
        f.Read(&json[0], json.size());
    }
    // parse config file & command line option
    std::shared_ptr<lgraph::GlobalConfig> config = std::make_shared<lgraph::GlobalConfig>();
    fma_common::Configuration argparser = config->InitConfig(cmd);
    try {
        argparser.ParseJson(json.c_str());
    } catch (std::exception& e) {
        LOG_ERROR() << "Failed to parse config file " << config_file << ": " << e.what();
        return -1;
    }
    try {
        argparser.ExitAfterHelp(true);
        argparser.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        LOG_ERROR() << e.what();
        return -1;
    }
    // now run the service
    lgraph::LGraphDaemon daemon(config);
    if (cmd == "run") {
        return daemon.Run();
    } else if (cmd == "start") {
        return daemon.Start();
    } else if (cmd == "restart") {
        return daemon.Restart();
    } else if (cmd == "stop") {
        return daemon.Stop();
    }
}
