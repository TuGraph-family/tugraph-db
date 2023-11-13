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

size_t BFSCore(OlapBase<Empty>& graph, size_t root_vid, ParallelVector<size_t>& parent) {
    size_t root = root_vid;
    auto active_in = graph.AllocVertexSubset();
    active_in.Add(root);
    auto active_out = graph.AllocVertexSubset();
    parent.Fill((size_t)-1);
    parent[root] = root;

    size_t num_activations = 1;
    size_t discovered_vertices = 0;
    for (int ii = 0; num_activations != 0; ii++) {
        printf("activates(%d) <= %lu\n", ii, num_activations);
        discovered_vertices += num_activations;
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                size_t num_activations = 0;
                for (auto& edge : graph.OutEdges(vi)) {
                    size_t dst = edge.neighbour;
                    if (parent[dst] == (size_t)-1) {
                        auto lock = graph.GuardVertexLock(dst);
                        if (parent[dst] == (size_t)-1) {
                            parent[dst] = vi;
                            num_activations += 1;
                            active_out.Add(dst);
                        }
                    }
                }
                return num_activations;
            },
            active_in);
        active_in.Swap(active_out);
    }
    return discovered_vertices;
}
