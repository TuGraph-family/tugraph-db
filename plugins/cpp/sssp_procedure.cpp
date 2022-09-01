/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/lgraph_olap.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    double start_time;

    // prepare
    start_time = get_time();
    int64_t root_id = 0;
    std::string label = "node";
    std::string field = "id";
    try {
        json input = json::parse(request);
        if (input["root_id"].is_number()) {
            root_id = input["root_id"].get<int64_t>();
        }
        if (input["label"].is_string()) {
            label = input["label"].get<std::string>();
        }
        if (input["field"].is_string()) {
            field = input["field"].get<std::string>();
        }
    } catch (std::exception& e) {
        throw std::runtime_error("json parse error, root_id, label, field, output_file needed");
        return false;
    }
    auto txn = db.CreateReadTxn();
    lgraph_api::FieldData root_field_data(root_id);
    int64_t root_vid =
        txn.GetVertexIndexIterator(label, field, root_field_data, root_field_data).GetVid();
    Snapshot<double> snapshot(db, txn, SNAPSHOT_PARALLEL, nullptr, edge_convert_default<double>);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<double> distance = snapshot.AllocVertexArray<double>();
    SSSPCore(snapshot, root_vid, distance);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto all_vertices = snapshot.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_distance_vi = snapshot.ProcessVertexActive<size_t>(
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
                   snapshot.OriginalVid(a) < snapshot.OriginalVid(b)))
                 ? a
                 : b;
        });
    // TODO(any): write distance back to graph
    printf("max distance is: distance[%ld]=%lf\n", max_distance_vi, distance[max_distance_vi]);
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_distance_vid"] = snapshot.OriginalVid(max_distance_vi);
        output["max_distance_val"] = distance[max_distance_vi];
        output["num_vertices"] = snapshot.NumVertices();
        output["num_edges"] = snapshot.NumEdges();
        output["prepare_cost"] = prepare_cost;
        output["core_cost"] = core_cost;
        output["output_cost"] = output_cost;
        output["total_cost"] = prepare_cost + core_cost + output_cost;
        response = output.dump();
    }
    return true;
}
