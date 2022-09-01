/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#pragma once

#include <string>

namespace lgraph {

struct PluginDesc {
    PluginDesc() {}
    PluginDesc(const std::string& n, const std::string d, bool ro)
        : name(n), desc(d), read_only(ro) {}

    std::string name;
    std::string desc;
    bool read_only;
};

struct PluginCode {
    PluginCode() {}
    PluginCode(const std::string& n, const std::string d, bool ro, const std::string c,
               const std::string ct)
        : name(n), desc(d), read_only(ro), code(c), code_type(ct) {}

    std::string name;
    std::string desc;
    bool read_only;
    std::string code;
    std::string code_type;
};

}  // namespace lgraph
