/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include "lgraph/olap_base.h"
#include "lgraph/olap_on_db.h"
#include "tools/json.hpp"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

size_t BFSCore(OlapBase<Empty>& graph, size_t root_vid, ParallelVector<size_t>& parent) {
    size_t root = root_vid;
    auto active_in = graph.AllocVertexSubset();
    active_in.Add(root);
    auto active_out = graph.AllocVertexSubset();
    parent.Fill((size_t)-1);
    parent[root] = root;

    size_t num_activations = 1;
    size_t discovered_vertices = 0;
    for (int ii = 0; num_activations != 0; ii++) {
        printf("activates(%d) <= %lu\n", ii, num_activations);
        discovered_vertices += num_activations;
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                size_t num_activations = 0;
                for (auto& edge : graph.OutEdges(vi)) {
                    size_t dst = edge.neighbour;
                    if (parent[dst] == (size_t)-1) {
                        auto lock = graph.GuardVertexLock(dst);
                        if (parent[dst] == (size_t)-1) {
                            parent[dst] = vi;
                            num_activations += 1;
                            active_out.Add(dst);
                        }
                    }
                }
                return num_activations;
            },
            active_in);
        active_in.Swap(active_out);
    }
    return discovered_vertices;
}

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    auto start_time = get_time();

    // prepare
    start_time = get_time();
    std::string root_value = "0";
    std::string root_label = "";
    std::string root_field = "";
    std::string vertex_label_filter = "";
    std::string edge_label_filter = "";
    std::string parent_id = "";
    std::string output_file = "";
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(root_value, "root_value", input);
        parse_from_json(root_label, "root_label", input);
        parse_from_json(root_field, "root_field", input);
        parse_from_json(vertex_label_filter, "vertex_label_filter", input);
        parse_from_json(edge_label_filter, "edge_label_filter", input);
        parse_from_json(parent_id, "parent_id", input);
        parse_from_json(output_file, "output_file", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    auto txn = db.CreateReadTxn();

    std::function<bool(VertexIterator&)> vertex_filter = nullptr;
    std::function<bool(OutEdgeIterator&, Empty&)> edge_filter = nullptr;

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

    if (root_label.empty() || root_field.empty()) {
        if (root_label.empty() && root_field.empty()) {
            root_label = "node";
            root_field = "id";
        } else {
            THROW_CODE(InputError, "root_label or root_field is empty");
        }
    }

    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL, vertex_filter, edge_filter);
    int64_t root_vid =
        txn.GetVertexIndexIterator(root_label, root_field, root_value, root_value).GetVid();
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
