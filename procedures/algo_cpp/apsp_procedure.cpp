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
    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL, nullptr, edge_convert_default<double>);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    std::tuple<size_t, size_t, double> max_tuple;
    std::vector< std::tuple<size_t, size_t, double> > result;
    APSPCore(olapondb, result, max_tuple);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write distance back to graph
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_distance_src"] = olapondb.OriginalVid(std::get<0>(max_tuple));
        output["max_distance_dst"] = olapondb.OriginalVid(std::get<1>(max_tuple));
        output["max_distance_val"] = std::get<2>(max_tuple);
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
