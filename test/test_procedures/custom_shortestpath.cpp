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
#include "lgraph/lgraph_txn.h"

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
extern "C" LGAPI bool ProcessInTxn(Transaction &txn,
                                   const std::string &request,
                                   std::string &response) {
    std::string start_node, end_node;
    try {
        json input = json::parse(request);
        start_node = input["start"].get<std::string>();
        end_node = input["end"].get<std::string>();
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

    Result result({{"length", LGraphType::INTEGER},
        {"nodeIds", LGraphType::LIST},
    });

    auto r = result.MutableRecord();
    r->Insert("length", FieldData::Int32(5));
    r->Insert("nodeIds",  std::vector<FieldData>{
                            FieldData::Int64(parse_vertex_node(start_node)),
                            FieldData::Int64(100), FieldData::Int64(200), FieldData::Int64(300),
                            FieldData::Int64(parse_vertex_node(end_node))
                        });
    response = result.Dump();
    return true;
}

