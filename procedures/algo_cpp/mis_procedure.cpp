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

    int make_symmetric = 1;
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(make_symmetric, "make_symmetric", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    size_t construct_param = SNAPSHOT_PARALLEL;
    if (make_symmetric != 0) {
        construct_param = SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, construct_param);

    auto preprocessing_time = get_time();
    auto mis = olapondb.AllocVertexArray<bool>();
    size_t mis_size = 0;
    MISCore(olapondb, mis, mis_size);
    auto exec_time = get_time();

    json output;
    output["MIS_size"] = mis_size;
    auto output_time = get_time();

    output["preprocessing_time"] = (double)(preprocessing_time - start_time);
    output["exec_time"] = (double)(exec_time - preprocessing_time);
    output["output_time"] = (double)(output_time - exec_time);
    output["sum_time"] = (double)(output_time - start_time);
    response = output.dump();
    return true;
}
