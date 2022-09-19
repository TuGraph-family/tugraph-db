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
    int num_iterations = 20;
    try {
        json input = json::parse(request);
        if (input["num_iterations"].is_number()) {
            num_iterations = input["num_iterations"].get<int>();
        }
    } catch (std::exception& e) {
        throw std::runtime_error("json parse error");
        return false;
    }

    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<double> pr = olapondb.AllocVertexArray<double>();
    PageRankCore(olapondb, num_iterations, pr);
    auto all_vertices = olapondb.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_pr_vi =
        olapondb.ProcessVertexActive<size_t>([&](size_t vi) { return vi; }, all_vertices, 0,
                                    [&](size_t a, size_t b) { return pr[a] > pr[b] ? a : b; });
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write pr back to graph
    printf("max rank value is pr[%ld] = %lf\n", max_pr_vi, pr[max_pr_vi]);
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["max_pr_id"] = olapondb.OriginalVid(max_pr_vi);
        output["max_pr_val"] = pr[max_pr_vi];
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
