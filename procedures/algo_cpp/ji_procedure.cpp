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
    std::string src_label = "node";
    std::string src_field = "id";
    std::string dst_label = "node";
    std::string dst_field = "id";
    std::vector<std::pair<std::string, std::string>> search_list = {
        {"0", "1"}, {"1", "972"}, {"101", "202"}};
    auto txn = db.CreateReadTxn();
    try {
        json input = json::parse(request);
        parse_from_json(src_label, "src_label", input);
        parse_from_json(src_field, "src_field", input);
        parse_from_json(dst_label, "dst_label", input);
        parse_from_json(dst_field, "dst_field", input);
        parse_from_json(search_list, "search_pairs", input);
        if (input["search_pairs"].is_array()) {
            search_list.clear();
            for (auto& e : input["search_pairs"]) {
                std::string src_string = static_cast<std::string>(e[0]);
                std::string dst_string = static_cast<std::string>(e[1]);
                search_list.push_back(std::make_pair(src_string, dst_string));
            }
        }
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    std::vector<std::tuple<std::string, std::string, double>> result_list;

    for (auto search_pair : search_list) {
        auto src_string = search_pair.first;
        auto dst_string = search_pair.second;
        auto src_vid =
            txn.GetVertexIndexIterator(src_label, src_field, src_string, src_string).GetVid();
        auto dst_vid =
            txn.GetVertexIndexIterator(dst_label, dst_field, dst_string, dst_string).GetVid();
        auto id_pair = std::make_pair(src_vid, dst_vid);
        double score = JiCore(olapondb, id_pair);
        result_list.push_back(std::make_tuple(src_string, dst_string, score));
    }

    auto core_cost = get_time() - start_time;

    // return
    {
        json output;
        output["ji_list"] = result_list;
        output["num_vertices"] = olapondb.NumVertices();
        output["num_edges"] = olapondb.NumEdges();
        output["prepare_cost"] = prepare_cost;
        output["core_cost"] = core_cost;
        output["total_cost"] = prepare_cost + core_cost;
        response = output.dump();
    }
    return true;
}
