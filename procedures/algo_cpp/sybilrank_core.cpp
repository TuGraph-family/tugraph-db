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

#include <cmath>
#include "lgraph/olap_base.h"
#include "./algo.h"

void SybilRankCore(OlapBase<Empty> &graph, ParallelVector<size_t> &trust_seeds,
                   ParallelVector<double> &curr) {
    auto next = graph.AllocVertexArray<double>();
    curr.Fill(0);
    auto active_in = graph.AllocVertexSubset();
    auto active_out = graph.AllocVertexSubset();
    for (auto &s : trust_seeds) {
        curr[s] = 1;
        active_in.Add(s);
        if (graph.OutDegree(s) > 0) {
            curr[s] /= graph.OutDegree(s);
        }
    }

    int iterations = std::ceil(std::log(graph.NumVertices()));
    std::cout << "iterations=" << iterations << std::endl;
    double delta = trust_seeds.Size();
    size_t num_vertices = graph.NumVertices();

    for (int i_i = 0; i_i < iterations; i_i++) {
        next.Fill(0);
        graph.ProcessVertexInRange<size_t>(
            [&](size_t src) {
                for (auto &edge : graph.OutEdges(src)) {
                    auto dst = edge.neighbour;
                    active_out.Add(dst);
                    write_add(&next[dst], curr[src]);
                }
                return 1;
            },
            0, num_vertices);
        if (i_i != iterations - 1) {
            delta = graph.ProcessVertexInRange<double>(
                [&](size_t src) {
                    if (graph.OutDegree(src) > 0) {
                        next[src] /= graph.OutDegree(src);
                        return std::fabs(next[src] - curr[src]) * graph.OutDegree(src);
                    }
                    return std::fabs(next[src] - curr[src]);
                },
                0, num_vertices);
        } else {
            delta = graph.ProcessVertexInRange<double>(
                [&](size_t src) { return std::fabs(next[src] - curr[src]); }, 0, num_vertices);
        }
        curr.Swap(next);
        std::cout << "delta[" << i_i << "] = " << delta << std::endl;
    }
}
