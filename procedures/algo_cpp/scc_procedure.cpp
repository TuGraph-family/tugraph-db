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

#include "lgraph/olap_on_db.h"
#include "tools/json.hpp"
#include "./algo.h"

using namespace lgraph_api;
using namespace lgraph_api::olap;
using json = nlohmann::json;

void CountComp(OlapBase<Empty>& graph, ParallelVector<size_t>& label, size_t& max, size_t& num) {
    ParallelVector<size_t> cnt = graph.AllocVertexArray<size_t>();
    cnt.Fill(0);
    graph.ProcessVertexInRange<size_t>([&](size_t v) {
      if (graph.OutDegree(v) == 0 && graph.InDegree(v) == 0) return 0;
      size_t v_label = label[v];
      write_add(&cnt[v_label], (size_t)1);
      return 0;
    }, 0, label.Size());
    max = 0;
    num = graph.ProcessVertexInRange<size_t>([&](size_t v) {
      if (max < cnt[v])
          write_max(&max, cnt[v]);
      return ((graph.OutDegree(v) > 0 || graph.InDegree(v) > 0) && cnt[v] > 0) ? 1 : 0;
    }, 0, label.Size());
}


extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    double start_time;

    // prepare
    start_time = get_time();

    try {
        json input = json::parse(request);
    } catch (std::exception& e) {
        throw std::runtime_error("json parse error");
        return false;
    }
    auto txn = db.CreateReadTxn();
    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);
    auto prepare_cost = get_time() - start_time;

    // core
    start_time = get_time();
    auto label = olapondb.AllocVertexArray<size_t>();
    SCCCore(olapondb, label);
    size_t num_components, max_component;
    CountComp(olapondb, label, max_component, num_components);
    auto core_cost = get_time() - start_time;

    // output
    start_time = get_time();
    // TODO(any): write distance back to graph
    auto output_cost = get_time() - start_time;

    // return
    {
        json output;
        output["num_components"] = num_components;
        output["max_component"] = max_component;
        output["num_vertices"] = olapondb.NumVertices();
        output["num_edges"] = olapondb.NumEdges();
        output["prepare_cost"] = prepare_cost;
        output["core_cost"] = core_cost;
        output["output_cost"] = output_cost;
        output["total_cost"] = prepare_cost + core_cost + output_cost;
        response = output.dump();
    }
    return true;
}
