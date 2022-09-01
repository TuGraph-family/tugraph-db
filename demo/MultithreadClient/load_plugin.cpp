
#include "load_plugin.h"
#include "tools/json.hpp"
#include <fstream>
#include <boost/algorithm/string/trim_all.hpp>

namespace multithread_client {

    void LoadPlugin::process() {
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
            std::string source_file;
            std::string plugin_type;
            std::string plugin_name;
            std::string code_type;
            std::string plugin_description;
            bool read_only;
            std::string load_graph;
            if (!parse_line(line, source_file, plugin_type, plugin_name, code_type, plugin_description,
                            read_only, load_graph)) {
                ofs << "configuration file missing field" << "\n";
                if (!config.continue_on_error) {
                    return;
                }
            }
            std::string res;
            bool success = channel->LoadPlugin(res, source_file, plugin_type, plugin_name, code_type,
                                               plugin_description, read_only, load_graph);
            if (!success) {
                ofs << "failure load " << " and install " << plugin_name <<  " to " << load_graph << " because " << res << "\n";
                if (!config.continue_on_error) {
                    return;
                }
            } else {
                ofs << "success load " << source_file << " and install " << plugin_name <<  " to " << load_graph << "\n";
            }
        }
    }

    bool LoadPlugin::parse_line(std::string& line, std::string& source_file, std::string& plugin_type,
                                std::string& plugin_name, std::string& code_type, std::string& plugin_description,
                                bool& read_only, std::string& load_graph) {
        boost::trim_all(line);
        nlohmann::json obj = nlohmann::json::parse(line);
        if (!obj.contains("PluginFile") || !obj.contains("PluginType") || !obj.contains("PluginName")
            || !obj.contains("CodeType") || !obj.contains("PluginDescription")
            || !obj.contains("ReadOnly") || !obj.contains("LoadGraph")) {
            return false;
        }
        source_file = obj["PluginFile"].get<std::string>();
        plugin_type = obj["PluginType"].get<std::string>();
        plugin_name = obj["PluginName"].get<std::string>();
        code_type = obj["CodeType"].get<std::string>();
        plugin_description = obj["PluginDescription"].get<std::string>();
        read_only = obj["ReadOnly"].get<bool>();
        load_graph = obj["LoadGraph"].get<std::string>();
        return true;
    }

} // end of namespace multithread_client