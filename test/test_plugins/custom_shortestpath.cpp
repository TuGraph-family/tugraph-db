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


#include <iostream>
#include <cstdlib>
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_types.h"

#include "tools/json.hpp"

using json = nlohmann::json;

using namespace lgraph_api;

extern "C" LGAPI bool GetSignature(SigSpec &sig_spec) {
    sig_spec.input_list = {
        {.name = "start", .index = 0, .type = LGraphType::NODE},
        {.name = "end", .index = 1, .type = LGraphType::NODE},
    };
    sig_spec.result_list = {
        {.name = "length", .index = 0, .type = LGraphType::INTEGER},
        {.name = "nodeIds", .index = 1, .type = LGraphType::LIST},
    };
    return true;
}

extern "C" LGAPI bool Process(GraphDB &db, const std::string &request, std::string &response) {
    std::string start_node, end_node;
    std::cout << "request = " << request << std::endl;
    try {
        json input = json::parse(request);
        start_node = input["start"].get<std::string>();
        end_node = input["end"].get<std::string>();
        std::cout << "start = " << start_node << "end = " << end_node << std::endl;
    } catch (std::exception &e) {
        response = std::string("error parsing json: ") + e.what();
        return false;
    }
    // handle the shortest path algo in dummy mode
    // ...
    auto parse_vertex_node = [](const std::string& s) -> int64_t {
        auto id_begin = s.find("[");
        return strtoll(s.c_str() + id_begin + 1, NULL, 10);
    };

    std::vector<json> records;
    json element1;
    element1["length"] = 5;
    element1["nodeIds"] = std::vector<int64_t>{
        parse_vertex_node(start_node),
        100, 200, 300,
        parse_vertex_node(end_node)
    };
    records.emplace_back(std::move(element1));
    json output = records;
    response = output.dump();
    return true;
}
