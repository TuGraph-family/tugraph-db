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

#include <string>

namespace lgraph {

struct PluginDesc {
    PluginDesc() {}
    PluginDesc(const std::string& n, const std::string co, const std::string d, const std::string v,
               bool ro, const std::string& s)
        : name(n), code_type(co), desc(d), version(v), read_only(ro), signature(s) {}

    std::string name;
    std::string code_type;
    std::string desc;
    std::string version;
    bool read_only;
    std::string signature;
};

struct PluginCode {
    PluginCode() {}
    PluginCode(const std::string& n, const std::string d, const std::string v, bool ro,
               const std::string c, const std::string ct)
        : name(n), desc(d), version(v), read_only(ro), code(c), code_type(ct) {}

    std::string name;
    std::string desc;
    std::string version;
    bool read_only;
    std::string code;
    std::string code_type;
};

}  // namespace lgraph
