/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
        return db.AddVertexLabel(label, std::vector<FieldSpec>({{"id", STRING, false}}), "id");
    } catch (std::exception& e) {
        response = e.what();
        return false;
    }
}
