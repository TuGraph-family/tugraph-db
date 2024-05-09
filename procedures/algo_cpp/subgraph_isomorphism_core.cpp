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

#include <unordered_set>
#include <unordered_map>
#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

int subgraph_sort_compare(const void *a, const void *b) {
    const auto *ptr_a = (const AdjUnit<Empty> *)a;
    const auto *ptr_b = (const AdjUnit<Empty> *)b;
    return ptr_a->neighbour > ptr_b->neighbour ? 1 : -1;
}
bool subgraph_adj_compare(const AdjUnit<Empty> &a, const AdjUnit<Empty> &b) {
    return (a.neighbour < b.neighbour);
}

bool subgraph_upper_compare(size_t a, const AdjUnit<Empty> &b) { return (a < b.neighbour); }

bool subgraph_check_match(const size_t *vtx_list, size_t it, size_t n) {
    for (size_t i = 0; i < n; i++) {
        if (vtx_list[i] == it) return true;
    }
    return false;
}

size_t subquery_merge(size_t *solution, const size_t *src_list, OlapBase<Empty> &graph,
                      std::vector<std::unordered_set<size_t>> query,
                      std::unordered_map<size_t, size_t> *candidate,
                      ParallelVector<size_t> &match_times) {
    size_t query_vtx = 1;
    size_t local_num = 0;
    while (query_vtx > 0) {
        size_t it;
        if (solution[query_vtx] == (size_t)-1) {
            if (src_list[query_vtx] != (size_t)-1) {
                auto edges = graph.OutEdges(solution[src_list[query_vtx]]);
                auto pos = edges.begin();
                if (pos == edges.end()) {
                    solution[query_vtx] = (size_t)-1;
                    query_vtx--;
                    continue;
                }
                it = pos->neighbour;
                solution[query_vtx] = it;
            } else {
                auto pos = candidate[query_vtx].begin();
                it = pos->first;
                solution[query_vtx] = it;
            }
        } else {
            if (src_list[query_vtx] != (size_t)-1) {
                auto edges = graph.OutEdges(solution[src_list[query_vtx]]);
                auto pos = std::upper_bound(edges.begin(), edges.end(), solution[query_vtx],
                                            subgraph_upper_compare);
                if (pos == edges.end()) {
                    solution[query_vtx] = (size_t)-1;
                    query_vtx--;
                    continue;
                }
                it = pos->neighbour;
                solution[query_vtx] = it;
            } else {
                auto pos = candidate[query_vtx].find(solution[query_vtx]);
                ++pos;
                if (pos == candidate[query_vtx].end()) {
                    solution[query_vtx] = (size_t)-1;
                    query_vtx--;
                    continue;
                }
                it = pos->first;
                solution[query_vtx] = it;
            }
        }

        if (subgraph_check_match(solution, it, query_vtx)) {
            continue;
        }
        if (candidate[query_vtx].count(solution[query_vtx]) == 0) {
            continue;
        }

        bool search_ret = true;
        for (auto &query_dst : query[query_vtx]) {
            if (query_dst >= query_vtx) {
                continue;
            }

            auto edges = graph.OutEdges(it);
            AdjUnit<Empty> dst{solution[query_dst]};
            if (!std::binary_search(edges.begin(), edges.end(), dst, subgraph_adj_compare)) {
                search_ret = false;
                break;
            }
        }
        if (!search_ret) {
            continue;
        }

        for (size_t query_src = 0; query_src < query_vtx; query_src++) {
            if (query[query_src].count(query_vtx) == 0) {
                continue;
            }
            auto edges = graph.InEdges(it);
            AdjUnit<Empty> src{solution[query_src]};
            if (!std::binary_search(edges.begin(), edges.end(), src, subgraph_adj_compare)) {
                search_ret = false;
                break;
            }
        }
        if (!search_ret) {
            continue;
        }
        query_vtx++;

        if (query_vtx == query.size()) {
            for (size_t i = 0; i < query_vtx; i++) {
                write_add(&match_times[solution[i]], (size_t)1);
            }
            local_num++;
            query_vtx--;
        }
    }
    return local_num;
}

size_t SubgraphIsomorphismCore(OlapBase<Empty> &graph,
                               std::vector<std::unordered_set<size_t>> &query,
                               ParallelVector<size_t> &counts) {
    graph.ProcessVertexInRange<char>(
        [&](size_t vtx) {
            auto edges = graph.OutEdges(vtx);
            qsort(edges.begin(), graph.OutDegree(vtx), sizeof(AdjUnit<Empty>),
                  subgraph_sort_compare);
            edges = graph.InEdges(vtx);
            qsort(edges.begin(), graph.InDegree(vtx), sizeof(AdjUnit<Empty>),
                  subgraph_sort_compare);
            return 0;
        },
        0, graph.NumVertices());
    printf("sorted\n");
    size_t num_vertices = graph.NumVertices();
    size_t num_subgraph;
    int k = query.size();

    auto *candidate = new std::unordered_map<size_t, size_t>[k];
    graph.ProcessVertexInRange<char>(
        [&](size_t vtx) {
            for (size_t i = 0; i < graph.NumVertices(); i++) {
                candidate[vtx].insert(std::pair<size_t, size_t>(i, (size_t)0));
            }
            return 0;
        },
        0, k);
    printf("candidate\n");

    auto *src = new size_t[k];
    graph.ProcessVertexInRange<char>(
        [&](size_t v) {
            src[v] = (size_t)-1;
            return 0;
        },
        0, k);
    graph.ProcessVertexInRange<int>(
        [&](size_t vtx) {
            for (auto &dst : query[vtx]) {
                if (src[dst] == (size_t)-1 && vtx < dst) {
                    write_min(&src[dst], vtx);
                }
            }
            return 0;
        },
        0, k);

    num_subgraph = graph.ProcessVertexInRange<size_t>(
        [&](size_t vtx) {
            if (candidate[0].count(vtx) == 0) {
                return (size_t)0;
            } else {
                auto *solution = new size_t[k];
                for (int i = 0; i < k; i++) {
                    solution[i] = (size_t)-1;
                }
                solution[0] = vtx;
                auto local_num = subquery_merge(solution, src, graph, query, candidate, counts);
                delete[] solution;
                return local_num;
            }
        },
        0, num_vertices);

    printf("match %zu subgraph\n", num_subgraph);
    return num_subgraph;
}
