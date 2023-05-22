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

/* python lgraph_plugin.py -c Load -n 'SortStr' -p ./custom_function_sortstr.so -d 'custom function:
 * sort string of characters' -a localhost:7079
 * MATCH (n:City) RETURN n.name, custom.SortStr(n.name)  */
extern "C" LGAPI bool Process(GraphDB &db, const std::string &request, std::string &response) {
    response = request;
    sort(response.begin(), response.end());
    return true;
}
