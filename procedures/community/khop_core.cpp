/*
 * Copyright 2024 Yingqi Zhao
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lgraph/olap_base.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

size_t k_hop(OlapBase<Empty>& graph, size_t root_vid, ParallelVector<size_t>& result, size_t k) {
    size_t root = root_vid;
    auto active_in = graph.AllocVertexSubset();
    active_in.Add(root);
    auto active_out = graph.AllocVertexSubset();
    auto parent = graph.AllocVertexArray<size_t>();
    parent.Fill(0);
    parent[root] = root;
    size_t num_activations = 1;
    size_t discovered_vertices = 0, j = 0;
    for (size_t ii = 0; ii < k; ii++) {
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                size_t num_activations = 0;
                for (auto& edge : graph.OutEdges(vi)) {
                    size_t dst = edge.neighbour;
                    if (parent[dst] == 0) {
                        auto lock = graph.GuardVertexLock(dst);
                        if (parent[dst] == 0) {
                            parent[dst] = vi;
                            num_activations += 1;
                            active_out.Add(dst);
                            result[j++] = dst;
                        }
                    }
                }
                return num_activations;
            },
            active_in);
        printf("activates(%lu) <= %lu \n", ii+1, num_activations);
        discovered_vertices += num_activations;
        active_in.Swap(active_out);
    }
    return discovered_vertices;
}
