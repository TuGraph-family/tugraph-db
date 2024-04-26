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

#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

const double d = (double)0.85;

void TrustrankCore(OlapBase<Empty>& graph, size_t iterations,
        ParallelVector<double>& curr, std::vector<size_t> trust_list) {
    auto active = graph.AllocVertexSubset();
    active.Fill();
    auto next = graph.AllocVertexArray<double>();
    auto init = graph.AllocVertexArray<double>();
    init.Fill(0.0);
    size_t trustUser_num = trust_list.size();
    if (trustUser_num == 0) {
        std::cout << "Error: there is no trustUser read from trustedUser_dir!" << std::endl;
    }

    for (auto ele : trust_list) {
        init[ele] = (double)1.0 / (double)trustUser_num;;
    }

    double delta = graph.ProcessVertexActive<double>(
        [&](size_t vtx){
            curr[vtx] = init[vtx];
            if (graph.OutDegree(vtx) > 0) {
                curr[vtx] /= graph.OutDegree(vtx);
            }
                return init[vtx];
            },
            active);

    for (size_t i_i = 0; i_i < iterations; i_i++) {
        printf("delta(%ld) = %lf\n", i_i, delta);
        next.Fill(0.0);

        delta = graph.ProcessVertexActive<size_t>(
            [&](size_t vi){
                double sum = (double)0;
                for (auto &edge : graph.InEdges(vi)) {
                    size_t src = edge.neighbour;
                    sum += curr[src];
                }
                next[vi] = sum;
                return 0;
            },
            active);
        if (i_i == iterations - 1) {
            delta = graph.ProcessVertexActive<double>(
                [&](size_t vtx) {
                    next[vtx] = (1 - d) * init[vtx] + d * next[vtx];
                    return 0;
                },
                active);
        } else {
            delta = graph.ProcessVertexActive<double>(
                [&](size_t vtx) {
                    next[vtx] = (1 - d) * init[vtx] + d * next[vtx];
                    if (graph.OutDegree(vtx) > 0) {
                        next[vtx] /= graph.OutDegree(vtx);
                        return fabs(next[vtx] - curr[vtx]) * graph.OutDegree(vtx);
                    }
                    return fabs(next[vtx] - curr[vtx]);
                    },
                active);
            }
            curr.Swap(next);
        }
    std::cout << "ranking process done!" <<std::endl;
}
