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

size_t count_common(ParallelVector<size_t>& num_triangle,
                    AdjList<Empty>& list_a, AdjList<Empty>& list_b, size_t a, size_t b) {
    size_t local_count = 0;
    AdjUnit<Empty>* ptr_a = list_a.begin();
    AdjUnit<Empty>* ptr_b = list_b.begin();
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

int compare(const void *a, const void *b) {
    const AdjUnit<Empty> *ptr_a = (const AdjUnit<Empty> *)a;
    const AdjUnit<Empty> *ptr_b = (const AdjUnit<Empty> *)b;
    return (ptr_a->neighbour - ptr_b->neighbour);
}

double LCCCore(OlapBase<Empty>& graph, ParallelVector<double>& score) {
    score.Fill(0.0);
    size_t num_vertices = graph.NumVertices();
    auto num_triangle = graph.AllocVertexArray<size_t>();
    num_triangle.Fill(0);

    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            auto edges = graph.OutEdges(vi);
            qsort(edges.begin(), graph.OutDegree(vi), sizeof(AdjUnit<Empty>), compare);
            return 1;
        },
        0, num_vertices);

    graph.ProcessVertexInRange<size_t>(
        [&](size_t src) {
            if (graph.OutDegree(src) < 2) {return (size_t)0;}
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
                        local_count += count_common(num_triangle, src_adj, neighbour_adj, src, dst);
                    }
                }
                return local_count;
        },
        0, num_vertices);

    graph.ProcessVertexInRange<size_t>(
        [&](size_t v) {
            size_t valid_degree = 0;
            size_t prev_nbr = (size_t)-1;
            for (auto edge : graph.OutEdges(v)) {
                auto nbr = edge.neighbour;
                if (nbr == prev_nbr || nbr == v) {
                    continue;
                } else {
                    valid_degree += 1;
                    prev_nbr = nbr;
                }
            }
            if (valid_degree < 2) return 0;
            score[v] = 2.0 * num_triangle[v] / (valid_degree) / (valid_degree - 1);
            return 0;
        },
        0, num_vertices);

    double sum_lcc = graph.ProcessVertexInRange<double>(
        [&] (size_t v) {
            return score[v];
        },
        0, num_vertices);
    // std::cout << "average_lcc = " << sum_lcc / num_vertices << std::endl;
    return sum_lcc / num_vertices;
}
