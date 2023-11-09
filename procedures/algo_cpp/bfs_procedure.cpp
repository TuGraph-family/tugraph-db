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
    std::string root_id = "0";
    std::string vertex_label = "";
    std::string vertex_field = "";
    std::string edge_label = "";
    std::string parent_id = "";
    std::string output_file = "";
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(root_id, "root_id", input);
        parse_from_json(vertex_label, "vertex_label", input);
        parse_from_json(vertex_field, "vertex_field", input);
        parse_from_json(edge_label, "edge_label", input);
        parse_from_json(parent_id, "parent_id", input);
        parse_from_json(output_file, "output_file", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    auto txn = db.CreateReadTxn();

    std::function<bool(VertexIterator &)> vertex_filter = nullptr;
    std::function<bool(OutEdgeIterator &, Empty &)> edge_filter = nullptr;

    if (!vertex_label.empty()) {
        vertex_filter = [&vertex_label](VertexIterator& vit) {
            return vit.GetLabel() == vertex_label;
        };
        if (edge_label.empty()) {
            throw InputError("vertex_label is set, but edge_label is not set");
        }
        if (vertex_field.empty()) {
            throw InputError("vertex_label is set, but vertex_field is not set");
        }
    }

    if (!edge_label.empty()) {
        edge_filter = [&edge_label](OutEdgeIterator& eit, Empty& edata) {
            return eit.GetLabel() == edge_label;
        };
    }

    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL, vertex_filter, edge_filter);
    if (vertex_label.empty() && vertex_field.empty()) {
        vertex_label = "node";
        vertex_field = "id";
    }
    int64_t root_vid = txn.GetVertexIndexIterator(vertex_label,
                vertex_field, root_id, root_id).GetVid();
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<size_t> parent = olapondb.AllocVertexArray<size_t>();
    size_t count = BFSCore(olapondb, olapondb.MappedVid(root_vid), parent);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
#pragma omp parallel for
    for (size_t i = 0; i < parent.Size(); i++) {
        if (parent[i] != (size_t)-1) {
            parent[i] = olapondb.OriginalVid(parent[i]);
        }
    }
    if (output_file != "") {
        olapondb.WriteToFile<size_t>(parent, output_file);
    }
    txn.Commit();

    if (parent_id != "") {
        olapondb.WriteToGraphDB<size_t>(parent, parent_id);
    }

    printf("found_vertices = %ld\n", count);
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["found_vertices"] = count;
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
