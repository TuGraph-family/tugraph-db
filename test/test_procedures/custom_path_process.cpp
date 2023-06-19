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
#include "lgraph/lgraph_result.h"
#include "lgraph/lgraph_txn.h"

#include "tools/json.hpp"

using json = nlohmann::json;

using namespace lgraph_api;

extern "C" LGAPI bool GetSignature(SigSpec &sig_spec) {
    sig_spec.input_list = {
        {.name = "nodeIds", .index = 0, .type = LGraphType::LIST},
    };
    sig_spec.result_list = {
        {.name = "idSum", .index = 0, .type = LGraphType::INTEGER},
    };
    return true;
}
extern "C" LGAPI bool ProcessInTxn(Transaction &txn,
                                   const std::string &request,
                                   std::string &response) {
    std::vector<int64_t> nodeIds;
    try {
        json input = json::parse(request);
        nodeIds = input["nodeIds"].get<std::vector<std::int64_t>>();
    } catch (std::exception &e) {
        response = std::string("error parsing json: ") + e.what();
        return false;
    }

    int64_t sum = 0;
    for (auto id : nodeIds) {
        sum += id;
    }

    Result result({{"idSum", LGraphType::INTEGER}});

    auto r = result.MutableRecord();
    r->Insert("idSum", FieldData::Int64(sum));
    response = result.Dump();
    return true;
}
