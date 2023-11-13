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

size_t cn_count_common(AdjList<Empty> & list_a, AdjList<Empty> & list_b) {
    size_t local_count = 0;
    AdjUnit<Empty> * ptr_a = list_a.begin();
    AdjUnit<Empty> * ptr_b = list_b.begin();
    while (ptr_a != list_a.end() && ptr_b != list_b.end()) {
        if (ptr_a->neighbour < ptr_b->neighbour) {
          ptr_a++;
        } else if (ptr_a->neighbour > ptr_b->neighbour) {
          ptr_b++;
        } else {
          local_count++;
          ptr_a++;
          ptr_b++;
        }
    }
    return local_count;
}

int cn_compare(const void * a, const void * b) {
    const AdjUnit<Empty> * ptr_a = (const AdjUnit<Empty>*) a;
    const AdjUnit<Empty> * ptr_b = (const AdjUnit<Empty>*) b;
    return (ptr_a->neighbour - ptr_b->neighbour);
}

size_t CNCore(OlapBase<Empty> & graph, std::pair<size_t, size_t> search_pair) {
    size_t src = search_pair.first;
    size_t dst = search_pair.second;

    qsort(graph.OutEdges(src).begin(), graph.OutDegree(src), sizeof(AdjUnit<Empty>), cn_compare);
    qsort(graph.OutEdges(dst).begin(), graph.OutDegree(dst), sizeof(AdjUnit<Empty>), cn_compare);

    AdjList<Empty> src_list = graph.OutEdges(src);
    AdjList<Empty> dst_list = graph.OutEdges(dst);

    return cn_count_common(src_list, dst_list);
}
