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
    std::vector<int64_t> root_vids = {0, 1};
    std::string label = "node";
    std::string field = "id";

    // prepare
    start_time = get_time();
    try {
        json input = json::parse(request);
        parse_from_json(label, "label", input);
        parse_from_json(field, "field", input);
        parse_from_json(root_vids, "root_vids", input);
    } catch (std::exception& e) {
        throw std::runtime_error("json parse error, root_vid needed");
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL, nullptr, edge_convert_default<double>);

    std::vector<size_t> roots;
    for (auto ele : root_vids) {
        lgraph_api::FieldData root_field_data(ele);
        int64_t root_vid = txn.GetVertexIndexIterator
                (label, field, root_field_data, root_field_data).GetVid();
        roots.push_back(olapondb.MappedVid(root_vid));
    }
    auto prepare_cost = get_time() - start_time;

     // core
    start_time = get_time();
    ParallelVector<double> distance = olapondb.AllocVertexArray<double>();
    MSSPCore(olapondb, roots, distance);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto all_vertices = olapondb.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_distance_vi = olapondb.ProcessVertexActive<size_t>(
        [&](size_t vi) {
            if (distance[vi] >= 1e10) {
                distance[vi] = -1;
            }
            return (size_t)vi;
        },
        all_vertices, 0, [&](size_t a, size_t b) {
            return (distance[a] > distance[b] || (distance[a] == distance[b] &&
                   olapondb.OriginalVid(a) < olapondb.OriginalVid(b))) ? a : b; });
    auto output_cost = get_time() - start_time;

    // return
    json output;
    output["max_distance_vid"] = max_distance_vi;
    output["max_distance_val"] = distance[max_distance_vi];
    output["num_vertices"] = olapondb.NumVertices();
    output["num_edges"] = olapondb.NumEdges();
    output["prepare_cost"] = prepare_cost;
    output["core_cost"] = core_cost;
    output["output_cost"] = output_cost;
    output["total_cost"] = prepare_cost + core_cost + output_cost;
    response = output.dump();
    return true;
}
