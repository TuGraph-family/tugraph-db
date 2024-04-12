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

    std::vector<std::unordered_set<size_t>> query;
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(query, "query", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    if (query.empty()) {
        query.emplace_back(std::unordered_set<size_t>{1, 2});
        query.emplace_back(std::unordered_set<size_t>{2});
        query.emplace_back(std::unordered_set<size_t>{});
//        throw std::runtime_error("query is empty.");
    }

    auto txn = db.CreateReadTxn();

    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);

    auto preprocessing_time = get_time();
    auto counts = olapondb.AllocVertexArray<size_t>();
    size_t subgraph_num = SubgraphIsomorphismCore(olapondb, query, counts);
    auto exec_time = get_time();

    json output;
    // TODO(any): write count back to graph
    output["match_subgraph_num"] = subgraph_num;
    auto output_time = get_time();

    output["preprocessing_time"] = (double)(preprocessing_time - start_time);
    output["exec_time"] = (double)(exec_time - preprocessing_time);
    output["output_time"] = (double)(output_time - exec_time);
    output["sum_time"] = (double)(output_time - start_time);
    response = output.dump();
    return true;
}
