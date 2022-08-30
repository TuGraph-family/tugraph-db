/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <stdlib.h>
#include "lgraph/lgraph.h"
using namespace lgraph_api;

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    response = "AntGroup test";
    return true;
}
