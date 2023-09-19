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

void CLCECore(OlapBase<double>& graph, size_t samples, ParallelVector<double>& score,
                   ParallelVector<size_t>& path_num) {
    graph.Transpose();
    size_t vertices = graph.NumVertices();
    score.Fill(0.0);
    path_num.Fill(0);
    auto distance = graph.AllocVertexArray<double>();
    auto active_in = graph.AllocVertexSubset();
    auto active_out = graph.AllocVertexSubset();
    unsigned int seed = 0;

    for (size_t s_i = 0; s_i < samples; s_i++) {
        size_t root = rand_r(&seed) % vertices;
        size_t active_vertices = 1;
        // printf("clce caculating process has finished %6.2lf%%\r", 100. * (s_i + 1) / samples);
        // fflush(stdout);

        active_in.Clear();
        active_in.Add(root);
        distance.Fill(1e10);
        distance[root] = (double)0;
        while (active_vertices > 0) {
            active_out.Clear();
            active_vertices = graph.ProcessVertexActive<size_t>(
                [&](size_t vi) {
                    size_t activated = 0;
                    for (auto & edge : graph.OutEdges(vi)) {
                        size_t dst = edge.neighbour;
                        double relax_dist = distance[vi] + edge.edge_data;
                        graph.GuardVertexLock(dst);
                        if (relax_dist < distance[dst]) {
                            distance[dst] = relax_dist;
                            active_out.Add(dst);
                            activated += 1;
                        }
                    }
                    return activated;
                },
                active_in);
            active_in.Swap(active_out);
        }

        graph.ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                if (distance[vi] >= 1e10 || vi == root) {
                    return 0;
                }
                score[vi] += distance[vi];
                path_num[vi] += 1;
                return 0;
            },
            0, vertices);
    }

    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            if (score[vi] != 0.0) {
                score[vi] = (double)path_num[vi] / score[vi];
            }
            return 0;
        },
        0, vertices);
}
