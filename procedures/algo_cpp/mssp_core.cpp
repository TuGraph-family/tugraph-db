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

void MSSPCore(OlapBase<double>& graph,
        std::vector<size_t> roots, ParallelVector<double>& distance) {
    auto active_in = graph.AllocVertexSubset();
    for (auto ele : roots) {
        active_in.Add(ele);
    }

    auto active_out = graph.AllocVertexSubset();
    distance.Fill((double)2e10);

    graph.ProcessVertexActive<size_t>(
        [&](size_t v_i) {
            distance[v_i] = 0.0;
            return 0;
        },
        active_in);
    size_t num_activations = 1;
    for (int ii = 0; num_activations != 0; ii++) {
        printf("activates(%d) <= %lu\n", ii, num_activations);
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t v_i) {
                size_t local_num_activations = 0;
                for (auto& edge : graph.OutEdges(v_i)) {
                    size_t dst = edge.neighbour;
                    float update_distance = distance[v_i] + edge.edge_data;
                    auto lock = graph.GuardVertexLock(dst);
                    if (distance[dst] > update_distance) {
                        distance[dst] = update_distance;
                        active_out.Add(dst);
                        local_num_activations += 1;
                    }
                }
                return local_num_activations;
            },
            active_in);
        active_in.Swap(active_out);
    }
}
