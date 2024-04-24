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

using json = nlohmann::json;
using namespace lgraph_api;
using namespace lgraph_api::olap;

#define __ADD_DANGLING__

void PageRankCore(OlapBase<Empty>& graph, int num_iterations, ParallelVector<double>& curr) {
    auto all_vertices = graph.AllocVertexSubset();
    all_vertices.Fill();
    auto next = graph.AllocVertexArray<double>();
    size_t num_vertices = graph.NumVertices();

    double one_over_n = (double)1 / num_vertices;
    double delta = 1;
    double dangling = graph.ProcessVertexActive<double>(
        [&](size_t vi) {
            curr[vi] = one_over_n;
            if (graph.OutDegree(vi) > 0) {
                curr[vi] /= graph.OutDegree(vi);
                return 0.0;
            }
#ifdef __ADD_DANGLING__
            return one_over_n;
#else
            return 0.0;
#endif
        },
        all_vertices);
    dangling /= num_vertices;

    double d = (double)0.85;
    for (int ii = 0; ii < num_iterations; ii++) {
        printf("delta(%d)=%lf\n", ii, delta);
        next.Fill((double)0);
        delta = graph.ProcessVertexActive<double>(
            [&](size_t vi) {
                double sum = 0;
                for (auto& edge : graph.InEdges(vi)) {
                    size_t src = edge.neighbour;
                    sum += curr[src];
                }
                next[vi] = sum;
                next[vi] = (1 - d) * one_over_n + d * next[vi] + d * dangling;
                if (ii == num_iterations - 1) {
                    return (double)0;
                } else {
                    if (graph.OutDegree(vi) > 0) {
                        next[vi] /= graph.OutDegree(vi);
                        return fabs(next[vi] - curr[vi]) * graph.OutDegree(vi);
                    } else {
                        return fabs(next[vi] - curr[vi]);
                    }
                }
            },
            all_vertices);
        curr.Swap(next);

#ifdef __ADD_DANGLING__
        dangling = graph.ProcessVertexActive<double>(
            [&](size_t vi) {
                if (graph.OutDegree(vi) == 0)
                    return curr[vi];
                return 0.0;
            },
            all_vertices);
        dangling /= num_vertices;
#endif
    }
}

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
