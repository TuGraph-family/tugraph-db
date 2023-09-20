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

using namespace lgraph_api;
using namespace lgraph_api::olap;

void HITSCore(OlapBase<Empty> & graph, ParallelVector<double>& authority,
        ParallelVector<double>& hub, const int iterations, const double delta) {
    authority.Fill(1.0);
    hub.Fill(1.0);
    auto authority_next = graph.AllocVertexArray<double>();
    auto hub_next = graph.AllocVertexArray<double>();
    auto active = graph.AllocVertexSubset();
    active.Fill();

    double curr_delta = 1.0;
    int iter_i = 0;
    while (iter_i++ < iterations && curr_delta >= delta) {
        authority_next.Fill(0.0);
        hub_next.Fill(0.0);

        double total_authority = graph.ProcessVertexActive<double>(
                [&] (size_t vi) {
                    double local_authority = 0.;
                    for (auto & edge : graph.InEdges(vi)) {
                        auto nbr = edge.neighbour;
                        local_authority += hub[nbr];
                    }
                    authority_next[vi] = local_authority;
                    return local_authority * local_authority;
                },
                active);
        double normalize_authority = std::sqrt(total_authority);
        curr_delta = graph.ProcessVertexActive<double>(
                [&] (size_t vi) {
                    authority_next[vi] /= normalize_authority;
                    return std::abs(authority_next[vi] - authority[vi]);
                },
                active);
        authority_next.Swap(authority);

        double total_hub = graph.ProcessVertexActive<double>(
                [&] (size_t vi) {
                    double local_hub = 0.;
                    for (auto & edge : graph.OutEdges(vi)) {
                        auto nbr = edge.neighbour;
                        local_hub += authority[nbr];
                    }
                    hub_next[vi] = local_hub;
                    return local_hub * local_hub;
                },
                active);
        double normalize_hub = std::sqrt(total_hub);
        curr_delta += graph.ProcessVertexActive<double>(
                [&] (size_t vi) {
                    hub_next[vi] /= normalize_hub;
                    return std::abs(hub_next[vi] - hub[vi]);
                },
                active);
        hub_next.Swap(hub);

        printf("delta(%d) = %lf\n", iter_i, curr_delta);
    }
}
