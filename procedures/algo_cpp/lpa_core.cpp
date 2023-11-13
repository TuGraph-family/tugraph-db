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

#include <unordered_map>
#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

void InitializeLabelWithOwnVid(OlapBase<Empty>& graph, ParallelVector<size_t>& label) {
    size_t num_vertices = label.Size();
    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            label[vi] = vi;
            return 0;
        },
        0, num_vertices);
}
double calcu_modularity(OlapBase<Empty> & graph, ParallelVector<size_t> & label) {
    auto active = graph.AllocVertexSubset();
    active.Fill();

    size_t num_intra_edges = graph.ProcessVertexActive<size_t>(
            [&](size_t v) {
                size_t num_intra_edges = 0;
                for (auto e : graph.OutEdges(v)) {
                    size_t nbr = e.neighbour;
                    if (label[v] == label[nbr]) num_intra_edges += 1;
                }
                return num_intra_edges;
            },
            active);

    size_t num_edges = graph.NumEdges();
    printf("ratio of intra-community edges: %.2lf%%\n", 100.0 * num_intra_edges / num_edges);

    auto count = graph.AllocVertexArray<size_t>();
    count.Fill(0);
    graph.ProcessVertexActive<size_t>(
        [&](size_t v) {
            size_t v_label = label[v];
            __sync_fetch_and_add(&count[v_label], 1);
            return 0;
        },
        active);

    size_t num_comm = graph.ProcessVertexActive<size_t>(
        [&](size_t v) {
            return count[v] > 0 ? 1 : 0;
        },
        active);
    printf("number of communities: %lu\n", num_comm);

    size_t largest_comm_size = 0;
    for (size_t v = 0; v < graph.NumVertices(); v++) {
        largest_comm_size = std::max(largest_comm_size, count[v]);
    }
    std::cout << "largest_comm_size = " << largest_comm_size << std::endl;

    auto e_tot = graph.AllocVertexArray<size_t>();
    e_tot.Fill(0.0);
    graph.ProcessVertexActive<size_t>(
        [&] (size_t v) {
            __sync_fetch_and_add(&e_tot[label[v]], (double)graph.OutDegree(v));
            return 0;
        },
        active);
    return graph.ProcessVertexActive<double>(
        [&] (size_t v) {
            double q = 0.0;
                for (auto edge : graph.OutEdges(v)) {
                    size_t nbr = edge.neighbour;
                    if (label[v] == label[nbr]) q += 1;
                }
                q -= 1.0 * graph.OutDegree(v) * e_tot[label[v]] / num_edges;
                return q;
        },
        active) / num_edges;
}

double LPACore(OlapBase<Empty>& graph, ParallelVector<size_t>& label,
        int num_iterations, bool sync_flag) {
    InitializeLabelWithOwnVid(graph, label);
    size_t num_vertices = label.Size();
    size_t num_vertices_graph = graph.NumVertices();
    auto next = graph.AllocVertexArray<size_t>();
    if (num_vertices != num_vertices_graph) {
        throw std::runtime_error("|V| mismatch");
    }
    for (int i_iter=0; i_iter < num_iterations; i_iter++) {
        graph.ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                if (graph.OutDegree(vi) == 0) {
                    return 0;
                }
                std::unordered_map<size_t, size_t> count;
                if (sync_flag) {
                    count[label[vi]] = 1;
                }
                for (auto& edge : graph.OutEdges(vi)) {
                    size_t nbr = edge.neighbour;
                    size_t nbr_label = label[nbr];
                    auto it = count.find(nbr_label);
                    if (it == count.end()) {
                        count[nbr_label] = 1;
                    } else {
                        it->second += 1;
                    }
                }
                std::pair<size_t, size_t> max_kv(label[vi], 0);
                for (auto & kv : count) {
                    if (max_kv.second < kv.second ||
                        (max_kv.second == kv.second && max_kv.first > kv.first)) {
                        max_kv = kv;
                    }
                }
                next[vi] = max_kv.first;
                if (max_kv.first != label[vi]) {
                    if (!sync_flag) {
                        label[vi] = max_kv.first;
                    }
                    return 1;
                } else {
                    return 0;
                }
            },
        0, num_vertices);
        if (sync_flag) {
            label.Swap(next);
        }
        // printf("%lu labels changed\n", num_changed);
    }
    return calcu_modularity(graph, label);
}

