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

ParallelVector<size_t> InitializeLabelWithOwnVid(OlapBase<double>& graph) {
    ParallelVector<size_t> label = graph.AllocVertexArray<size_t>();
    size_t num_vertices = label.Size();
    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            label[vi] = vi;
            return 0;
        },
        0, num_vertices);
    return label;
}

double ComputeModularity(OlapBase<double>& graph, ParallelVector<size_t>& label) {
    size_t num_vertices = label.Size();
    size_t num_vertices_graph = graph.NumVertices();
    if (num_vertices != num_vertices_graph) {
        throw std::runtime_error("|V| mismatch");
    }
    double intra_edges_weight = graph.ProcessVertexInRange<double>(
        [&](size_t vi) {
            double local_intra_edges_weight = 0;
            for (auto e : graph.OutEdges(vi)) {
                if (label[vi] == label[e.neighbour]) {
                    local_intra_edges_weight += e.edge_data;
                }
            }
            return local_intra_edges_weight;
        },
        0, num_vertices);
    double all_edges_weight = graph.ProcessVertexInRange<double>(
        [&](size_t vi) {
            double local_edges_weight = 0;
            for (auto e : graph.OutEdges(vi)) {
                local_edges_weight += e.edge_data;
            }
            return local_edges_weight;
        },
        0, num_vertices);
    std::cout << "ratio of intra-community weights: "
              << 100.0 * intra_edges_weight / all_edges_weight << "%\n";

    auto count = graph.AllocVertexArray<size_t>();
    count.Fill((size_t)0);
    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            if (graph.OutDegree(vi) == 0) {
                return 0;
            }
            size_t v_label = label[vi];
            graph.AcquireVertexLock(v_label);
            count[v_label] += 1;
            graph.ReleaseVertexLock(v_label);
            return 0;
        },
        0, num_vertices);
    size_t num_comm =
        graph.ProcessVertexInRange<size_t>([&](size_t vi) {
            return count[vi] > 0 ? 1 : 0; }, 0, num_vertices);
    std::cout << "number of communities: " << num_comm << std::endl;
    size_t num_edges = graph.NumEdges();
    return graph.ProcessVertexInRange<double>(
               [&](size_t vi) {
                   double q = 0.0;
                   std::unordered_map<size_t, size_t> count;
                   for (auto& edge : graph.OutEdges(vi)) {
                       size_t nbr = edge.neighbour;
                       if (label[vi] != label[nbr]) continue;
                       auto it = count.find(nbr);
                       if (it == count.end()) {
                           count[nbr] = 1;
                       } else {
                           it->second += 1;
                       }
                   }
                   for (auto kv : count) {
                       q += (kv.second - 1.0 * graph.OutDegree(vi) *
                                             graph.OutDegree(kv.first) / num_edges);
                   }
                   return q;
               },
               0, num_vertices) / num_edges;
}

double WLPACore(OlapBase<double>& graph, int num_iterations) {
    auto label = InitializeLabelWithOwnVid(graph);
    size_t num_vertices = label.Size();
    size_t num_vertices_graph = graph.NumVertices();
    if (num_vertices != num_vertices_graph) {
        throw std::runtime_error("|V| mismatch");
    }
    auto all_vertices = graph.AllocVertexSubset();
    unsigned int seed = 0;
    for (int ii = 0; ii < num_iterations; ii++) {
        size_t num_changed = graph.ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                std::unordered_map<size_t, double> count;
                for (auto& edge : graph.OutEdges(vi)) {
                    size_t nbr = edge.neighbour;
                    size_t nbr_label = label[nbr];
                    auto it = count.find(nbr_label);
                    if (it == count.end()) {
                        count[nbr_label] = edge.edge_data;
                    } else {
                        it->second += edge.edge_data;
                    }
                }
                std::vector<std::pair<size_t, double> > max_kv;
                for (auto kv : count) {
                    if (max_kv.empty()) {
                        max_kv.push_back(kv);
                    } else {
                        if (max_kv[0].second == kv.second) {
                            max_kv.push_back(kv);
                        } else if (max_kv[0].second < kv.second) {
                            max_kv.clear();
                            max_kv.push_back(kv);
                        }
                    }
                }
                size_t sampled_label = label[vi];
                if (!max_kv.empty()) {
                    int r = rand_r(&seed) % max_kv.size();
                    sampled_label = max_kv[r].first;
                }
                if (label[vi] != sampled_label) {
                    label[vi] = sampled_label;
                    return 1;
                } else {
                    return 0;
                }
            },
            0, num_vertices);
        printf("num_changed(%d) = %lu\n", ii, num_changed);
    }
    return ComputeModularity(graph, label);
}
