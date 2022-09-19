/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

void WCCCore(OlapBase<Empty>& graph, ParallelVector<size_t>& label) {
    auto active_in = graph.AllocVertexSubset();
    active_in.Fill();
    auto active_out = graph.AllocVertexSubset();
    size_t num_activations = graph.ProcessVertexActive<size_t>(
        [&](size_t vi) {
            label[vi] = vi;
            return 1;
        },
        active_in);

    for (int ii = 0; num_activations != 0; ii++) {
        printf("activates(%d) <= %lu\n", ii, num_activations);
        active_out.Clear();
        num_activations = graph.ProcessVertexActive<size_t>(
            [&](size_t src) {
                size_t num_activations = 0;
                for (auto& edge : graph.OutEdges(src)) {
                    size_t dst = edge.neighbour;
                    if (label[src] < label[dst]) {
                        auto lock = graph.GuardVertexLock(dst);
                        if (label[src] < label[dst]) {
                            label[dst] = label[src];
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
}

