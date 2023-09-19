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
    int make_symmetric = 1;
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(make_symmetric, "make_symmetric", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    size_t construct_param = SNAPSHOT_PARALLEL;
    if (make_symmetric != 0) {
        construct_param = SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED;
    }
    OlapOnDB<Empty> olapondb(db, txn, construct_param);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto num_triangle = olapondb.AllocVertexArray<size_t>();
    num_triangle.Fill(0);
    auto discovered_triangles = FastTriangleCore(olapondb, num_triangle);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write counts back to graph
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["discovered_triangles"] = discovered_triangles;
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
