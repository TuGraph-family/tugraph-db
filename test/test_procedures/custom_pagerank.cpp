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


#include <cstdlib>
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_types.h"
#include "lgraph/lgraph_result.h"

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

extern "C" LGAPI bool ProcessInTxn(Transaction &txn,
                                   const std::string &request,
                                   std::string &response) {
    int64_t num_iteration;
    try {
        json input = json::parse(request);
        num_iteration = input["num_iteration"].get<int64_t>();
    } catch (std::exception &e) {
        response = std::string("error parsing json: ") + e.what();
        return false;
    }
    // handle the page rank algo in dummy mode
    // ...

    Result result({{"node", LGraphType::NODE},
                   {"weight", LGraphType::FLOAT},
                   });


    for (size_t i = 0; i < 2; i++) {
        auto r = result.MutableRecord();
        auto vit = txn.GetVertexIterator(i);
        r->Insert("node", vit);
        r->Insert("weight", FieldData::Float(float(i) + 0.1*float(i)));
    }
    response = result.Dump();
    return true;
}

