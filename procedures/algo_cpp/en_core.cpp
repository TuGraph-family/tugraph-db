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

size_t ENCore(OlapBase<Empty>& graph, size_t root, int hop_value) {
    auto active_in = graph.AllocVertexSubset();
    auto active_out = graph.AllocVertexSubset();
    auto discoverd = graph.AllocVertexArray<int>();

    size_t num_activations = 1;
    size_t total_num = 1;

    active_in.Add(root);
    discoverd.Fill((size_t)0);
    discoverd[root] = 1;

    int ii = 0;
    while (num_activations > 0 && ii < hop_value) {
        // printf("activates(%d) <= %lu\n", ii, num_activations);
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                size_t num_activations = 0;
                for (auto& edge : graph.OutEdges(vi)) {
                    size_t dst = edge.neighbour;
                    if (discoverd[dst] == (size_t)0) {
                        auto lock = graph.GuardVertexLock(dst);
                        if (discoverd[dst] == (size_t)0) {
                            discoverd[dst] = 1;
                            num_activations += 1;
                            active_out.Add(dst);
                        }
                    }
                }
                return num_activations;
            },
            active_in);
        active_in.Swap(active_out);
        printf("active(%d)=%lu\n", ii++, num_activations);
        total_num += num_activations;
    }
    return total_num;
}
