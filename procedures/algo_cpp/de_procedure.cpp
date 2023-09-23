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
    size_t samples = 64;
    try {
        json input = json::parse(request);
        parse_from_json(samples, "samples", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    std::set<size_t> roots;
    unsigned int seed = 0;
    size_t num_vertices = olapondb.NumVertices();
    for (size_t k=0; k < samples; k++) {
        size_t s = rand_r(&seed) % num_vertices;
        roots.insert(s);
    }
    size_t max_path = 0;
    printf("1st phase from %lu seeds\n", roots.size());
    max_path = DECore(olapondb, roots);
    printf("1st phase max path: %ld\n", max_path + 1);
    printf("2nd phase from %lu seeds\n", roots.size());
    max_path = DECore(olapondb, roots);
    printf("2nd phase max path: %ld\n", max_path + 1);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_diamension"] = max_path + 1;
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
