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

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    auto start_time = get_time();

    std::vector<std::string> vertices_id;
    std::string label = "node";
    std::string field = "id";
    int k = 3;
    std::cout << "Input: " << request << std::endl;
    try {
        json input = json::parse(request);
        parse_from_json(vertices_id, "vertices", input);
        parse_from_json(label, "label", input);
        parse_from_json(field, "field", input);
        parse_from_json(k, "k", input);
    } catch (std::exception& e) {
        response = "json parse error: " + std::string(e.what());
        std::cout << response << std::endl;
        return false;
    }

    auto txn = db.CreateReadTxn();

    OlapOnDB<Empty> olapondb(db, txn, SNAPSHOT_PARALLEL);

    ParallelVector<size_t> vertices_vid;
    if (!vertices_id.empty()) {
        vertices_vid.ReAlloc(vertices_id.size());
        for (auto id : vertices_id) {
            int64_t vid = txn.GetVertexIndexIterator(label, field, id, id).GetVid();
            vertices_vid.Append(olapondb.MappedVid(vid));
        }
    } else {
        //        throw std::runtime_error("No node found for input in DB.");
        vertices_vid.ReAlloc(1);
        int64_t vid = txn.GetVertexIndexIterator(label, field, "0", "0").GetVid();
        vertices_vid.Append(olapondb.MappedVid(vid));
    }

    auto preprocessing_time = get_time();
    auto counts = olapondb.AllocVertexArray<std::map<uint64_t, size_t>>();
    MotifCore(olapondb, vertices_vid, k, counts);
    auto exec_time = get_time();

    json output;
    for (size_t i = 0; i < vertices_vid.Size(); i++) {
        output["motif"].push_back(counts[i]);
    }
    auto output_time = get_time();

    output["preprocessing_time"] = (double)(preprocessing_time - start_time);
    output["exec_time"] = (double)(exec_time - preprocessing_time);
    output["output_time"] = (double)(output_time - exec_time);
    output["sum_time"] = (double)(output_time - start_time);
    response = output.dump();
    return true;
}
