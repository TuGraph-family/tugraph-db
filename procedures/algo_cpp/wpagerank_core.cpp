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

void WPageRankCore(OlapBase<double> &graph, int num_iterations, ParallelVector<double>& curr) {
    auto all_vertices = graph.AllocVertexSubset();
    auto next = graph.AllocVertexArray<double>();
    size_t num_vertices = graph.NumVertices();

    double one_over_n = (double)1 / (double)num_vertices;
    all_vertices.Fill();

    auto sum_out_weight = graph.AllocVertexArray<double>();

    graph.ProcessVertexActive<char>(
        [&](size_t vi) {
            double local_sum = 0.0;
            for (auto &e : graph.OutEdges(vi)) {
                local_sum += e.edge_data;
            }
            sum_out_weight[vi] = local_sum;
            return 0;
        },
        all_vertices);

    double delta = graph.ProcessVertexActive<double>(
        [&](size_t vi) {
            curr[vi] = one_over_n;
            if (graph.OutDegree(vi) > 0) {
                curr[vi] /= sum_out_weight[vi];
            }
            return one_over_n;
        },
        all_vertices);

    double d = (double)0.85;

    for (int ii = 0; ii < num_iterations; ii++) {
        printf("delta(%d)=%lf\n", ii, delta);

        next.Fill((double)0);

        delta = graph.ProcessVertexActive<double>(
            [&](size_t vi) {
                double sum = 0;
                for (auto &edge : graph.InEdges(vi)) {
                    size_t src = edge.neighbour;
                    sum += curr[src] * edge.edge_data;
                }
                next[vi] = sum;
                next[vi] = (1 - d) * one_over_n + d * next[vi];

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
    }
}
