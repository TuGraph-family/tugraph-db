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
    double start_time;

    // prepare
    start_time = get_time();
    int iterations = 100;
    double delta = 0.0001;

    try {
        json input = json::parse(request);
        parse_from_json(iterations, "iterations", input);
        parse_from_json(delta, "delta", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    size_t max_authority_index = 0;
    size_t max_hub_index = 0;
    auto authority = olapondb.AllocVertexArray<double>();
    auto hub = olapondb.AllocVertexArray<double>();
    HITSCore(olapondb, authority, hub, iterations, delta);
    for (size_t vi = 0; vi < olapondb.NumVertices(); vi++) {
        if (authority[vi] > authority[max_authority_index]) {
            max_authority_index = vi;
        }
        if (hub[vi] > hub[max_hub_index]) {
            max_hub_index = vi;
        }
    }
    printf("max authority value is authority[%lu] = %lf\n",
                max_authority_index, authority[max_authority_index]);
    printf("max hub value is hub[%lu] = %lf\n", max_hub_index, hub[max_hub_index]);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write distance back to graph
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_authority_index"] = max_authority_index;
        output["max_authority"] = authority[max_authority_index];
        output["max_hub_index"] = max_hub_index;
        output["max_hub"] = hub[max_hub_index];
        output["num_vertices"] = olapondb.NumVertices();
        output["num_edges"] = olapondb.NumEdges();
        output["prepare_cost"] = prepare_cost;
        output["core_cost"] = core_cost;
        output["output_cost"] = output_cost;
        output["total_cost"] = prepare_cost + core_cost + output_cost;
        response = output.dump();
    }
    return true;
}
