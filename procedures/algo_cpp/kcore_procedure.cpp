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
    double start_time;

    // prepare
    start_time = get_time();
    size_t value_k = 10;
    std::string output_file = "";
    try {
        json input = json::parse(request);
        parse_from_json(value_k, "value_k", input);
        parse_from_json(output_file, "output_file", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    auto vertex_convert = [&](VertexIterator& vit) -> bool {
        // size_t vid = vit.GetId();
        return true;
    };
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED, vertex_convert);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto result = olapondb.AllocVertexArray<bool>();
    size_t num_result_vertices = KCoreCore(olapondb, result, value_k);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        FILE* fout = fopen(output_file.c_str(), "w");
#pragma omp parallel for
        for (size_t i = 0; i < olapondb.NumVertices(); i++) {
            if (result[i]) {
                fprintf(fout, "%ld, %d\n", i, result[i]);
            }
        }
        fclose(fout);
    }
    double output_cost = get_time() - start_time;

    json output;
    output["num_result_vertices"] = num_result_vertices;
    output["num_vertices"] = olapondb.NumVertices();
    output["num_edges"] = olapondb.NumEdges();
    output["prepare_cost"] = prepare_cost;
    output["core_cost"] = core_cost;
    output["total_cost"] = prepare_cost + core_cost + output_cost;
    response = output.dump();
    return true;
}
