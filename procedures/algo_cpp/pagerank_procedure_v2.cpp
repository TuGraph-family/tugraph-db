/**
* Copyright 2024 AntGroup CO., Ltd.
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

#include <cstdlib>
#include "lgraph/lgraph.h"
#include "lgraph/lgraph_types.h"
#include "lgraph/lgraph_result.h"
#include "tools/json.hpp"
#include "./algo.h"

using json = nlohmann::json;
using namespace lgraph_api;
using namespace lgraph_api::olap;

extern "C" LGAPI bool GetSignature(SigSpec &sig_spec) {
    sig_spec.input_list = {
        {.name = "num_iteration", .index = 0, .type = LGraphType::INTEGER},
    };
    sig_spec.result_list = {
        {.name = "node", .index = 0, .type = LGraphType::NODE},
        {.name = "weight", .index = 1, .type = LGraphType::DOUBLE}
    };
    return true;
}

extern "C" LGAPI bool ProcessInTxn(Transaction &txn,
                                   const std::string &request,
                                   Result &response) {
    double start_time = get_time();
    int64_t num_iteration;
    LOG_DEBUG() << "input: " << request;
    try {
        json input = json::parse(request);
        num_iteration = input["num_iteration"].get<int64_t>();
    } catch (std::exception &e) {
        response.ResetHeader({
            {"errMsg", LGraphType::STRING}
        });
        response.MutableRecord()->Insert(
            "errMsg",
            FieldData::String(std::string("error parsing json: ") + e.what()));
        return false;
    }

    OlapOnDB<Empty> graph(txn, SNAPSHOT_PARALLEL);
    auto prepare_cost = get_time() - start_time;
    start_time = get_time();
    auto pr = graph.AllocVertexArray<double>();
    PageRankCore(graph, num_iteration, pr);
    size_t max_pr_vi = graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) { return vi; }, 0, graph.NumVertices(), 0,
        [&](size_t a, size_t b) { return pr[a] > pr[b] ? a : b; });
    LOG_INFO() << FMA_FMT("max_pr: pr[{}] = {}", max_pr_vi, pr[max_pr_vi]);
    auto core_cost = get_time() - start_time;

    start_time = get_time();
    response.ResetHeader({
        {"node", LGraphType::NODE},
        {"weight", LGraphType::DOUBLE},
    });

    response.Resize(pr.Size());
    graph.ProcessVertexInRange<char>(
        [&] (size_t vid) {
            auto r = response.At(vid);
            r->InsertVertexByID("node", graph.OriginalVid(vid));
            r->Insert("weight", FieldData::Double(pr[vid]));
            return 0;
        },
        0, pr.Size());
    auto output_cost = get_time() - start_time;

    LOG_DEBUG() << "prepare_cost: " << prepare_cost << " (s)\n"
                << "core_cost: " << core_cost << " (s)\n"
                << "output_cost: " << output_cost << " (s)\n"
                << "response.Size: " << response.Size();
    return true;
}
