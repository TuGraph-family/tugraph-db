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

ParallelVector<size_t> InitializeLabelWithOwnVid(OlapBase<Empty>& graph) {
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

double ComputeModularity(OlapBase<Empty>& graph, ParallelVector<size_t>& label) {
    size_t num_vertices = label.Size();
    size_t num_vertices_graph = graph.NumVertices();
    if (num_vertices != num_vertices_graph) {
        throw std::runtime_error("|V| mismatch");
    }
    size_t num_edges = graph.NumEdges();
    return graph.ProcessVertexInRange<double>(
               [&](size_t vi) {
                   double q = 0;
                   // for (auto & edge : graph.OutEdges(vi)) {
                   //     size_t nbr = edge.neighbour;
                   //     if (label[vi] == label[nbr]) {
                   //         q += (1 - 1.0 * graph.OutDegree(vi) * graph.OutDegree(nbr) /
                   //         num_edges);
                   //     }
                   // }
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
               0, num_vertices) /
           num_edges;
}

void ComputeMetrics(OlapBase<Empty>& graph,
        ParallelVector<std::vector<std::pair<size_t, size_t>>>& label_map,
        size_t threshold) {
    size_t num_vertices_graph = graph.NumVertices();
    size_t total_pair_num = graph.ProcessVertexInRange<size_t>(
    [&](size_t vi) { return (size_t)label_map[vi].size(); }, 0, num_vertices_graph);
    std::cout << "total_pair_num = " << total_pair_num << std::endl;
    std::cout << "ratio of pairs on vertices = " << (double)total_pair_num / num_vertices_graph
              << std::endl;

    size_t num_intra_edges = graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            size_t num_intra_edges = 0;
            for (auto& e : graph.OutEdges(vi)) {
                size_t nbr = e.neighbour;
                int is_intra = 0;
                std::vector<size_t> nbr_label;
                for (auto& ele : label_map[nbr]) {
                    if (ele.second >= threshold) {
                        nbr_label.push_back(ele.first);
                    }
                }
                for (auto& ele : label_map[vi]) {
                    if (ele.second < threshold) {
                        continue;
                    }
                    auto find_it = find(nbr_label.begin(), nbr_label.end(), ele.first);
                    if (find_it != nbr_label.end()) {
                        is_intra = 1;
                        break;
                    }
                }
                num_intra_edges += is_intra;
            }
            return num_intra_edges;
        },
        0, num_vertices_graph);

    size_t num_edges = graph.NumEdges();
    printf("ratio of intra-community edges = %.2lf%%\n", 100.0 * num_intra_edges / num_edges);

    auto count = graph.AllocVertexArray<size_t>();
    count.Fill((size_t)0);

    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            if (graph.OutDegree(vi) == 0) {
                return 0;
            }
            for (auto& ele : label_map[vi]) {
                graph.AcquireVertexLock(vi);
                if (ele.second >= threshold) {
                    count[ele.first] += 1;
                }
                graph.ReleaseVertexLock(vi);
            }
            return 0;
        },
        0, num_vertices_graph);

    size_t num_comm =
        graph.ProcessVertexInRange<size_t>([&](size_t vi) {
            return count[vi] > 0 ? 1 : 0; }, 0, num_vertices_graph);
    std::cout << "number of communities = " << num_comm << std::endl;
}

double SLPACore(OlapBase<Empty>& graph,
        ParallelVector<std::vector<std::pair<size_t, size_t>>>& label_map,
        int num_iterations, size_t threshold) {
    auto label = InitializeLabelWithOwnVid(graph);
    size_t num_vertices = label.Size();
    size_t num_vertices_graph = graph.NumVertices();
    if (num_vertices != num_vertices_graph) {
        throw std::runtime_error("|V| mismatch");
    }

    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            label_map[vi].push_back(std::pair<size_t, size_t>(label[vi], 1));
            return 0;
        },
        0, num_vertices);

    auto all_vertices = graph.AllocVertexSubset();
    thread_local unsigned int seed = 0;
    for (int ii = 0; ii < num_iterations; ii++) {
        size_t num_changed = graph.ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                std::unordered_map<size_t, size_t> count;
                for (auto& edge : graph.OutEdges(vi)) {
                    size_t dst = edge.neighbour;
                    size_t neighbour_label_times = 0;
                    graph.GuardVertexLock(dst);
                    int local_map_length = label_map[dst].size();
                    for (int i = 0; i < local_map_length; i++) {
                        neighbour_label_times += label_map[dst][i].second;
                    }
                    if (neighbour_label_times == 0) {
                        continue;
                    }
                    size_t r = rand_r(&seed) % neighbour_label_times;
                    size_t nbr_label;
                    for (int i = 0; i < local_map_length; i++) {
                        if (r < (size_t)label_map[dst][i].second) {
                            nbr_label = label_map[dst][i].first;
                            break;
                        }
                        r -= label_map[dst][i].second;
                    }
                    graph.ReleaseVertexLock(dst);

                    auto it = count.find(nbr_label);
                    if (it == count.end()) {
                        count[nbr_label] = 1;
                    } else {
                        it->second += 1;
                    }
                }

                std::vector<std::pair<size_t, size_t>> max_kv;
                for (auto& kv : count) {
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

                graph.AcquireVertexLock(vi);
                bool has_label = false;
                for (auto& ele : label_map[vi]) {
                    if (ele.first == sampled_label) {
                        has_label = true;
                        ele.second++;
                        break;
                    }
                }
                if (has_label == false) {
                    label_map[vi].push_back(std::pair<size_t, size_t>(sampled_label, 1));
                }
                graph.ReleaseVertexLock(vi);

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
    ComputeMetrics(graph, label_map, threshold);
    return ComputeModularity(graph, label);
}
