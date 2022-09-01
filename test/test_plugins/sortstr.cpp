/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <iostream>
#include "lgraph/lgraph.h"

#include "tools/json.hpp"

using json = nlohmann::json;

using namespace lgraph_api;

/* python lgraph_plugin.py -c Load -n 'SortStr' -p ./custom_function_sortstr.so -d 'custom function:
 * sort string of characters' -a localhost:7079
 * MATCH (n:City) RETURN n.name, custom.SortStr(n.name)  */
extern "C" LGAPI bool Process(GraphDB &db, const std::string &request, std::string &response) {
    response = request;
    sort(response.begin(), response.end());
    return true;
}
