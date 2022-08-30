/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
        server_.Start();
        return server_.WaitTillKilled();
    }
};
}  // namespace lgraph

int main(int argc, char** argv) {
    // get config file path, if specified
    std::string config_file = "/usr/local/etc/lgraph.json";
    std::string cmd = "run";
    if (!lgraph::GlobalConfig::PrintVersion(config_file, cmd, &argc, &argv)) {
        return 0;
    }
    // try to read the config file
    std::string json;
    {
        fma_common::InputFmaStream f(config_file);
        if (!f.Good()) {
            FMA_WARN() << "Error opening config file " << config_file;
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
        FMA_ERR() << "Failed to parse config file " << config_file << ": " << e.what();
        return -1;
    }
    try {
        argparser.ExitAfterHelp(true);
        argparser.ParseAndFinalize(argc, argv);
    } catch (std::exception& e) {
        FMA_ERR() << "Failed to parse command line option: " << e.what();
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
