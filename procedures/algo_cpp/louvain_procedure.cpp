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
    double start_time = get_time();

    // prepare
    std::string algorithm = "louvain";
    size_t active_threshold = 0;
    int is_sync = 0;
    std::string output_file = "";
    try {
        json input = json::parse(request);
        parse_from_json(active_threshold, "active_threshold", input);
        parse_from_json(is_sync, "is_sync", input);
        parse_from_json(output_file, "output_file", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<double> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED, nullptr,
                              edge_convert_default<double>);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto label = olapondb.AllocVertexArray<size_t>();
    auto modularity = LouvainCore(olapondb, label, active_threshold, is_sync);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    if (output_file != "") {
        FILE* fout = fopen(output_file.c_str(), "w");
#pragma omp parallel for
        for (size_t i = 0; i < olapondb.NumVertices(); i++) {
            if (label[i]) {
                fprintf(fout, "%ld, %ld\n", i, label[i]);
            }
        }
        fclose(fout);
    }
    double output_cost = get_time() - start_time;

    json output;
    output["modularity"] = modularity;
    output["prepare_cost"] = prepare_cost;
    output["core_cost"] = core_cost;
    output["output_cost"] = output_cost;
    output["total_cost"] = prepare_cost + core_cost + output_cost;
    response = output.dump();
    return true;
}
