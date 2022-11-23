/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/olap_on_db.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

void CountComp(OlapBase<Empty>& graph, ParallelVector<size_t>& label, size_t& max, size_t& num) {
    ParallelVector<size_t> cnt = graph.AllocVertexArray<size_t>();
    cnt.Fill(0);
    graph.ProcessVertexInRange<size_t>(
        [&](size_t v) {
            if (graph.OutDegree(v) == 0) return 0;
            size_t v_label = label[v];
            write_add(&cnt[v_label], (size_t)1);
            return 0;
        },
        0, label.Size());
    max = 0;
    num = graph.ProcessVertexInRange<size_t>(
        [&](size_t v) {
            if (max < cnt[v]) write_max(&max, cnt[v]);
            return (graph.OutDegree(v) > 0 && cnt[v] > 0) ? 1 : 0;
        },
        0, label.Size());
}

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    double start_time;

    // prepare
    start_time = get_time();
    auto txn = db.CreateReadTxn();
    std::string label = "node";
    std::string field = "id";
    try {
        json input = json::parse(request);
        parse_from_json(label, "label", input);
        parse_from_json(field, "field", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<size_t> wcc_label = olapondb.AllocVertexArray<size_t>();
    WCCCore(olapondb, wcc_label);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write wcc_label back to graph
    size_t num_components, max_component;
    CountComp(olapondb, wcc_label, max_component, num_components);
    printf("max_component = %ld\n", max_component);
    printf("num_components = %ld\n", num_components);
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["num_components"] = num_components;
        output["max_component"] = max_component;
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
