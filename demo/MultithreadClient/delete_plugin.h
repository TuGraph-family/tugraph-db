#pragma once

#include "config.h"
#include "lgraph/lgraph_rpc_client.h"
#include <memory>
#include <string>

namespace multithread_client {

class DeletePlugin {
 public:
    DeletePlugin(const Config& config) : config(config) {
        channel = std::make_shared<lgraph::RpcClient>(config.host, config.user, config.password);
    }

    void process();

 private:
    bool parse_line(std::string& line, std::string& cypher, std::string& plugin,
                    std::string& graph);

    const Config& config;
    std::shared_ptr<lgraph::RpcClient> channel;
};

}  // namespace multithread_client