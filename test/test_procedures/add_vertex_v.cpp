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

#include <iostream>
#include "lgraph/lgraph.h"

#include "tools/json.hpp"

using json = nlohmann::json;

using namespace lgraph_api;

extern "C" LGAPI bool Process(GraphDB& db, const std::string& request, std::string& response) {
    size_t vertex_size = 300000;
    try {
        std::cout << "  input: " << request << std::endl;
        json input = json::parse(request);
        if (input.contains("vertex_size")) {
            vertex_size = input["vertex_size"].get<size_t>();
        }
    } catch (std::exception& e) {
        response = std::string("failed to parse json: ") + e.what();
        return false;
    }
    try {
        VertexOptions options;
        options.primary_field = "id";
        if (!db.AddVertexLabel("v", std::vector<FieldSpec>({{"id", STRING, false}}), options))
            return false;
        auto txn = db.CreateWriteTxn();
        for (int i = 0; i < vertex_size; ++i) {
             txn.AddVertex("v", std::vector<std::string>{"id"},
                              std::vector<std::string>{std::to_string(i)});
        }
        txn.Commit();
        return true;
    } catch (std::exception& e) {
        response = e.what();
        return false;
    }
}
