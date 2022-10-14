/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
    int64_t root_id = 0;
    std::string label = "node";
    std::string field = "id";
    std::cout << "Input: " << request << std::endl;
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
        throw std::runtime_error("json parse error");
        return false;
    }
    auto txn = db.CreateReadTxn();
    lgraph_api::FieldData root_field_data(root_id);
    int64_t root_vid =
        txn.GetVertexIndexIterator(label, field, root_field_data, root_field_data).GetVid();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<size_t> parent = olapondb.AllocVertexArray<size_t>();
    size_t count = BFSCore(olapondb, root_vid, parent);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write parent back to graph
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
