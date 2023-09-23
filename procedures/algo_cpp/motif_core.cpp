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

#include <random>
#include <unordered_map>
#include "lgraph/olap_base.h"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

int motif_sort_compare(const void *a, const void *b) {
    const AdjUnit<Empty> *ptr_a = (const AdjUnit<Empty> *)a;
    const AdjUnit<Empty> *ptr_b = (const AdjUnit<Empty> *)b;
    return ptr_a->neighbour > ptr_b->neighbour ? 1 : -1;
}
bool motif_adj_compare(const AdjUnit<Empty> &a, const AdjUnit<Empty> &b) {
    return (a.neighbour < b.neighbour);
}

bool motif_check_match(size_t *vtx_list, size_t it, size_t n,
                       ParallelVector<ParallelBitset *> &active) {
    if (n > 1 && vtx_list[n - 1] > it && active[n - 1]->Has(it)) {
        return true;
    }
    for (size_t i = 0; i < n; i++) {
        if (vtx_list[i] == it) return true;
    }
    return false;
}

void mfinder(int k, OlapBase<Empty> &graph, size_t root, size_t *counts) {
    size_t vertice_num = graph.NumVertices();
    ParallelVector<ParallelBitset *> actives(k);
    std::vector<size_t> *vectors = new std::vector<size_t>[k];
    size_t *pos = new size_t[k];
    size_t *vtx_list = new size_t[k];
    for (int i = 0; i < k; i++) {
        actives.Append(new ParallelBitset(vertice_num));
        pos[i] = 0;
        vtx_list[i] = (size_t)-1;
    }
    actives[0]->Add(root);
    vectors[0].push_back(root);
    vtx_list[0] = root;
    int i = 1;
    uint64_t subgraph = 0ull;
    while (i > 0) {
        if (pos[i - 1] >= vectors[i - 1].size()) {
            pos[i - 1] = 0;
            i--;
            continue;
        }
        if (i < k - 1) {
            memcpy(actives[i]->Data(), actives[i - 1]->Data(),
                   sizeof(uint64_t) * (WORD_OFFSET(actives[i]->Size()) + 1));
            vectors[i].resize(vectors[i - 1].size());
            memcpy(vectors[i].data(), vectors[i - 1].data(),
                   vectors[i - 1].size() * sizeof(size_t));
            size_t vtx = vectors[i - 1][pos[i - 1]];
            ++pos[i - 1];
            bool flag = false;
            while (motif_check_match(vtx_list, vtx, i - 1, actives)) {
                if (pos[i - 1] >= vectors[i - 1].size()) {
                    flag = true;
                    break;
                }
                vtx = vectors[i - 1][pos[i - 1]];
                ++pos[i - 1];
            }
            if (flag) {
                pos[i - 1] = 0;
                i--;
                continue;
            }
            vtx_list[i - 1] = vtx;
            auto outgoing_adj = graph.OutEdges(vtx);
            for (auto &edge : outgoing_adj) {
                size_t dst = edge.neighbour;
                if (actives[i]->Has(dst) == 0) {
                    actives[i]->Add(dst);
                    vectors[i].push_back(dst);
                }
            }
            auto incoming_adj = graph.InEdges(vtx);
            for (auto &edge : incoming_adj) {
                size_t src = edge.neighbour;
                if (actives[i]->Has(src) == 0) {
                    actives[i]->Add(src);
                    vectors[i].push_back(src);
                }
            }
            for (int b = 0; b < i - 1; b++) {
                AdjUnit<Empty> adj;
                adj.neighbour = vtx_list[b];
                if (std::binary_search(outgoing_adj.begin(), outgoing_adj.end(), adj,
                                       motif_adj_compare)) {
                    subgraph |= 1ull << ((i - 1) * k + b);
                } else {
                    subgraph &= ~(1ull << ((i - 1) * k + b));
                }
                if (std::binary_search(incoming_adj.begin(), incoming_adj.end(), adj,
                                       motif_adj_compare)) {
                    subgraph |= 1ull << (b * k + i - 1);
                } else {
                    subgraph &= ~(1ull << (i - 1 + b * k));
                }
            }
            ++i;
        } else {
            memcpy(actives[i]->Data(), actives[i - 1]->Data(),
                   sizeof(uint64_t) * (WORD_OFFSET(actives[i]->Size()) + 1));
            size_t vtx = vectors[i - 1][pos[i - 1]];
            ++pos[i - 1];
            bool flag = false;
            while (motif_check_match(vtx_list, vtx, i - 1, actives)) {
                if (pos[i - 1] >= vectors[i - 1].size()) {
                    flag = true;
                    break;
                }
                vtx = vectors[i - 1][pos[i - 1]];
                ++pos[i - 1];
            }
            if (flag) {
                pos[i - 1] = 0;
                i--;
                continue;
            }
            vtx_list[i - 1] = vtx;
            auto outgoing_adj = graph.OutEdges(vtx);
            for (auto &edge : outgoing_adj) {
                size_t dst = edge.neighbour;
                actives[i]->Add(dst);
            }
            auto incoming_adj = graph.InEdges(vtx);
            for (auto &edge : incoming_adj) {
                size_t src = edge.neighbour;
                actives[i]->Add(src);
            }
            for (int b = 0; b < i - 1; b++) {
                AdjUnit<Empty> adj;
                adj.neighbour = vtx_list[b];
                if (std::binary_search(outgoing_adj.begin(), outgoing_adj.end(), adj,
                                       motif_adj_compare)) {
                    subgraph |= 1ull << ((i - 1) * k + b);
                } else {
                    subgraph &= ~(1ull << ((i - 1) * k + b));
                }
                if (std::binary_search(incoming_adj.begin(), incoming_adj.end(), adj,
                                       motif_adj_compare)) {
                    subgraph |= 1ull << (b * k + i - 1);
                } else {
                    subgraph &= ~(1ull << (i - 1 + b * k));
                }
            }
            graph.ProcessVertexActive<char>(
                [&](size_t vtx) {
                    if (motif_check_match(vtx_list, vtx, i, actives)) return 0;
                    uint64_t mat = subgraph;
                    auto outgoing_adj = graph.OutEdges(vtx);
                    auto incoming_adj = graph.InEdges(vtx);
                    for (int b = 0; b < i; b++) {
                        AdjUnit<Empty> adj;
                        adj.neighbour = vtx_list[b];
                        if (std::binary_search(outgoing_adj.begin(), outgoing_adj.end(), adj,
                                               motif_adj_compare)) {
                            mat |= 1ull << (i * k + b);
                        } else {
                            mat &= ~(1ull << (i * k + b));
                        }
                        if (std::binary_search(incoming_adj.begin(), incoming_adj.end(), adj,
                                               motif_adj_compare)) {
                            mat |= 1ull << (b * k + i);
                        } else {
                            mat &= ~(1ull << (b * k + i));
                        }
                    }
                    write_add(&counts[mat], (size_t)1);
                    return 0;
                },
                *actives[i]);
        }
    }

    for (int j = 0; j < k; j++) {
        delete actives[j];
    }
    delete[] pos;
    delete[] vtx_list;
    delete[] vectors;
}

void MotifCore(OlapBase<Empty> &graph, ParallelVector<size_t> &motif_vertices, int k,
               ParallelVector<std::map<uint64_t, size_t>> &motif_count) {
    auto active = graph.AllocVertexSubset();
    active.Fill();
    graph.ProcessVertexInRange<char>(
        [&](size_t vtx) {
            auto edges = graph.OutEdges(vtx);
            qsort(edges.begin(), graph.OutDegree(vtx), sizeof(AdjUnit<Empty>), motif_sort_compare);
            edges = graph.InEdges(vtx);
            qsort(edges.begin(), graph.InDegree(vtx), sizeof(AdjUnit<Empty>), motif_sort_compare);
            return 0;
        },
        0, graph.NumVertices());
    printf("sorted\n");

    uint64_t counts_len = 1ull << (k * k - 1);
    size_t *counts = new size_t[counts_len];
    bool *flags = new bool[counts_len];

    std::vector<std::vector<size_t>> permutation;
    std::vector<size_t> v_0;
    for (size_t i = 0; i < (size_t)k; i++) {
        v_0.push_back(i);
    }
    permutation.push_back(v_0);
    for (int i = 0; i < k; i++) {
        int s = permutation.size();
        for (int l = 0; l < s; l++) {
            for (int j = i + 1; j < k; j++) {
                v_0 = permutation[l];
                std::swap(v_0[i], v_0[j]);
                permutation.push_back(v_0);
            }
        }
    }

    int root_idx = 0;
    for (auto &root : motif_vertices) {
        memset(counts, 0, sizeof(size_t) * counts_len);
        mfinder(k, graph, root, counts);

        memset(flags, 0, sizeof(bool) * counts_len);
        for (uint64_t i = 0; i < counts_len; i++) {
            if (!flags[i] && counts[i] != 0) {
                flags[i] = true;
                uint64_t min_idx = i;
                for (auto &v : permutation) {
                    int idx = 0;
                    uint64_t subgraph = i;
                    uint64_t new_subgraph = 0ull;
                    while (subgraph != 0) {
                        if ((subgraph & 1ull) != 0) {
                            int a = idx / k;
                            int b = idx % k;
                            new_subgraph |= 1ull << (v[a] * k + v[b]);
                        }
                        subgraph >>= 1;
                        ++idx;
                    }
                    if (new_subgraph < counts_len && !flags[new_subgraph]) {
                        flags[new_subgraph] = true;
                        counts[i] += counts[new_subgraph];
                        if (min_idx > new_subgraph) min_idx = new_subgraph;
                    }
                }
                if (counts[i] > 0) {
                    motif_count[root_idx].insert(std::make_pair(min_idx, counts[i]));
                }
            }
        }
        root_idx++;
    }
    delete[] counts;
    delete[] flags;
}
