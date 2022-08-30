#pragma once

#include "config.h"
#include "lgraph/lgraph_rpc_client.h"
#include <memory>
#include <string>

namespace multithread_client {

class LoadPlugin {
 public:
    LoadPlugin(const Config& config) : config(config) {
        channel = std::make_shared<lgraph::RpcClient>(config.host, config.user, config.password);
    }

    void process();

 private:
    bool parse_line(std::string& line, std::string& source_file, std::string& plugin_type,
                    std::string& plugin_name, std::string& code_type,
                    std::string& plugin_description, bool& read_only, std::string& load_graph);

    const Config& config;
    std::shared_ptr<lgraph::RpcClient> channel;
};

}  // end of namespace multithread_client