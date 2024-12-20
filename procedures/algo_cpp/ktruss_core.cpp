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

size_t ktruss_count_common(std::vector<AdjUnit<size_t>> &list_a,
                           std::vector<AdjUnit<size_t>> &list_b, size_t a, size_t b) {
    size_t local_count = 0;
    auto ptr_a = list_a.begin();
    auto ptr_b = list_b.begin();
    size_t threshold = (a < b) ? a : b;
    while (ptr_a != list_a.end() && ptr_b != list_b.end() && ptr_a->neighbour < threshold &&
           ptr_b->neighbour < threshold) {
        if (ptr_a->edge_data == (size_t)-1) {
            ptr_a++;
        } else if (ptr_b->edge_data == (size_t)-1) {
            ptr_b++;
        } else if (ptr_a->neighbour < ptr_b->neighbour) {
            ptr_a++;
        } else if (ptr_a->neighbour > ptr_b->neighbour) {
            ptr_b++;
        } else {
#if USING_CENTOS9
            auto edge_a = ptr_a->edge_data;
            auto edge_b = ptr_b->edge_data;
            write_add(&edge_a, (size_t)1);
            write_add(&edge_b, (size_t)1);
#else
            write_add(&ptr_a->edge_data, (size_t)1);
            write_add(&ptr_b->edge_data, (size_t)1);
#endif
            local_count++;
            ptr_a++;
            ptr_b++;
        }
    }
    return local_count;
}

int truss_compare(const void *a, const void *b) {
    const AdjUnit<size_t> *ptr_a = (const AdjUnit<size_t> *)a;
    const AdjUnit<size_t> *ptr_b = (const AdjUnit<size_t> *)b;
    return ptr_a->neighbour > ptr_b->neighbour ? 1 : -1;
}

size_t KTrussCore(OlapBase<Empty> &graph, size_t value_k,
                  std::vector<std::vector<size_t>> &sub_neighbours) {
    auto active = graph.AllocVertexSubset();
    active.Fill();
    size_t vertices_num = graph.NumVertices();

    std::vector<int> orders;
    auto index_order = graph.AllocVertexArray<int>();
    index_order.Fill(0);
    orders.resize(vertices_num);

    auto compare_adj = [](const void *a, const void *b) -> int {
        auto ptr_a = (AdjUnit<Empty> *)a;
        auto ptr_b = (AdjUnit<Empty> *)b;
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
    auto compare_index = [&](const int &a, const int &b) -> bool {
        return (graph.OutDegree(a) > graph.OutDegree(b));
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
    size_t num_edges = graph.ProcessVertexInRange<size_t>(
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

    std::vector<std::vector<AdjUnit<size_t>>> edges(vertices_num);
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
                    AdjUnit<size_t> e;
                    e.edge_data = 0;
                    e.neighbour = neighbour_index;
                    edges[i][local_index] = e;
                    local_index++;
                }
            }
            return local_index;
        },
        0, vertices_num);
    assert(num_edges == sorted_edges);
    std::cout << "sorted_edges = " << sorted_edges << std::endl;

    graph.ProcessVertexInRange<int>(
        [&](size_t i) {
            qsort(edges[i].data(), edges[i].size(), sizeof(AdjUnit<size_t>), truss_compare);
            return 0;
        },
        0, vertices_num);
    printf("sorted\n");

    size_t left_edges_cnt = num_edges;
    size_t erase_edge_cnt = 1;
    int ii = 0;
    while (erase_edge_cnt != 0) {
        graph.ProcessVertexInRange<int>(
            [&](size_t src) {
                size_t local_count = 0;
                auto &src_adj = edges[src];
                for (auto &edge : src_adj) {
                    if (edge.edge_data == (size_t)-1) {
                        continue;
                    }
                    size_t dst = edge.neighbour;
                    {
                        auto &neighbour_adj = edges[dst];
                        local_count = ktruss_count_common(src_adj, neighbour_adj, src, dst);
                    }
#if USING_CENTOS9
                    auto edge_data = edge.edge_data;
                    write_add(&edge_data, local_count);
#else
                    write_add(&edge.edge_data, local_count);
#endif
                }
                return 0;
            },
            0, vertices_num);

        erase_edge_cnt = graph.ProcessVertexInRange<size_t>(
            [&](size_t vtx) {
                size_t local_cnt = 0;
                auto &src_adj = edges[vtx];
                for (auto &edge : src_adj) {
                    if (edge.edge_data == (size_t)-1) {
                        continue;
                    } else if (edge.edge_data < value_k - 2) {
                        edge.edge_data = (size_t)-1;
                        local_cnt++;
                    } else {
                        edge.edge_data = 0;
                    }
                }
                return local_cnt;
            },
            0, vertices_num);

        std::cout << "active(" << ii << "): erase " << erase_edge_cnt << " edges" << std::endl;
        left_edges_cnt -= erase_edge_cnt;
        ii++;
    }

    graph.ProcessVertexInRange<int>(
        [&](size_t vtx) {
            size_t original_v = orders[vtx];
            auto &out_edges = edges[vtx];
            for (auto &edge : out_edges) {
                if (edge.edge_data == (size_t)-1) {
                    continue;
                } else {
                    sub_neighbours[original_v].push_back(orders[edge.neighbour]);
                }
            }
            return 0;
        },
        0, vertices_num);

    std::cout << "last_edge:" << left_edges_cnt << std::endl;

    return left_edges_cnt;
}
