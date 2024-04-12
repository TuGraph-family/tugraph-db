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
    double gamma = 0.2;
    double theta = 0.1;
    unsigned random_seed = 0;
    size_t threshold = 0;
    std::string weight = "";
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(gamma, "gamma", input);
        assert(gamma > 0 && gamma <= 1);
        parse_from_json(theta, "theta", input);
        assert(theta > 0 && theta <= 1);
        parse_from_json(random_seed, "random_seed", input);
        parse_from_json(weight, "weight", input);
        parse_from_json(threshold, "threshold", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    auto edge_convert = [&](OutEdgeIterator& eit, double& edge_data) -> bool {
        if (weight.length() == 0) {
            edge_data = 1;
            return true;
        }
        edge_data = eit.GetField(weight).real();
        if (edge_data > 0.0) return true;
        return false;
    };
    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED, nullptr,
                              edge_convert);
    printf("|V| = %lu\n", olapondb.NumVertices());
    printf("|E| = %lu\n", olapondb.NumEdges());
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<size_t> label = olapondb.AllocVertexArray<size_t>();
    LeidenCore(olapondb, label, random_seed, theta, gamma, threshold);
    printf("label.size=%lu\n", label.Size());
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    double output_cost = get_time() - start_time;

    // return
    {
        json output;
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
