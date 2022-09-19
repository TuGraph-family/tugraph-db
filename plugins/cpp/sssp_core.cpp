/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

void SSSPCore(OlapBase<double>& graph, size_t root, ParallelVector<double>& distance) {
    auto active_in = graph.AllocVertexSubset();
    active_in.Add(root);
    std::cout<< "root:" << root << std::endl;
    auto active_out = graph.AllocVertexSubset();
    distance.Fill((double)2e10);

    distance[root] = 0.0;
    size_t num_activations = 1;
    for (int ii = 0; num_activations != 0; ii++) {
        printf("activates(%d) <= %lu\n", ii, num_activations);
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t v_i) {
                size_t local_num_activations = 0;
                for (auto& edge : graph.OutEdges(v_i)) {
                    size_t dst = edge.neighbour;
                    double update_distance = distance[v_i] + edge.edge_data;
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
