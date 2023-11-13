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

void MISCore(OlapBase<Empty> &graph, ParallelVector<bool> &mis, size_t &mis_size) {
    mis.Fill(0);
    size_t num_vertices = graph.NumVertices();
    auto label_curr = graph.AllocVertexArray<size_t>();
    auto label_next = graph.AllocVertexArray<size_t>();
    graph.ProcessVertexInRange<int>(
        [&](size_t vtx) {
            label_curr[vtx] = vtx;
            label_next[vtx] = vtx;
            return 0;
        },
        (size_t)0, num_vertices);
    size_t total_set_num = 0;
    size_t active_num = 1;
    int i_i = 0;
    while (active_num != 0) {
        active_num = graph.ProcessVertexInRange<size_t>(
            [&](size_t dst) {
                auto edges = graph.OutEdges(dst);
                for (auto &edge : edges) {
                    size_t src = edge.neighbour;
                    if (label_curr[src] <= label_curr[dst]) {
                        return (size_t)0;
                    }
                }
                label_next[dst] = (size_t)-1;
                mis[dst] = true;
                for (auto &edge : edges) {
                    write_max(&label_next[edge.neighbour], (size_t)-1);
                }
                return (size_t)1;
            },
            0, num_vertices);
        total_set_num += active_num;
        if (active_num > 0) {
            graph.ProcessVertexInRange<size_t>(
                [&](size_t vtx) {
                    label_curr[vtx] = label_next[vtx];
                    return (size_t)0;
                },
                0, num_vertices);
        }
        printf("active(%d)=%zu\n", i_i++, active_num);
    }
    printf("|mis|=%zu\n", total_set_num);
    mis_size = total_set_num;
}
