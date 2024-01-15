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

#include <stdlib.h>
#include "lgraph/lgraph.h"
#include "tools/lgraph_log.h"  // add log dependency
using namespace lgraph_api;

void LogExample() {
    LOG_DEBUG() << "This is a debug level log message.";
    LOG_INFO() << "This is a info level log message.";
    LOG_WARN() << "This is a warning level log message.";
    LOG_ERROR() << "This is a error level log message.";
}

extern "C" bool Process(GraphDB& db, const std::string& request, std::string& response) {
    response = "TuGraph log demo";
    LogExample();
    return true;
}
