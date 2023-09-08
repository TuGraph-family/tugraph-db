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

#include "lgraph/olap_on_db.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    auto start_time = get_time();

    // prepare
    start_time = get_time();
    size_t root_vid = 0;
    int hop_value = 1;
    try {
        json input = json::parse(request);
        if (input["root_vid"].is_number()) {
            root_vid = input["root_vid"].get<int64_t>();
        }
        if (input["hop_value"].is_number()) {
            hop_value = input["hop_value"].get<int64_t>();
        }
    } catch (std::exception& e) {
        throw std::runtime_error("json parse error");
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto discovered_num = ENCore(olapondb, root_vid, hop_value);
    auto core_cost = get_time() - start_time;

    // return
    {
        json output;
        output["num_reachable_vertices"] = discovered_num;
        output["num_vertices"] = olapondb.NumVertices();
        output["num_edges"] = olapondb.NumEdges();
        output["prepare_cost"] = prepare_cost;
        output["core_cost"] = core_cost;
        output["total_cost"] = prepare_cost + core_cost;
        response = output.dump();
    }
    return true;
}
