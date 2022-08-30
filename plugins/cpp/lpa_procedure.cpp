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
    int num_iterations = 20;
    int sync_flag = 1;
    try {
        json input = json::parse(request);
        if (input["num_iterations"].is_number()) {
            num_iterations = input["num_iterations"].get<int64_t>();
        }
        if (input["sync_flag"].is_number()) {
            sync_flag = input["sync_flag"].get<int64_t>();
        }
    } catch (std::exception& e) {
        throw std::runtime_error("json parse error");
        return false;
    }
    auto txn = db.CreateReadTxn();
    Snapshot<Empty> snapshot(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    double modularity = LPACore(snapshot, num_iterations, sync_flag);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["modularity"] = modularity;
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
