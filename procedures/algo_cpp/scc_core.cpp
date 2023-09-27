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

void SCCCore(OlapBase<Empty> & graph, ParallelVector<size_t>& label) {
    auto active_in = graph.AllocVertexSubset();
    active_in.Fill();
    auto active_out = graph.AllocVertexSubset();
    auto deleted = graph.AllocVertexSubset();
    auto visited = graph.AllocVertexSubset();

    size_t num_vertices = graph.NumVertices();
    // initialize labels and locate the pivot
    size_t pivot = graph.ProcessVertexActive<size_t>(
        [&](size_t vi) {
            label[vi] = vi;
            return vi;
        },
        active_in, 0,
        [&](size_t a, size_t b) {
            return graph.OutDegree(a) * graph.InDegree(a) >
                           graph.OutDegree(b) * graph.InDegree(b)
                       ? a
                       : b;
        });
    printf("use %lu as the pivot with out/in degree = (%lu, %lu)\n", pivot,
           graph.OutDegree(pivot), graph.InDegree(pivot));
    // use the pivot to find the giant component via BFS
    size_t active_vertices = 1;
    active_in.Clear();
    active_in.Add(pivot);
    visited.Clear();
    visited.Add(pivot);
    while (active_vertices > 0) {
        active_out.Clear();
        graph.ProcessVertexActive<size_t>(
            [&](size_t src) {
                size_t active_vertices = 0;
                for (auto& edge : graph.OutEdges(src)) {
                    size_t dst = edge.neighbour;
                    if (!visited.Has(dst) && !active_out.Has(dst) && active_out.Add(dst)) {
                        active_vertices += 1;
                    }
                }
                return active_vertices;
            },
            active_in);
        active_vertices = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                visited.Add(vi);
                return 1;
            },
            active_out);
        active_in.Swap(active_out);
    }
    graph.Transpose();
    active_vertices = 1;
    active_in.Clear();
    active_in.Add(pivot);
    deleted.Clear();
    deleted.Add(pivot);
    while (active_vertices > 0) {
        active_out.Clear();
        graph.ProcessVertexActive<size_t>(
            [&](size_t src) {
                size_t active_vertices = 0;
                for (auto& edge : graph.OutEdges(src)) {
                    size_t dst = edge.neighbour;
                    if (!deleted.Has(dst) && visited.Has(dst) && !active_out.Has(dst) &&
                        active_out.Add(dst)) {
                        active_vertices += 1;
                    }
                }
                return active_vertices;
            },
            active_in);
        active_vertices = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                deleted.Add(vi);
                return 1;
            },
            active_out);
        active_in.Swap(active_out);
    }
    // use the smallest identifier as the representative
    for (size_t vi = 0; vi < num_vertices; vi++) {
        if (deleted.Has(vi)) {
            pivot = vi;
            break;
        }
    }
    size_t remained_vertices = num_vertices;
    remained_vertices -= graph.ProcessVertexActive<size_t>(
        [&](size_t vi) {
            label[vi] = pivot;
            return 1;
        },
        deleted);
    while (remained_vertices > 0) {
        // printf("remained_vertices = %lu\n", remained_vertices);
        graph.Transpose();
        active_in.Fill();
        active_out.Clear();
        visited.Clear();
        active_vertices = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                if (!deleted.Has(vi)) {
                    label[vi] = vi;
                    active_out.Add(vi);
                    visited.Add(vi);
                    return 1;
                }
                return 0;
            },
            active_in);
        while (active_vertices > 0) {
            active_in.Swap(active_out);
            active_out.Clear();
            active_vertices = graph.ProcessVertexActive<size_t>(
                [&](size_t src) {
                    size_t active_vertices = 0;
                    for (auto& edge : graph.OutEdges(src)) {
                        size_t dst = edge.neighbour;
                        if (deleted.Has(dst)) continue;
                        if (label[src] < label[dst]) {
                            auto lock = graph.GuardVertexLock(dst);
                            if (label[src] < label[dst]) {
                                label[dst] = label[src];
                                active_vertices += 1;
                                active_out.Add(dst);
                            }
                        }
                    }
                    return active_vertices;
                },
                active_in);
        }
        graph.Transpose();
        active_in.Swap(visited);
        visited.Clear();
        active_out.Clear();
        active_vertices = graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                if (label[vi] == vi) {
                    active_out.Add(vi);
                    visited.Add(vi);
                    return 1;
                }
                return 0;
            },
            active_in);
        while (active_vertices > 0) {
            active_in.Swap(active_out);
            active_out.Clear();
            graph.ProcessVertexActive<size_t>(
                [&](size_t src) {
                    size_t active_vertices = 0;
                    for (auto& edge : graph.OutEdges(src)) {
                        size_t dst = edge.neighbour;
                        if (deleted.Has(dst)) continue;
                        if (label[dst] == label[src] && !visited.Has(dst) && !active_out.Has(dst) &&
                            active_out.Add(dst)) {
                            active_vertices += 1;
                        }
                    }
                    return active_vertices;
                },
                active_in);
            active_vertices = graph.ProcessVertexActive<size_t>(
                [&](size_t vi) {
                    visited.Add(vi);
                    return 1;
                },
                active_out);
        }
        remained_vertices -= graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                deleted.Add(vi);
                return 1;
            },
            visited);
    }
}
