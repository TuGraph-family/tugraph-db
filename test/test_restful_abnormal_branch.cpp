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

#include "fma-common/configuration.h"
#include "fma-common/logging.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

#include "restful/server/json_convert.h"
#include "client/cpp/restful/rest_client.h"
#include "fma-common/string_formatter.h"
#include "server/state_machine.h"
#include "restful/server/rest_server.h"

// The 'U' macro can be used to create a string or character literal of the platform type, i.e.
// utility::char_t. If you are using a library causing conflicts with 'U' macro, it can be turned
// off by defining the macro '_TURN_OFF_PLATFORM_STRING' before including the C++ REST SDK header
// files, and e.g. use '_XPLATSTR' instead.
#define _TURN_OFF_PLATFORM_STRING
#include "cpprest/http_client.h"

#include "./graph_factory.h"
#include "./test_tools.h"
#include "./ut_utils.h"
using namespace concurrency::streams;  // Asynchronous streams

using web::http::http_request;
using web::http::http_response;
using web::http::methods;
using web::http::status_codes;
using web::http::client::http_client;

void test_client_other_branch(RestClient& client);

static void SetAuthInfo(std::string header, http_request& request) {
    request.headers().clear();
    request.headers().add(_TU("Authorization"), _TU(header));
    request.headers().add(_TU("Content-Type"), _TU("application/json"));
}

inline void WriteFiles(const std::map<std::string, std::string>& name_contents) {
    for (auto& kv : name_contents) {
        fma_common::OutputFmaStream stream(kv.first);
        stream.Write(kv.second.data(), kv.second.size());
    }
}

// disabled for now, will refactor
/*
FMA_SET_TEST_PARAMS(RestfulAbnormal,
    "--ssl true",
    "--ssl false");
*/

class TestRestfulAbnormal : public TuGraphTest {};

TEST_F(TestRestfulAbnormal, RestfulAbnormal) {
    using namespace fma_common;
    using namespace lgraph;
    bool enable_ssl = true;
    std::string host = "127.0.0.1";
    uint16_t port = 6464;
    std::string db_name = "default";
    std::shared_ptr<lgraph::GlobalConfig> gconfig = std::make_shared<lgraph::GlobalConfig>();
    {
        int argc = _ut_argc;
        char** argv = _ut_argv;
        Configuration config;
        config.Add(enable_ssl, "ssl", true).Comment("Enable SSL");
        config.Add(host, "host", true).Comment("Host address");
        config.Add(port, "port", true).Comment("HTTP port");
        config.Add(gconfig->enable_ip_check, "enable_ip_check", true).Comment("Enable IP check.");
        config.ParseAndFinalize(argc, argv);
    }
#ifdef _WIN32
    enable_ssl = false;
#endif

    const std::string key_path = "./key";
    const std::string cert_path = "./cert";
    if (enable_ssl) {
        lgraph::WriteCertFiles(cert_path, key_path);
    }

    // -----------------------------------
    // start server
    // build a test db
    { GraphFactory::create_modern(); }
    // Build listener's URI from the configured address and the hard-coded path
    // "RestServer/Action"
    lgraph::StateMachine::Config sm_config;
    sm_config.db_dir = "./testdb";
    lgraph::StateMachine state_machine(sm_config, gconfig);
    auto res = state_machine.GetMasterRestAddr();
    UT_EXPECT_EQ(res, "");
    state_machine.Start();
    lgraph::RestServer::Config rest_config;
    rest_config.host = host;
    rest_config.port = port;
    rest_config.use_ssl = enable_ssl;
    if (enable_ssl) {
        UT_EXPECT_ANY_THROW(lgraph::RestServer(&state_machine, rest_config, gconfig));
        rest_config.server_key = key_path;
        UT_EXPECT_ANY_THROW(lgraph::RestServer(&state_machine, rest_config, gconfig));
        rest_config.server_cert = cert_path;
    }
    // open the db_config api to set
    lgraph::RestServer rest_server(&state_machine, rest_config, gconfig);

    // -----------------------------------
    // server started, now start test
    std::string url;
    if (enable_ssl)
        url = fma_common::StringFormatter::Format("https://{}:{}/", host, port);
    else
        url = fma_common::StringFormatter::Format("http://{}:{}/", host, port);
    UT_LOG() << "  url:  " << url;

    // now start client
    RestClient client(url, cert_path);
    client.Login("admin", "73@TuGraph");
    UT_LOG() << "Testing login succeeded";

    // schema
    UT_LOG() << "Add company schema";
    {
        std::vector<FieldSpec> fds;
        fds.emplace_back(FieldSpec("name", FieldType::STRING, false));
        fds.emplace_back(FieldSpec("address", FieldType::STRING, false));
        fds.emplace_back(FieldSpec("scale", FieldType::INT32, false));
        UT_EXPECT_EQ(client.AddVertexLabel(db_name, "company", fds, "name"), true);
        UT_EXPECT_ANY_THROW(client.AddVertexLabel(db_name, "company", fds, "name"));
    }

    // index
    UT_LOG() << "Testing index";
    {
        // UT_EXPECT_EQ(client.AddVertexIndex(db_name, "person", "uid", true), true);
        // UT_EXPECT_EQ(client.AddVertexIndex(db_name, "person", "name", false), true);
        // UT_EXPECT_EQ(client.AddVertexIndex(db_name, "person", "age", false), true);
        // UT_EXPECT_EQ(client.AddVertexIndex(db_name, "software", "name", false), true);
        // UT_EXPECT_EQ(client.AddVertexIndex(db_name, "company", "address", false), true);

        client.AddUser("qw", true, "111");
    }
    UT_LOG() << "Testing index succeeded";

    test_client_other_branch(client);

    UT_LOG() << "\n" << __func__ << " succeeded";
    UT_LOG() << "\n" << __func__ << " exit";
}

void LoginAndSetHeader(http_client& client, http_request& request, const std::string& user,
                       const std::string& password) {
    web::json::value body;
    body[_TU("user")] = web::json::value::string(_TU(user));
    body[_TU("password")] = web::json::value::string(_TU(password));
    auto response = client.request(methods::POST, _TU("/login"), body).get();
    if (response.status_code() != status_codes::OK) {
        UT_LOG() << "DEBUG " << __LINE__ << "\n\n";
        throw std::runtime_error("authentication failed");
    }
    auto token = response.extract_json().get().at(_TU("jwt")).as_string();
    std::string author = "Bearer " + _TS(token);
    SetAuthInfo(author, request);
}

// test request different paths and branches
void test_client_other_branch(RestClient& rest_client) {
    http_client* client = (http_client*)rest_client.GetClient();
    const std::string db_name = "default";
    UT_LOG() << "\n=====cases of the excepted requestes=====";
    web::json::value body;
    body[_TU("user")] = web::json::value::string(_TU("qw"));
    body[_TU("password")] = web::json::value::string(_TU("111"));
    auto response = client->request(methods::POST, _TU("/login"), body).get();
    if (response.status_code() != status_codes::OK) {
        UT_LOG() << "DEBUG " << __LINE__ << "\n\n";
        throw std::runtime_error("authentication failed");
    }
    auto token = response.extract_json().get().at(_TU("jwt")).as_string();
    std::string author;
    author = "Bearer " + ToStdString(token);

    http_request request;
    SetAuthInfo(author, request);
    request.set_request_uri(_TU("/user"));
    request.set_method(methods::GET);
    response = client->request(request).get();
    UT_LOG() << _TS(response.to_string());
    UT_EXPECT_EQ(response.status_code(), status_codes::OK);

    request.headers().clear();
    request.headers().add(_TU("Content-Type"), _TU("application/json"));
    request.set_request_uri(_TU("/user"));
    request.set_method(methods::GET);
    response = client->request(request).get();
    UT_LOG() << _TS(response.to_string());
    UT_EXPECT_EQ(response.status_code(), status_codes::Unauthorized);

    SetAuthInfo(author, request);
    request.set_request_uri(_TU("/task"));
    request.set_method(methods::GET);
    response = client->request(request).get();
    UT_EXPECT_EQ(response.status_code(), status_codes::OK);

    SetAuthInfo(author, request);
    body[_TU("password")] = web::json::value::string(_TU("654321"));
    request.set_request_uri(_TU("/user"));
    request.set_method(methods::PUT);
    request.set_body(body);
    response = client->request(request).get();
    UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

    SetAuthInfo(author, request);
    request.set_request_uri(_TU("/user/admin"));
    request.set_method(methods::DEL);
    response = client->request(request).get();
    UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
    LoginAndSetHeader(*client, request, "qw", "111");

    request.set_request_uri(_TU("/info"));
    request.set_method(methods::GET);
    response = client->request(request).get();
    UT_EXPECT_EQ(response.status_code(), status_codes::OK);

    // test get
    {
        UT_LOG() << "\ntest get branch of excepiton";
        request.set_method(methods::GET);
        request.set_request_uri(_TU(std::string("/db/") + db_name + "/node/0"));
        response = client->request(request).get();
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

#if 0  // for ci when skip_web
        request.set_method(methods::GET);
        request.set_request_uri(_TU("/"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
        UT_LOG() << response.status_code() << " " << _TS(response.to_string());
#endif
        request.set_request_uri(
            _TU("/info/log/?begin_time=time1&end_time=time2&user=user1&num_log=100"));
        request.set_method(methods::GET);
        response = client->request(request).get();
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_request_uri(_TU("/info/log/2d"));
        request.set_method(methods::GET);
        response = client->request(request).get();
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/node"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/node/20"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/node/2"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(
            _TU("/db/" + db_name + "/node/3/relationship/4/0/properties/88/err"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/node/2/relationship/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << " " << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/node/20/properties/person/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/node/2/properties/person/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/node/0/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/label/error/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/label/relationship/knows"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/label/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/index/name?a=b&c=d"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/index/name/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/user/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.set_method(methods::GET);
        request.set_request_uri(_TU("/error/error"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        request.headers().add(_TU("server_version"), _TU("20000"));
        request.set_method(methods::GET);
        request.set_request_uri(_TU("/db/" + db_name + "/label"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::TemporaryRedirect);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());
    }

    // test post
    {
        UT_LOG() << "\ntest post branch of excepiton";

        request.set_method(methods::POST);
        request.set_request_uri(_TU("/"));
        response = client->request(request).get();
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        body.erase(_TU("user"));
        response = client->request(methods::POST, _TU("/login"), body).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        LoginAndSetHeader(*client, request, "qw", "111");
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

#ifndef _WIN32
        // less not readonly cyper
        body.erase(_TU("password"));

        body[_TU("script")] = web::json::value("MATCH (n) RETURN n,n.name LIMIT $param1");
        web::json::value json_value;
        json_value[_TU("$param44")] = web::json::value(true);
        body[_TU("parameters")] = json_value;
        request.set_method(methods::POST);
        request.set_request_uri(_TU("/cypher"));
        request.set_body(body);
        client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        UT_LOG() << response.status_code() << _TS(response.extract_json().get().serialize());

        body[_TU("script")] = web::json::value("MATCH (n)");
        json_value.erase(_TU("$param44"));
        json_value[_TU("$param1")] = web::json::value(_TU("testadba"));
        body.erase(_TU("parameters"));
        body[_TU("parameters")] = json_value;
        request.set_method(methods::POST);
        request.set_request_uri(_TU("/cypher"));
        request.set_body(body);
        client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
#endif

        {
            body = web::json::value();

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/node"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            body[_TU("label")] = web::json::value(_TU("error"));
            web::json::value test_json;
            test_json[_TU("name")] = web::json::value(_TU("20"));
            body[_TU("data")] = test_json;
            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/node"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        }
        {
            request.set_method(methods::POST);
            body.erase(_TU("label"));
            request.set_request_uri(_TU("/db/" + db_name + "/relationship"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            body.erase(_TU("data"));
            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/relationship"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        }
        {
            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/label"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/index"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/python_plugin"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/error"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + ("/misc/sub_graph")));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/error/sub_graph"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/node/0/relationship"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/node/0/relationship"));
            body[_TU("destination")] = web::json::value(5);
            body[_TU("label")] = web::json::value(_TU("created"));
            body[_TU("data")] = web::json::value::array();
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/db/" + db_name + "/error/0/relationship"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

            request.set_method(methods::POST);
            request.set_request_uri(_TU("/user"));
            request.set_body(body);
            client->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        }
    }

    // test delete
    {
        UT_LOG() << "\ntest delete branch of excepiton";

        LoginAndSetHeader(*client, request, "qw", "111");
        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/kkk"));
        response = client->request(request).get();
        UT_LOG() << response.status_code();

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/cpp_plugin/scan_graph"));
        response = client->request(request).get();
        UT_LOG() << response.status_code();

        // check import vertex (delete)
        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/node/5"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/node//"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/node/6"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/python_plugin/13"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::Unauthorized);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/error/11"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/index/software/lang"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/error/label/name"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/node/1/relationship/2/name"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/node/1/relationship//name"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/node/1/relationship/2/0"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/error/1/relationship/2/name"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // authority fail
        SetAuthInfo("error author", request);
        request.set_method(methods::DEL);
        request.set_request_uri(_TU("/db/" + db_name + "/"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::Unauthorized);
    }

    // test put
    {
        LoginAndSetHeader(*client, request, "qw", "111");

        UT_LOG() << "\ntest put branch of excepiton";
        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // body is empty
        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/node/1"));
        request.set_body(web::json::value());
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // body not emptiy
        web::json::value body_set, json_tmp;
        json_tmp[_TU("name1")] = web::json::value();
        body_set[_TU("data")] = json_tmp;
        body_set[_TU("label")] = web::json::value(_TU("person"));
        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/node/1"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // issue 875
        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/node//"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        body_set.erase(_TU("data"));
        body_set.erase(_TU("label"));
        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/user/wang"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/error/wang"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/node/3/relationship/4/0"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/node//relationship//"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/node/a3/relationship/4/0"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/node/3/relation/4/0"));
        request.set_body(body_set);
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // autority failed
        SetAuthInfo("error author", request);
        request.set_method(methods::PUT);
        request.set_request_uri(_TU("/db/" + db_name + "/"));
        response = client->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::Unauthorized);
    }

    UT_LOG() << "\n" << __func__ << " succeeded";
}
