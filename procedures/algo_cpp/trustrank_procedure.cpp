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
    size_t iterations = 20;
    std::vector<int64_t> trust_list = {0, 1, 3};
    std::string label = "node";
    std::string field = "id";

    try {
        json input = json::parse(request);
        parse_from_json(iterations, "iterations", input);
        parse_from_json(label, "label", input);
        parse_from_json(field, "field", input);
        if (input["trust_list"].is_array()) {
            trust_list.clear();
        }
        for (auto ele : input["trust_list"]) {
            trust_list.push_back(ele);
        }
    } catch (std::exception& e) {
        throw std::runtime_error("json parse error");
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);
    std::vector<size_t> trust;
    for (auto ele : trust_list) {
        lgraph_api::FieldData root_field_data(ele);
        // int64_t root_vid = txn.GetVertexIndexIterator
        //         (label, field, root_field_data, root_field_data).GetVid();
        trust.push_back(olapondb.MappedVid(ele));
    }
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto curr = olapondb.AllocVertexArray<double>();
    TrustrankCore(olapondb, iterations, curr, trust);
    auto all_vertices = olapondb.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_curr_vi =
        olapondb.ProcessVertexActive<size_t>([&](size_t vi) { return vi; }, all_vertices, 0,
                                    [&](size_t a, size_t b) { return curr[a] > curr[b] ? a : b; });
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write curr back to graph
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_trustrank_id"] = olapondb.OriginalVid(max_curr_vi);
        output["max_trustrank_val"] = curr[max_curr_vi];
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
