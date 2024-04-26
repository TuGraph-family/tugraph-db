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

#include <stack>
#include "lgraph/olap_base.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;

size_t LocateCycleCore(OlapBase<Empty>& graph, size_t k) {
    auto active = graph.AllocVertexSubset();
    active.Fill();
    size_t num_rings = graph.ProcessVertexActive<size_t>(
        [&](size_t vi){
            std::vector< std::vector<size_t>> local_paths;
            std::stack<std::vector<size_t>> path_stack;
            if (graph.OutDegree(vi) == 0 || graph.InDegree(vi) == 0) return (size_t)0;
            path_stack.push(std::vector<size_t>(1, vi));
            while (!path_stack.empty()) {
                auto path = path_stack.top();
                path_stack.pop();
                size_t v_top = path.back();
                if (path.size() > k) continue;
                for (auto & edge : graph.OutEdges(v_top)) {
                    size_t nbr = edge.neighbour;
                    if (nbr < vi) continue;
                    std::vector<size_t> temp(path);
                    temp.push_back(nbr);
                    if (std::find(path.begin(), path.end(),
                        nbr) != path.end() && path.size() != k) continue;
                    if (nbr == vi && path.size() == k) {
                        local_paths.push_back(temp);
                    }
                    path_stack.push(temp);
                }
            }
            return local_paths.size();
        },
        active);
    return num_rings;
}
