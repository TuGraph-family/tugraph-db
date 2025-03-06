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
    std::string root_value = "0";
    std::string root_label = "node";
    std::string root_field = "id";
    std::string output_file = "";
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(root_value, "root_value", input);
        parse_from_json(root_label, "root_label", input);
        parse_from_json(root_field, "root_field", input);
        parse_from_json(output_file, "output_file", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    auto txn = db.CreateReadTxn();
    int64_t root_vid =
        txn.GetVertexIndexIterator(root_label, root_field, root_value, root_value).GetVid();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    ParallelVector<size_t> parent = olapondb.AllocVertexArray<size_t>();
    size_t count = BFSCore(olapondb, olapondb.MappedVid(root_vid), parent);
    printf("found_vertices = %ld\n", count);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write parent back to graph
    if (output_file != "") {
        auto active_all = olapondb.AllocVertexSubset();
        active_all.Fill();
        olapondb.ProcessVertexActive<int>(
            [&] (size_t vid) {
                if (parent[vid] != (size_t)-1) {
                    parent[vid] = olapondb.OriginalVid(parent[vid]);
                }
                return 0;
            },
            active_all);
        olapondb.WriteToFile<size_t>(parent, output_file, [&](size_t vid, size_t vdata) -> bool {
            return vdata != (size_t)-1;
        });
    }

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
