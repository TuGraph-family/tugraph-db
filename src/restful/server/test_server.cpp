/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

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
