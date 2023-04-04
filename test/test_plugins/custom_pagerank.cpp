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
        {.name = "num_iteration", .index = 0, .type = LGraphType::INTEGER},
    };
    sig_spec.result_list = {
        {.name = "node", .index = 0, .type = LGraphType::NODE},
        {.name = "weight", .index = 1, .type = LGraphType::FLOAT}
    };
    return true;
}

extern "C" LGAPI bool Process(GraphDB &db, const std::string &request, std::string &response) {
    int64_t num_iteration;
    std::cout << "request = " << request << std::endl;
    try {
        json input = json::parse(request);
        num_iteration = input["num_iteration"].get<int64_t>();
    } catch (std::exception &e) {
        response = std::string("error parsing json: ") + e.what();
        return false;
    }
    // handle the page rank algo in dummy mode
    // ...
    std::cout << "number iteration = " << num_iteration << std::endl;

    std::vector<json> result;
    for (size_t i = 0; i < 2; i++) {
        json element;
        element["node"] = "V[" + std::to_string(i) + "]";
        element["weight"] = float(i) + 0.1*float(i);
        result.emplace_back(std::move(element));
    }
    json output = result;
    response = output.dump();
    return true;
}
