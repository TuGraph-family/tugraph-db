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
    std::string label;
    try {
        std::cout << "  input: " << request << std::endl;
        json input = json::parse(request);
        label = input["label"].get<std::string>();
    } catch (std::exception& e) {
        response = std::string("failed to parse json: ") + e.what();
        return false;
    }
    try {
        return db.AddVertexLabel(label, std::vector<FieldSpec>({{"id", STRING, false}}),
                                 VertexOptions("id"));
    } catch (std::exception& e) {
        response = e.what();
        return false;
    }
}
