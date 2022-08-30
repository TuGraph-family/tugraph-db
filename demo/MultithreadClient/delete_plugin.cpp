
#include "delete_plugin.h"
#include "tools/json.hpp"
#include <sstream>
#include <fstream>
#include <boost/algorithm/string/trim_all.hpp>

namespace multithread_client {

    void DeletePlugin::process() {
        std::fstream ifs(config.input, std::fstream::in);
        std::fstream ofs(config.output, std::fstream::out);
        std::string line;
        while (std::getline(ifs, line)) {
            if (line.empty()) continue;
            size_t idx = 0;
            if ((idx = line.find("##"), idx != std::string::npos)) {
                if (idx == 0) continue;
                line = line.substr(0, idx);
            }
            std::string cypher;
            std::string plugin;
            std::string graph;
            if (!parse_line(line, cypher, plugin, graph)) {
                ofs << "configuration file missing field" << "\n";
                if (!config.continue_on_error) {
                    return;
                }
            }
            std::string res;
            bool success = channel->CallCypher(res, cypher, graph);
            if (!success) {
                ofs << "failure delete " << plugin <<  " from " << graph << " because " << res << "\n";
                if (!config.continue_on_error) {
                    return;
                }
            } else {
                ofs << "success delete " << plugin << " from " << graph << "\n";
            }
        }
    }

    bool DeletePlugin::parse_line(std::string& line, std::string& cypher, std::string& plugin, std::string& graph) {
        boost::trim_all(line);
        nlohmann::json obj = nlohmann::json::parse(line);
        if (!obj.contains("PluginType") || !obj.contains("PluginName") || !obj.contains("Graph")) {
            return false;
        }
        std::stringstream ss;
        ss << "CALL db.plugin.deletePlugin('";
        ss << obj["PluginType"].get<std::string>();
        ss << "','";
        ss << obj["PluginName"].get<std::string>();
        ss << "')";
        cypher = ss.str();
        plugin = obj["PluginName"].get<std::string>();
        graph = obj["Graph"].get<std::string>();
        return true;
    }
} // endof namespace multithread_client