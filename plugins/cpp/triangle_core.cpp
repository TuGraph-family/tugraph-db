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

size_t TriangleCountCommon(ParallelVector<size_t>& num_triangle,
    AdjList<Empty> &list_a, AdjList<Empty> &list_b, size_t a, size_t b) {
    size_t local_count = 0;
    AdjUnit<Empty> *ptr_a = list_a.begin();
    AdjUnit<Empty> *ptr_b = list_b.begin();
    size_t pre_a = -1;
    size_t pre_b = -1;
    size_t threshold = (a < b) ? a : b;
    while (ptr_a != list_a.end() && ptr_b != list_b.end()
        && ptr_a->neighbour < threshold && ptr_b->neighbour < threshold) {
        if (pre_a == ptr_a->neighbour) {
            ptr_a++;
            continue;
        }
        if (pre_b == ptr_b->neighbour) {
            ptr_b++;
            continue;
        }
        if (ptr_a->neighbour < ptr_b->neighbour) {
            pre_a = ptr_a->neighbour;
            ptr_a++;
        } else if (ptr_a->neighbour > ptr_b->neighbour) {
            pre_b = ptr_b->neighbour;
            ptr_b++;
        } else {
            pre_a = ptr_a->neighbour;
            pre_b = ptr_b->neighbour;
            write_add(&num_triangle[a], (size_t)1);
            write_add(&num_triangle[b], (size_t)1);
            write_add(&num_triangle[pre_a], (size_t)1);
            local_count++;
            ptr_a++;
            ptr_b++;
        }
    }
    return local_count;
}

int triangle_compare(const void *a, const void *b) {
    const AdjUnit<Empty> *ptr_a = (const AdjUnit<Empty> *)a;
    const AdjUnit<Empty> *ptr_b = (const AdjUnit<Empty> *)b;
    return ptr_a->neighbour > ptr_b->neighbour ? 1 : -1;
}

size_t TriangleCore(OlapBase<Empty>& graph, ParallelVector<size_t>& num_triangle) {
    auto active = graph.AllocVertexSubset();
    active.Fill();
    graph.ProcessVertexActive<size_t>(
            [&](size_t vtx) {
                auto edges = graph.OutEdges(vtx);
                qsort(edges.begin(), graph.OutDegree(vtx),
                        sizeof(AdjUnit<Empty>), triangle_compare);
                return 1;
            },
            active);
    printf("sorted\n");
    size_t discovered_triangles = graph.ProcessVertexActive<size_t>(
            [&](size_t src) {
                size_t local_count = 0;
                AdjList<Empty> src_adj = graph.OutEdges(src);
                size_t pre = -1;
                for (auto edge : src_adj) {
                    size_t dst = edge.neighbour;
                    if (pre == dst) {
                        continue;
                    } else {
                        pre = dst;
                    }
                    if (src < dst) {
                        AdjList<Empty> neighbour_adj = graph.OutEdges(dst);
                        local_count += TriangleCountCommon(num_triangle,
                                        src_adj, neighbour_adj, src, dst);
                    }
                }
                return local_count;
            },
            active);
    printf("discovered %lu triangles\n", discovered_triangles);
    return discovered_triangles;
}
