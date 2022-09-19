/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto score = olapondb.AllocVertexArray<double>();
    double average_clco = LCCCore(olapondb, score);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["average_clco"] = average_clco;
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
