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
        {.name = "limit", .index = 0, .type = LGraphType::INTEGER},
    };
    sig_spec.result_list = {
        {.name = "node", .index = 0, .type = LGraphType::NODE},
        {.name = "salt", .index = 1, .type = LGraphType::FLOAT}
    };
    return true;
}

extern "C" LGAPI bool ProcessInTxn(Transaction &txn,
                                   const std::string &request,
                                   std::string &response) {
    int64_t limit;
    try {
        json input = json::parse(request);
        limit = input["limit"].get<int64_t>();
    } catch (std::exception &e) {
        response = std::string("error parsing json: ") + e.what();
        return false;
    }

    Result result({{"node", LGraphType::NODE},
                   {"salt", LGraphType::FLOAT},
                   });
    for (size_t i = 0; i < limit; i++) {
        auto r = result.MutableRecord();
        auto vit = txn.GetVertexIterator(i);
        r->Insert("node", vit);
        r->Insert("salt", FieldData::Float(20.23*float(i)));
    }
    response = result.Dump();
    return true;
}
