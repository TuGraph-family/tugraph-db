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
 *        - "vertex_label_filter": Filter vertex sets based on vertex labels
 *        - "edge_label_filter": Filter edge sets based on edge labels
 *        - "lcc_value": Vertex field name to be written back into the database.
 *        - "output_file": Lcc value to be written to the file.
 * @param response The output response in JSON format.
 *        The response will contain the following parameters:
 *        - "average_lcc": The average lcc of the graph.
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
    std::string vertex_label_filter = "";
    std::string edge_label_filter = "";
    std::string lcc_value = "";
    std::string output_file = "";
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(vertex_label_filter, "vertex_label_filter", input);
        parse_from_json(edge_label_filter, "edge_label_filter", input);
        parse_from_json(lcc_value, "lcc_value", input);
        parse_from_json(output_file, "output_file", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    std::function<bool(VertexIterator &)> vertex_filter = nullptr;
    std::function<bool(OutEdgeIterator &, Empty &)> edge_filter = nullptr;

    if (!vertex_label_filter.empty()) {
        vertex_filter = [&vertex_label_filter](VertexIterator& vit) {
            return vit.GetLabel() == vertex_label_filter;
        };
    }
    if (!edge_label_filter.empty()) {
        edge_filter = [&edge_label_filter](OutEdgeIterator& eit, Empty& edata) {
            return eit.GetLabel() == edge_label_filter;
        };
    }
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED,
            vertex_filter, edge_filter);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto score = olapondb.AllocVertexArray<double>();
    double average_lcc = LCCCore(olapondb, score);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        olapondb.WriteToFile<double>(score, output_file);
    }
    txn.Commit();

    if (lcc_value != "") {
        olapondb.WriteToGraphDB<double>(score, lcc_value);
    }
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["average_lcc"] = average_lcc;
        output["num_vertices"] = olapondb.NumVertices();
        output["num_edges"] = olapondb.NumEdges();
        output["prepare_cost"] = prepare_cost;
        output["core_cost"] = core_cost;
        output["output_cost"] = output_cost;
        output["total_cost"] = prepare_cost + core_cost + output_cost;
        response = output.dump();
        return true;
    }
}
