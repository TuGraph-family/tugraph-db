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

/**
 * Processes a request on a GraphDB.
 *
 * @param db The reference to the GraphDB object on which the request will be processed.
 * @param request The input request in JSON format.
 *        The request should contain the following parameters:
 *        - "root_value": The value of the root vertex.
 *        - "root_label": The label of the root vertex.
 *        - "root_field": The field of the root vertex.
 *        - "vertex_label_filter": Filter vertex sets based on vertex labels
 *        - "edge_label_filter": Filter edge sets based on edge labels
 *        - "sssp_distance": Vertex field name to be written back into the database.
 *        - "output_file": Sssp distance to be written to the file.
 * @param response The output response in JSON format.
 *        The response will contain the following parameters:
 *        - "max_distance_vid": The vertex id with the maximum distance.
 *        - "max_distance_val": The maximum distance.
 *        - "num_vertices": The number of vertices in the graph.
 *        - "num_edges": The number of edges in the graph.
 *        - "prepare_cost": The time cost of preparing the graph data.
 *        - "core_cost": The time cost of the core algorithm.
 *        - "output_cost": The time cost of writing the result to a file.
 *        - "total_cost": The total time cost.
 * @return True if the request is processed successfully, false otherwise.
 */
extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    double start_time;

    // prepare
    start_time = get_time();
    std::string root_value = "0";
    std::string root_label = "";
    std::string root_field = "";
    std::string vertex_label_filter = "";
    std::string edge_label_filter = "";
    std::string sssp_distance = "";
    std::string output_file = "";
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(root_value, "root_value", input);
        parse_from_json(root_label, "root_label", input);
        parse_from_json(root_field, "root_field", input);
        parse_from_json(vertex_label_filter, "vertex_label_filter", input);
        parse_from_json(edge_label_filter, "edge_label_filter", input);
        parse_from_json(sssp_distance, "sssp_distance", input);
        parse_from_json(output_file, "output_file", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    std::function<bool(VertexIterator &)> vertex_filter = nullptr;
    std::function<bool(OutEdgeIterator &, double &)> edge_filter = edge_convert_default<double>;

    if (!vertex_label_filter.empty()) {
        vertex_filter = [&vertex_label_filter](VertexIterator& vit) {
            return vit.GetLabel() == vertex_label_filter;
        };
    }

    if (!edge_label_filter.empty()) {
        auto edge_filter = [edge_label_filter](OutEdgeIterator& eit, double& edata) -> bool {
            if (eit.GetLabel() == edge_label_filter) {
                return edge_convert_default<double>(eit, edata);
            }
            return false;
        };
    }

    if (root_label.empty() || root_field.empty()) {
        if (root_label.empty() && root_field.empty()) {
            root_label = "node";
            root_field = "id";
        } else {
            throw InputError("root_label or root_field is empty");
        }
    }

    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL, vertex_filter, edge_filter);
    int64_t root_vid = txn.GetVertexIndexIterator(root_label, root_field,
            root_value, root_value).GetVid();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<double> distance = olapondb.AllocVertexArray<double>();
    SSSPCore(olapondb, olapondb.MappedVid(root_vid), distance);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        olapondb.WriteToFile<double>(distance, output_file);
    }
    txn.Commit();

    if (sssp_distance != "") {
        olapondb.WriteToGraphDB<double>(distance, sssp_distance);
    }

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
    printf("max distance is: distance[%ld]=%lf\n",
            olapondb.OriginalVid(max_distance_vi), distance[max_distance_vi]);
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
