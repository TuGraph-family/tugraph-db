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

int compare(const void *a, const void *b) { return *(double *)a > *(double *)b ? -1 : 1; }

extern "C" bool Process(GraphDB &db, const std::string &request, std::string &response) {
    double start_time;

    // prepare
    start_time = get_time();
    int num_iterations = 20;
    try {
        json input = json::parse(request);
        parse_from_json(num_iterations, "num_iterations", input);
    } catch (std::exception &e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL, nullptr, edge_convert_default<double>);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto wpr = olapondb.AllocVertexArray<double>();
    WPageRankCore(olapondb, num_iterations, wpr);
    auto all_vertices = olapondb.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_wpr_vi =
        olapondb.ProcessVertexActive<size_t>([&](size_t vi) { return vi; }, all_vertices, 0,
                                    [&](size_t a, size_t b) { return wpr[a] > wpr[b] ? a : b; });
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write wpr back to graph
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_wpr_vi"] = max_wpr_vi;
        output["max_wpr_value"] = wpr[max_wpr_vi];
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
