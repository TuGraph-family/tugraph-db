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

double DCCore(OlapBase<Empty>& graph) {
    auto all_vertices = graph.AllocVertexSubset();
    all_vertices.Fill();

    std::unordered_map<size_t, std::unordered_map<size_t, size_t> > dst_src_num_map;
    std::mutex my_mutex;

    graph.ProcessVertexActive<char>(
        [&](size_t vi) {
            std::unordered_map<size_t, std::unordered_map<size_t, size_t> > local_map;
            size_t src_degree = graph.OutDegree(vi);
            for (auto& e : graph.OutEdges(vi)) {
                size_t dst_degree = graph.OutDegree(e.neighbour);
                local_map[dst_degree][src_degree] += 1;
            }

            std::lock_guard<std::mutex> lk(my_mutex);
            for (auto& d_d : local_map) {
                size_t dst_degree = d_d.first;
                for (auto& s_d : d_d.second) {
                    size_t src_degree = s_d.first;
                    dst_src_num_map[dst_degree][src_degree] += s_d.second;
                }
            }
            return 0;
        },
        all_vertices);

    std::vector<std::pair<size_t, double> > dst_vector;
    for (auto& dst : dst_src_num_map) {
        size_t dst_degree = dst.first;
        size_t temp = 0;
        for (auto& src : dst.second) {
            temp += src.second;
        }
        dst_vector.push_back(
            std::make_pair(dst_degree, (double)temp / (double)graph.NumEdges()));
    }

    double variance = 0.0;
    double out_in = 0.0;
    for (auto& dst_pair : dst_vector) {
        variance += dst_pair.first * dst_pair.first * dst_pair.second;
        out_in += dst_pair.first * dst_pair.second;
    }

    variance -= out_in * out_in;
    printf("variance: %f\n", variance);

    double dc = 0.0;
    for (auto& f_pair : dst_vector) {
        for (auto& s_pair : dst_vector) {
            size_t d_d = f_pair.first;
            size_t s_d = s_pair.first;

            dc += d_d * s_d *
                  ((double)dst_src_num_map[d_d][s_d] / (double)graph.NumEdges() -
                   f_pair.second * s_pair.second);
        }
    }
    dc /= variance;
    return dc;
}
