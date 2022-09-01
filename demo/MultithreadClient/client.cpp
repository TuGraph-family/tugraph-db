
#include "client.h"
#include "load_plugin.h"
#include "cypher_sender.h"
#include "plugin_sender.h"
#include "delete_plugin.h"
#include <functional>
#include <unordered_map>

static const std::unordered_map<std::string, std::function<void(multithread_client::Client*)>> sRouter = {
        {"callcypher", std::bind(&multithread_client::Client::call_cypher, std::placeholders::_1)},
        {"loadplugin", std::bind(&multithread_client::Client::load_plugin, std::placeholders::_1)},
        {"callplugin", std::bind(&multithread_client::Client::call_plugin, std::placeholders::_1)},
        {"deleteplugin", std::bind(&multithread_client::Client::delete_plugin, std::placeholders::_1)},
};

namespace multithread_client {

    void Client::call_cypher() {
        std::shared_ptr<CypherSender> cs = std::make_shared<CypherSender>(config);
        cs->process();
    }

    void Client::load_plugin() {
        std::shared_ptr<LoadPlugin> lp = std::make_shared<LoadPlugin>(config);
        lp->process();
    }

    void Client::call_plugin() {
        std::shared_ptr<PluginSender> ps = std::make_shared<PluginSender>(config);
        ps->process();
    }

    void Client::delete_plugin() {
        std::shared_ptr<DeletePlugin> ds = std::make_shared<DeletePlugin>(config);
        ds->process();
    }

    void Client::process() {
        auto it = sRouter.find(config.mode);
        if (it == sRouter.end()) return;
        auto func = it->second;
        func(this);
    }

} // end of namespace multithread_client