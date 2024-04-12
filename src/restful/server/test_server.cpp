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

#include "fma-common/unit_test_utils.h"

#include "restful/server/stdafx.h"
#include "restful/server/rest_server.h"
#include "test/graph_factory.h"

using namespace web;
using namespace http;
using namespace utility;
using namespace http::experimental::listener;

using namespace lgraph;

FMA_SET_TEST_PARAMS(Server, "");

FMA_UNIT_TEST(Server) {
    uint16_t port = 7474;
    if (argc == 2) {
        port = atoi(argv[1]);
    }

    std::string host = "127.0.0.1";

    lgraph::StateMachine::Config server_config;
    server_config.async = true;
    server_config.db_dir = "./testdb";
    server_config.n_python_workers = 1;
    StateMachine server(server_config);

    RestServer::Config rest_config;
    rest_config.host = host;
    rest_config.port = port;
    RestServer rest_server(&server, rest_config);

    std::cout << "Press ENTER to exit." << std::endl;
    std::string line;
    std::getline(std::cin, line);
    rest_server.Stop();
    return 0;
}
