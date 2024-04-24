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

bool CheckClique(std::vector<std::vector<int> >& edges, int k, int* clique) {
    auto compare_edge = [&](const int& a, const int& b) -> bool { return (a > b) ? true : false; };
    for (int i = 0; i < k - 1; i++) {
        auto& outgoing_adj = edges[clique[i]];
        if (!std::binary_search(outgoing_adj.begin(), outgoing_adj.end(), clique[k],
                                compare_edge)) {
            return false;
        }
    }
    return true;
}

size_t KCliquesCore(OlapBase<Empty>& graph, int value_k, ParallelVector<size_t>& cliques_cnt) {
    size_t vertices_num = graph.NumVertices();
    std::vector<int> orders;
    auto index_order = graph.AllocVertexArray<int>();
    index_order.Fill(0);
    orders.resize(vertices_num);

    auto compare_adj = [](const void* a, const void* b) -> int {
        auto ptr_a = (AdjUnit<Empty>*)a;
        auto ptr_b = (AdjUnit<Empty>*)b;
        return ptr_a->neighbour > ptr_b->neighbour ? 1 : -1;
    };
    graph.ProcessVertexInRange<int>(
        [&](size_t v) {
            orders[v] = v;
            auto out_edges = graph.OutEdges(v);
            std::qsort(out_edges.begin(), graph.OutDegree(v), sizeof(AdjUnit<Empty>), compare_adj);
            return 0;
        },
        0, vertices_num);
    // order[v] = m中存储的是排名为v的节点m
    auto compare_index = [&](const int& a, const int& b) -> bool {
        return (graph.OutDegree(a) > graph.OutDegree(b)) ? true : false;
    };
    std::sort(orders.begin(), orders.end(), compare_index);
    // index_order[v] = m：节点v的排名为m，排名顺序由出边数决定
    graph.ProcessVertexInRange<int>(
        [&](size_t v) {
            index_order[orders[v]] = v;
            return 0;
        },
        0, vertices_num);

    auto edge_num = graph.AllocVertexArray<int>();
    edge_num.Fill(0);
    auto num_edges = graph.ProcessVertexInRange<size_t>(
        [&](size_t v) {
            size_t v_order = index_order[v];
            size_t local_edges_num = 0;
            size_t pre = -1;
            for (auto e : graph.OutEdges(v)) {
                size_t neighbour_order = index_order[e.neighbour];
                if (pre == e.neighbour) {
                    continue;
                } else {
                    pre = e.neighbour;
                }

                if (v_order > neighbour_order) {
                    local_edges_num++;
                }
            }
            edge_num[v] = local_edges_num;
            return local_edges_num;
        },
        0, vertices_num);
    std::cout << "num_edges = " << num_edges << std::endl;

    std::vector<std::vector<int> > edges(vertices_num);
    for (size_t i = 0; i < vertices_num; i++) {
        edges[i].resize(edge_num[orders[i]]);
    }

    size_t sorted_edges = graph.ProcessVertexInRange<size_t>(
        [&](size_t i) {
            int vid = orders[i];
            size_t local_index = 0;
            size_t pre = -1;
            for (auto e : graph.OutEdges(vid)) {
                if (pre == e.neighbour) {
                    continue;
                } else {
                    pre = e.neighbour;
                }
                size_t neighbour_index = index_order[e.neighbour];
                if (neighbour_index < i) {
                    edges[i][local_index] = neighbour_index;
                    local_index++;
                }
            }
            return local_index;
        },
        0, vertices_num);

    assert(sorted_edges == num_edges);
    std::cout << "sorted_edges = " << sorted_edges << std::endl;

    auto compare_edge = [&](const int& a, const int& b) -> bool { return (a > b) ? true : false; };

    graph.ProcessVertexInRange<int>(
        [&](size_t i) {
            std::sort(edges[i].begin(), edges[i].end(), compare_edge);
            return 0;
        },
        0, vertices_num);
    printf("sorted\n");

    size_t kcliques = graph.ProcessVertexInRange<size_t>(
        [&](size_t v_0) {
            size_t local_cnt = 0;
            int* clique = new int[value_k];
            size_t* degree_offset = new size_t[value_k];
            memset(clique, -1, value_k * sizeof(int));
            memset(degree_offset, 0, value_k * sizeof(size_t));
            int idx = 1;
            clique[0] = v_0;
            while (idx > 0) {
                int pre_v = clique[idx - 1];
                auto out_edges = edges[pre_v];
                size_t edge_idx = 0;
                for (edge_idx = degree_offset[idx - 1]; edge_idx < edges[pre_v].size();
                     edge_idx++) {
                    auto nbr = *(out_edges.begin() + edge_idx);
                    assert(nbr < pre_v);
                    clique[idx] = nbr;
                    if (!CheckClique(edges, idx, clique)) {
                        continue;
                    }
                    if (idx == value_k - 1) {
                        for (int c_v = 0; c_v < value_k; c_v++) {
                            write_add(&cliques_cnt[orders[clique[c_v]]], (size_t)1);
                        }
                        local_cnt++;
                        continue;
                    }
                    degree_offset[idx] = 0;
                    degree_offset[idx - 1] = edge_idx + 1;
                    idx++;
                    break;
                }
                if (edge_idx >= edges[pre_v].size()) {
                    clique[idx] = -1;
                    idx--;
                    degree_offset[idx] = 0;
                    clique[idx] = -1;
                }
            }
            delete clique;
            delete degree_offset;
            return local_cnt;
        },
        0, vertices_num);

    std::cout << "discovered " << kcliques << " " << value_k << "-cliques" << std::endl;

    return kcliques;
}
