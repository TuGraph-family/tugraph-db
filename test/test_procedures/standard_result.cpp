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

#include "lgraph/lgraph_result.h"
#include "lgraph/lgraph_traversal.h"
#include "iostream"
#include "tools/json.hpp"
using json = nlohmann::json;

using namespace lgraph_api;

extern "C" LGAPI bool Process(GraphDB &db, const std::string &request, std::string &response) {
    // defination
    Result result({{"float_var", LGraphType::FLOAT},
                   {"double_var", LGraphType::DOUBLE},
                   {"bool_var", LGraphType::BOOLEAN},
                   {"list_var", LGraphType::LIST},
                   {"in_edge", LGraphType::RELATIONSHIP},
                   {"out_edge", LGraphType::RELATIONSHIP},
                   {"path", LGraphType::PATH},
                   {"node", LGraphType::NODE},
                   {"edge_num_sum", LGraphType::INTEGER},
                   {"edge_num", LGraphType::MAP}});

    auto txn = db.CreateReadTxn();
    auto vit = txn.GetVertexIterator();
    size_t vid1 = 0;
    size_t vid2 = 0;
    std::vector<FieldData> vf;
    vf.push_back(FieldData(1));
    using Vertex = lgraph_api::traversal::Vertex;
    using Edge = lgraph_api::traversal::Edge;
    lgraph_api::traversal::Path p(Vertex(vit.GetId()));
    p.Append(Edge(0, 1, 1, 1, 1, true));
    for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
        auto it = vit.GetInEdgeIterator();
        if (it.IsValid()) vid1 = vit.GetId();
        auto ot = vit.GetOutEdgeIterator();
        if (ot.IsValid()) vid2 = vit.GetId();
    }
    for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
        auto record = result.MutableRecord();
        std::map<std::string, FieldData> edge_num_map;
        record->Insert("node", vit);
        int in_num_edges = 0;
        int out_num_edges = 0;
        float w = 123.233;
        std::string key = "key";
        double d = w;
        bool b = true;
        record->Insert("float_var", FieldData::Float(w));
        record->Insert("double_var", FieldData::Double(d));
        record->Insert("bool_var", FieldData::Bool(b));
        record->Insert("list_var", vf);
        auto vit1 = txn.GetVertexIterator();
        vit1.Goto(vid1);
        record->Insert("in_edge", vit1.GetInEdgeIterator());
        vit1.Goto(vid2);
        record->Insert("out_edge", vit1.GetOutEdgeIterator());
        record->Insert("path", p, &txn);
        for (auto eit = vit.GetOutEdgeIterator(); eit.IsValid(); eit.Next()) out_num_edges += 1;
        edge_num_map["out_num_edges"] = FieldData(out_num_edges);
        for (auto eit = vit.GetInEdgeIterator(); eit.IsValid(); eit.Next()) in_num_edges += 1;
        edge_num_map["in_num_edges"] = FieldData(in_num_edges);

        record->Insert("edge_num_sum", FieldData(in_num_edges + out_num_edges));

        record->Insert("edge_num", edge_num_map);
        Record copy_record(*record);
        copy_record = *record;
        break;
    }

    response = result.Dump();
    return true;
}
