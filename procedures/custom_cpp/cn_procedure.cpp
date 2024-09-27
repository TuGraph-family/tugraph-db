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
#include "../algo_cpp/algo.h"

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
    std::vector<std::pair<size_t, size_t> > search_list
        = {{0, 1}, {1, 972}, {101, 202}, {1000, 4000}};
    auto txn = db.CreateReadTxn();
    try {
        json input = json::parse(request);
        parse_from_json(src_label, "src_label", input);
        parse_from_json(src_field, "src_field", input);
        parse_from_json(dst_label, "dst_label", input);
        parse_from_json(dst_field, "dst_field", input);
        if (input["search_pairs"].is_array()) {
            search_list.clear();
            for (auto &e : input["search_pairs"]) {
                std::string src_id = e[0].get<std::string>();
                std::string dst_id = e[1].get<std::string>();
                lgraph_api::FieldData src_field_data(src_id);
                lgraph_api::FieldData dst_field_data(dst_id);
                size_t src = txn.GetVertexIndexIterator(src_label, src_field,
                                        src_field_data, src_field_data).GetVid();
                size_t dst = txn.GetVertexIndexIterator(dst_label, dst_field,
                                        dst_field_data, dst_field_data).GetVid();
                search_list.push_back(std::make_pair(src, dst));
            }
        }
    } catch (std::exception &e) {
        throw std::runtime_error("json parse error");
    }

    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL | SNAPSHOT_UNDIRECTED);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    std::vector< std::tuple<size_t, std::string, std::string, std::string,
            size_t, std::string, std::string, std::string, double> > result_list;
    for (auto search_pair : search_list) {
        size_t count_common = CNCore(olapondb, search_pair);
        auto vit_first = txn.GetVertexIterator(search_pair.first, false);
        auto vit_first_label = vit_first.GetLabel();
        auto vit_first_primary_field = txn.GetVertexPrimaryField(vit_first_label);
        auto vit_first_field_data = vit_first.GetField(vit_first_primary_field);

        auto vit_second = txn.GetVertexIterator(search_pair.second, false);
        auto vit_second_label = vit_second.GetLabel();
        auto vit_second_primary_field = txn.GetVertexPrimaryField(vit_second_label);
        auto vit_second_field_data = vit_second.GetField(vit_second_primary_field);
        result_list.push_back(std::make_tuple(
                            search_pair.first,
                            vit_first_label,
                            vit_first_primary_field,
                            vit_first_field_data.ToString(),
                            search_pair.second,
                            vit_second_label,
                            vit_second_primary_field,
                            vit_second_field_data.ToString(),
                            count_common));
    }
    auto core_cost = get_time() - start_time;

    // return
    json output;
    output["cn_list"] = result_list;
    output["num_vertices"] = olapondb.NumVertices();
    output["num_edges"] = olapondb.NumEdges();
    output["prepare_cost"] = prepare_cost;
    output["core_cost"] = core_cost;
    output["total_cost"] = prepare_cost + core_cost;
    response = output.dump();
    return true;
}
