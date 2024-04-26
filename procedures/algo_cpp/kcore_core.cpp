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

size_t KCoreCore(OlapBase<Empty>& graph, ParallelVector<bool>& result, size_t value_k) {
    auto degree = graph.AllocVertexArray<size_t>();
    auto core_num = graph.AllocVertexArray<size_t>();
    auto frontier = graph.AllocVertexSubset();
    auto to_remove = graph.AllocVertexSubset();
    auto remaining = graph.AllocVertexSubset();
    frontier.Fill();

    size_t num_active_vertices = graph.ProcessVertexActive<size_t>(
            [&] (size_t vi) {
                core_num[vi] = 0;
                degree[vi] = graph.OutDegree(vi);
                return 1;
            },
            frontier);
    std::cout << "vertices has out_edges: " << num_active_vertices << std::endl;
    size_t num_vertices_removed = 0;
    size_t num_vertices_to_remove = graph.NumVertices();
    while (num_vertices_to_remove > 0) {
        to_remove.Clear();
        remaining.Clear();
        num_vertices_to_remove = graph.ProcessVertexActive<size_t>(
                [&] (size_t vi) {
                    if (degree[vi] < value_k) {
                        degree[vi] = 0;
                        to_remove.Add(vi);
                        return 1;
                    } else {
                        remaining.Add(vi);
                        return 0;
                    }
                },
                frontier);
        num_vertices_removed += num_vertices_to_remove;
        printf("num_vertices_removed = %lu\n", num_vertices_removed);
        fflush(stdout);

        if (num_vertices_to_remove == 0) {
            break;
        }
        frontier.Swap(remaining);
        graph.ProcessVertexActive<size_t>(
            [&] (size_t vi) {
                for (auto edge : graph.OutEdges(vi)) {
                    size_t dst = edge.neighbour;
//                        graph.lock_vertex(dst);
//                        degree[dst] -= 1;
//                        graph.unlock_vertex(dst);
                    write_sub(&degree[dst], (size_t)1);
                }
                return 0;
            },
            to_remove);
    }
    result.Fill(false);

    size_t num_result_vertices = graph.ProcessVertexActive<size_t>(
            [&] (size_t vi) {
                result[vi] = true;
                return 1;
            },
            remaining);
    printf("number of %ld-core vertices: %lu\n", value_k, num_result_vertices);
    return num_result_vertices;
}
