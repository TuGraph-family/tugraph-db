/**
 * Copyright 2023 AntGroup CO., Ltd.
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

#include <iostream>
#include <vector>

#include "fma-common/string_util.h"
#include "lgraph/olap_on_db.h"
#include "tools/json.hpp"
#include "./algo.h"

using json = nlohmann::json;
using namespace lgraph_api;

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    double start_time;

    // prepare
    start_time = get_time();
    std::string feature = "feature";
    std::string feature_float = "feature_float";
    size_t stop_number = 1024;
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(feature, "feature", input);
        parse_from_json(feature_float, "feature_float", input);
        parse_from_json(stop_number, "stop_number", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }
    auto txn = db.CreateReadTxn();
    auto prepare_cost = get_time() - start_time;

    start_time = get_time();
    auto num_vertices = txn.GetNumVertices();
    auto task_ctx = GetThreadContext();
    std::vector<FieldData> feature_data_list(num_vertices);
    auto worker = Worker::SharedWorker();
    worker->Delegate([&]() {
#pragma omp parallel
{
    auto num_threads = omp_get_num_threads();
    auto thread_id = omp_get_thread_num();
    size_t begin = num_vertices / num_threads * thread_id;
    size_t end = num_vertices / num_threads * (thread_id + 1);
    if (thread_id == num_threads - 1) {
        end = num_vertices;
    }
    auto local_txn = db.ForkTxn(txn);
    auto vit = local_txn.GetVertexIterator();
    for (size_t i = begin; i < end; i++) {
        if (i % stop_number == 0 && ShouldKillThisTask(task_ctx)) break;
        vit.Goto(i);
        if (vit.IsValid()) {
            auto&& feature_string = vit.GetField(feature).ToString();
            auto&& feature_vector = fma_common::Split(feature_string, ",");
            float* feature_value = new float[feature_vector.size()];
            int idx = 0;
            for (auto& fea : feature_vector) {
                feature_value[idx] = std::stof(fea);
                idx++;
            }
            FieldData feature_data((char*)feature_value, feature_vector.size() * sizeof(float));
            feature_data_list[i] = feature_data;
            delete[] feature_value;
        }
    }
    local_txn.Abort();
}
    });
    txn.Abort();
    std::cout << "feature field has processed completed." << std::endl;
    auto get_field_time = get_time() - start_time;
    start_time = get_time();
    auto write_txn = db.CreateWriteTxn();
    auto vit1 = write_txn.GetVertexIterator();
    for (size_t i = 0; i < num_vertices; i++) {
        vit1.Goto(i);
        if (vit1.IsValid()) {
            vit1.SetField(feature_float, feature_data_list[i]);
        }
    }
    write_txn.Commit();
    auto set_field_time = get_time() - start_time;

    json output;
    output["prepare_cost"] = prepare_cost;
    output["get_field_cost"] = get_field_time;
    output["set_field_cost"] = set_field_time;
    output["total_cost"] = prepare_cost + get_field_time + set_field_time;
    response = output.dump();
    return true;
}
