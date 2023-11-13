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

size_t BCCore(OlapBase<Empty>& graph, size_t samples, ParallelVector<double>& score) {
    size_t vertices = graph.NumVertices();
    auto num_paths = graph.AllocVertexArray<double>();
    auto inv_num_paths = graph.AllocVertexArray<double>();
    auto dependencies = graph.AllocVertexArray<double>();
    auto active_all = graph.AllocVertexSubset();
    active_all.Fill();
    auto visited = graph.AllocVertexArray<bool>();
    auto level = graph.AllocVertexArray<size_t>();
    auto active_in = graph.AllocVertexSubset();
    auto active_out = graph.AllocVertexSubset();
    unsigned int seed = 0;
    // xorshift128plus rng;
    // srand(time(NULL));
    // int rand_init = 0;
    // while(rand_init == 0) {
    //     rand_init = rand();
    // }
    // rng.init(rand_init);
    // unsigned int seed = 2;
    for (size_t s_i = 0; s_i < samples; s_i++) {
        size_t root = rand_r(&seed) % vertices;
        // size_t root = ((s_i + 1) * (s_i+1) * (s_i+1)) % vertices;
        size_t active_vertices = 1;
        // printf("bc caculating process have finished %6.2lf%%\r", 100. * (s_i + 1) / samples);
        // fflush(stdout);

        visited.Fill(false);
        visited[root] = true;
        active_in.Clear();
        active_in.Add(root);
        level.Fill(vertices);
        level[root] = (size_t)0;
        num_paths.Fill(0.0);
        num_paths[root] = 1.0;
        size_t i_i = 0;

        while (active_vertices > 0) {
            i_i++;
            active_out.Clear();
            graph.ProcessVertexActive<size_t>(
                [&](size_t vi) {
                    for (auto edge : graph.OutEdges(vi)) {
                        size_t dst = edge.neighbour;
                        if (!visited[dst]) {
                            if (num_paths[dst] == 0) {
                                active_out.Add(dst);
                            }
                            auto lock = graph.GuardVertexLock(dst);
                            num_paths[dst] += num_paths[vi];
                        }
                    }
                    return 0;
                },
                active_in);
            active_vertices = graph.ProcessVertexActive<size_t>(
                [&](size_t vi) {
                    visited[vi] = true;
                    level[vi] = i_i;
                    return 1;
                },
                active_out);
            active_in.Swap(active_out);
        }

        graph.ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                inv_num_paths[vi] = 1. / num_paths[vi];
                dependencies[vi] = 0;
                return 1;
            },
            0, vertices);
        visited.Fill(false);
        active_in.Clear();

        graph.ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                if (level[vi] == i_i) {
                    active_in.Add(vi);
                    return 1;
                } else {
                    return 0;
                }
            },
            0, vertices);
        graph.ProcessVertexActive<size_t>(
            [&](size_t vi) {
                visited[vi] = true;
                dependencies[vi] += inv_num_paths[vi];
                return 1;
            },
            active_in);

        graph.Transpose();
        while (i_i > 0) {
            graph.ProcessVertexActive<size_t>(
                [&](size_t vi) {
                    for (auto edge : graph.OutEdges(vi)) {
                        size_t dst = edge.neighbour;
                        if (!visited[dst]) {
                            auto lock = graph.GuardVertexLock(dst);
                            dependencies[dst] += dependencies[vi];
                        }
                    }
                    return 0;
                },
                active_in);
            i_i--;
            active_in.Clear();

            graph.ProcessVertexInRange<size_t>(
                [&](size_t vi) {
                    if (level[vi] == i_i) {
                        active_in.Add(vi);
                        return 1;
                    } else {
                        return 0;
                    }
                },
                0, vertices);
            graph.ProcessVertexActive<size_t>(
                [&](size_t vi) {
                    visited[vi] = true;
                    dependencies[vi] += inv_num_paths[vi];
                    return 1;
                },
                active_in);
        }

        graph.Transpose();

        graph.ProcessVertexInRange<size_t>(
            [&](size_t vi) {
                dependencies[vi] = (dependencies[vi] - inv_num_paths[vi]) / inv_num_paths[vi];
                score[vi] += inv_num_paths[vi] != 1.0 / 0.0 ? dependencies[vi] : 0.0;
                return 1;
            },
            0, vertices);

        score[root] -= dependencies[root];
        num_paths.Swap(inv_num_paths);
    }

    auto all_vertices = graph.AllocVertexSubset();
    all_vertices.Fill();
    size_t max_score_vi = graph.ProcessVertexActive<size_t>(
        [&](size_t vi) { return vi; }, all_vertices, 0,
        [&](size_t a, size_t b) { return score[a] > score[b] ? a : b; });
    size_t min_score_vi = graph.ProcessVertexActive<size_t>(
        [&](size_t vi) { return vi; }, all_vertices, 0,
        [&](size_t a, size_t b) { return score[a] < score[b] ? a : b; });

    double max_score = score[max_score_vi];
    double min_score = score[min_score_vi];
    double score_diff = max_score - min_score;
    graph.ProcessVertexInRange<size_t>(
        [&](size_t vi) {
            score[vi] = (score[vi] - min_score) / score_diff;
            return 1;
        },
        0, vertices);
    std::cout << "max score: " << max_score << std::endl;
    std::cout << "min score: " << min_score << std::endl;
    return max_score_vi;
}
