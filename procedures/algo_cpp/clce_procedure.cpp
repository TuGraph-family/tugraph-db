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

    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL, nullptr, edge_convert_default<double>);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto score = olapondb.AllocVertexArray<double>();
    auto path_num = olapondb.AllocVertexArray<size_t>();
    CLCECore(olapondb, samples, score, path_num);
    auto active_all = olapondb.AllocVertexSubset();
    active_all.Fill();
    size_t max_score_vi = 0;
    olapondb.ProcessVertexActive<size_t>([&](size_t vi) {
        if (path_num[vi] > samples / 5 && score[vi] > score[max_score_vi]) {
            max_score_vi = vi;
        }
        return 0;
    }, active_all);
    double max_length = score[max_score_vi];
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_length_vi"] = olapondb.OriginalVid(max_score_vi);
        output["max_length"] = max_length;
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
