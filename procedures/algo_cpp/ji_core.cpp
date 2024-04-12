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

size_t JiCountCommon(AdjList<Empty> &list_a, AdjList<Empty> &list_b, size_t a, size_t b) {
    size_t local_count = 0;
    AdjUnit<Empty> *ptr_a = list_a.begin();
    AdjUnit<Empty> *ptr_b = list_b.begin();
    size_t pre_a = -1;
    size_t pre_b = -1;
    while (ptr_a != list_a.end() && ptr_b != list_b.end()) {
        if (pre_a == ptr_a->neighbour || ptr_a->neighbour == a) {
            ptr_a++;
            continue;
        }
        if (pre_b == ptr_b->neighbour || ptr_b->neighbour == b) {
            ptr_b++;
            continue;
        }
        if (ptr_a->neighbour < ptr_b->neighbour) {
            pre_a = ptr_a->neighbour;
            ptr_a++;
        } else if (ptr_a->neighbour > ptr_b->neighbour) {
            pre_b = ptr_b->neighbour;
            ptr_b++;
        } else {
            pre_a = ptr_a->neighbour;
            pre_b = ptr_b->neighbour;
            local_count++;
            ptr_a++;
            ptr_b++;
        }
    }
    return local_count;
}

int JiCompare(const void *a, const void *b) {
    const AdjUnit<Empty> *ptr_a = (const AdjUnit<Empty> *)a;
    const AdjUnit<Empty> *ptr_b = (const AdjUnit<Empty> *)b;
    return (ptr_a->neighbour - ptr_b->neighbour);
}

double JiCore(OlapBase<Empty> &graph, std::pair<size_t, size_t> search_pair) {
    size_t src = search_pair.first;
    size_t dst = search_pair.second;

    qsort(graph.OutEdges(src).begin(), graph.OutDegree(src), sizeof(AdjUnit<Empty>),
          JiCompare);
    qsort(graph.OutEdges(dst).begin(), graph.OutDegree(dst), sizeof(AdjUnit<Empty>),
          JiCompare);

    size_t prev = (size_t)-1;
    size_t src_valid_degree = 0;
    for (auto &edge : graph.OutEdges(src)) {
        auto nbr = edge.neighbour;
        if (prev == nbr || nbr == src) {
            continue;
        } else {
            src_valid_degree += 1;
            prev = nbr;
        }
    }

    prev = (size_t)-1;
    size_t dst_valid_degree = 0;
    for (auto &edge : graph.OutEdges(dst)) {
        auto nbr = edge.neighbour;
        if (prev == nbr || nbr == dst) {
            continue;
        } else {
            dst_valid_degree += 1;
            prev = nbr;
        }
    }

    double score = 0.0;
    if (src_valid_degree == 0 && dst_valid_degree == 0) {
        score = 1.0;
    } else if (src_valid_degree == 0 || dst_valid_degree == 0) {
        score = 0.0;
    } else {
        AdjList<Empty> src_list = graph.OutEdges(src);
        AdjList<Empty> dst_list = graph.OutEdges(dst);
        size_t count = JiCountCommon(src_list, dst_list, src, dst);
        score = 1.0 * count / (src_valid_degree + dst_valid_degree - count);
    }

    return score;
}
