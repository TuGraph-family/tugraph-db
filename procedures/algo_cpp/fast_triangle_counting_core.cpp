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

int count_vector_common(ParallelVector<size_t> &num, std::vector<size_t> &list_a,
                        std::vector<size_t> &list_b, size_t a, size_t b) {
    int local_count = 0;
    size_t index_a = 0;
    size_t index_b = 0;
    size_t pre_a = -1;
    size_t pre_b = -1;
    size_t threshold = (a < b) ? a : b;
    while (index_a < list_a.size() && index_b < list_b.size() && list_a[index_a] < threshold &&
           list_b[index_b] < threshold) {
        if (pre_a == list_a[index_a]) {
            index_a++;
            continue;
        }
        if (pre_b == list_b[index_b]) {
            index_b++;
            continue;
        }
        if (list_a[index_a] < list_b[index_b]) {
            pre_a = list_a[index_a];
            index_a++;
        } else if (list_a[index_a] > list_b[index_b]) {
            pre_b = list_b[index_b];
            index_b++;
        } else {
            pre_a = list_a[index_a];
            pre_b = list_b[index_b];
            local_count++;
            index_a++;
            index_b++;
            write_add(&num[a], 1);
            write_add(&num[b], 1);
            write_add(&num[pre_a], 1);
        }
    }
    return local_count;
}

size_t FastTriangleCore(OlapBase<Empty> &graph, ParallelVector<size_t> &num_triangle) {
    std::vector<int> orders;
    auto index_order = graph.AllocVertexArray<size_t>();
    index_order.Fill(0);
    int num_vertices = graph.NumVertices();
    orders.resize(num_vertices);

    graph.ProcessVertexInRange<int>(
        [&](size_t v) {
            orders[v] = v;
            return 0;
        },
        0, num_vertices);
    auto compare_index = [&](const size_t &a, const size_t &b) -> bool {
        return (graph.OutDegree(a) > graph.OutDegree(b));
    };
    std::sort(orders.begin(), orders.end(), compare_index);

    graph.ProcessVertexInRange<int>(
        [&](size_t v) {
            index_order[orders[v]] = v;
            return 0;
        },
        0, num_vertices);

    auto edge_num = graph.AllocVertexArray<int>();
    edge_num.Fill(0);
    graph.ProcessVertexInRange<int>(
        [&](size_t v) {
            size_t v_order = index_order[v];
            int local_edges_num = 0;
            for (auto &e : graph.OutEdges(v)) {
                size_t neighbour_order = index_order[e.neighbour];
                if (v_order > neighbour_order) {
                    local_edges_num++;
                }
            }
            edge_num[v] = local_edges_num;
            return local_edges_num;
        },
        0, num_vertices);

    std::vector<std::vector<size_t> > edges(num_vertices);
    for (int i = 0; i < num_vertices; i++) {
        edges[i].resize(edge_num[orders[i]]);
    }

    graph.ProcessVertexInRange<int>(
        [&](size_t i) {
            int vid = orders[i];
            int local_index = 0;
            for (auto &e : graph.OutEdges(vid)) {
                size_t neighbour_index = index_order[e.neighbour];
                if (neighbour_index < i) {
                    edges[i][local_index] = neighbour_index;
                    local_index++;
                }
            }
            return 0;
        },
        0, num_vertices);

    graph.ProcessVertexInRange<int>(
        [&](size_t i) {
            std::sort(edges[i].begin(), edges[i].end());
            return 0;
        },
        0, num_vertices);
    printf("sorted\n");

    size_t discovered_triangles = graph.ProcessVertexInRange<size_t>(
        [&](size_t v) {
            size_t local_count = 0;
            size_t pre = -1;
            for (auto dst : edges[v]) {
                if (pre == dst) {
                    continue;
                } else {
                    pre = dst;
                }
                local_count += count_vector_common(num_triangle, edges[v], edges[dst], v, dst);
            }
            return local_count;
        },
        0, num_vertices);
    printf("discovered %zu triangles\n", discovered_triangles);

    return discovered_triangles;
}
