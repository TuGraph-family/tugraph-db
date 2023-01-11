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
    std::string root_id = "0";
    std::string label = "node";
    std::string field = "id";
    try {
        json input = json::parse(request);
        parse_from_json(root_id, "root_id", input);
        parse_from_json(label, "label", input);
        parse_from_json(field, "field", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    int64_t root_vid = txn.GetVertexIndexIterator(label, field, root_id, root_id).GetVid();
    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL, nullptr, edge_convert_default<double>);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<double> distance = olapondb.AllocVertexArray<double>();
    SSSPCore(olapondb, root_vid, distance);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto all_vertices = olapondb.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_distance_vi = olapondb.ProcessVertexActive<size_t>(
        [&](size_t vi) {
            if (distance[vi] > 1e10) {
                distance[vi] = -1;
            }
            return (size_t)vi;
        },
        all_vertices, 0,
        [&](size_t a, size_t b) {
            return (distance[a] > distance[b] ||
                    (distance[a] == distance[b] &&
                     olapondb.OriginalVid(a) < olapondb.OriginalVid(b)))
                       ? a
                       : b;
        });
    // TODO(any): write distance back to graph
    printf("max distance is: distance[%ld]=%lf\n", max_distance_vi, distance[max_distance_vi]);
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_distance_vid"] = olapondb.OriginalVid(max_distance_vi);
        output["max_distance_val"] = distance[max_distance_vi];
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
