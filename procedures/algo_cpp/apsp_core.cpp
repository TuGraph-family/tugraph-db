/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

void APSPCore(OlapBase<double>& graph, std::vector< std::tuple<size_t, size_t, double> > result,
                std::tuple<size_t, size_t, double>& max_tuple) {
    auto active_in = graph.AllocVertexSubset();
    auto active_out = graph.AllocVertexSubset();
    ParallelVector<double> distance = graph.AllocVertexArray<double>();
    for (size_t v_i = 0; v_i < graph.NumVertices(); v_i++) {
//        printf("process has done %6.2lf%%\r", (double)(v_i + 1) / graph.NumVertices() * 100);
        active_in.Clear();
        active_in.Add(v_i);
        distance.Fill((double)1e10);
        distance[v_i] = 0.0;
        size_t num_activations = 1;
        for (int ii = 0; num_activations != 0; ii++) {
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
        for (size_t s_i = 0; s_i < graph.NumVertices(); s_i++) {
            if ((distance[s_i] >= (double)1e10) || (v_i == s_i)) continue;
            result.push_back(std::make_tuple(v_i, s_i, distance[s_i]));
        }
    }
    for (auto ele : result) {
        if (std::get<2>(ele) > std::get<2>(max_tuple)) {
            max_tuple = ele;
        }
    }
    printf("max_distance is distance[%lu,%lu] = %lf\n",
            std::get<0>(max_tuple), std::get<1>(max_tuple), std::get<2>(max_tuple));
}
