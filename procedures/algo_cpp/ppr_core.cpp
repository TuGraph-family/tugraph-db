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

// #define __ADD_DANGLING__
const double d = 0.85;

void PPRCore(OlapBase<Empty> & graph, ParallelVector<double>& curr,
                        size_t root, size_t iterations) {
    curr.Fill(0.0);
    curr[root] = 1.0;
    if (graph.OutDegree(root) == 0) {
        printf("error: root vertex %lu has no out edge.\n", root);
        return;
    }

    auto next = graph.AllocVertexArray<double>();
    auto active_all = graph.AllocVertexSubset();
    active_all.Fill();
    double delta = 1.0;

    curr[root] = 1.0 / graph.OutDegree(root);
    for (size_t iter_i = 0; iter_i < iterations; iter_i++) {
        next.Fill(0.0);

        graph.ProcessVertexActive<size_t>(
            [&] (size_t src) {
                for (auto & edge : graph.OutEdges(src)) {
                    size_t dst = edge.neighbour;
                    write_add(&next[dst], curr[src]);
                }
                return 0;
            },
            active_all);

        double dangling = 0;
#ifdef __ADD_DANGLING__
        size_t num_vertices = graph.NumVertices();
        dangling = graph.ProcessVertexActive<double>(
                [&] (size_t vi) {
                    if (graph.OutDegree(vi) == 0) {
                        return curr[vi] / num_vertices;
                    }
                    return (double)0;
                },
                active_all);
#endif   // __ADD_DANGLING__

        if (iter_i == iterations - 1) {
            delta = graph.ProcessVertexActive<double>(
                    [&] (size_t vi) {
                        next[vi] = d * next[vi];
                        if (vi == root) {
                            next[vi] += 1.0 - d + d * dangling;
                        }
                        return fabs(next[vi] - curr[vi] * graph.OutDegree(vi));
                    },
                    active_all);
        } else {
            delta = graph.ProcessVertexActive<double>(
                    [&] (size_t vi) {
                        next[vi] = d * next[vi];
                        if (vi == root) {
                            next[vi] += 1.0 - d + d * dangling;
                        }
                        if (graph.OutDegree(vi) > 0) {
                            next[vi] /= graph.OutDegree(vi);
                            return fabs(next[vi] - curr[vi]) * graph.OutDegree(vi);
                        }
                        return fabs(next[vi] - curr[vi]);
                    },
                    active_all);
        }
        printf("delta(%ld) = %lf\n", iter_i, delta);
        curr.Swap(next);
    }
    printf("pr[%lu] = %lf\n", root, curr[root]);
}
