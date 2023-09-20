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

#define __ADD_DANGLING__

void PageRankCore(OlapBase<Empty>& graph, int num_iterations, ParallelVector<double>& curr) {
    auto all_vertices = graph.AllocVertexSubset();
    all_vertices.Fill();
    auto next = graph.AllocVertexArray<double>();
    size_t num_vertices = graph.NumVertices();

    double one_over_n = (double)1 / num_vertices;
    double delta = 1;
    double dangling = graph.ProcessVertexActive<double>(
        [&](size_t vi) {
            curr[vi] = one_over_n;
            if (graph.OutDegree(vi) > 0) {
                curr[vi] /= graph.OutDegree(vi);
                return 0.0;
            }
#ifdef __ADD_DANGLING__
            return one_over_n;
#else
            return 0.0;
#endif
        },
        all_vertices);
    dangling /= num_vertices;

    double d = (double)0.85;
    for (int ii = 0; ii < num_iterations; ii++) {
        printf("delta(%d)=%lf\n", ii, delta);
        next.Fill((double)0);
        delta = graph.ProcessVertexActive<double>(
            [&](size_t vi) {
                double sum = 0;
                for (auto& edge : graph.InEdges(vi)) {
                    size_t src = edge.neighbour;
                    sum += curr[src];
                }
                next[vi] = sum;
                next[vi] = (1 - d) * one_over_n + d * next[vi] + d * dangling;
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

#ifdef __ADD_DANGLING__
        dangling = graph.ProcessVertexActive<double>(
            [&](size_t vi) {
                if (graph.OutDegree(vi) == 0)
                    return curr[vi];
                return 0.0;
            },
            all_vertices);
        dangling /= num_vertices;
#endif
    }
}
