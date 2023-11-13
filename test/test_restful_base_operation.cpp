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
#include "fma-common/string_formatter.h"
#include "fma-common/utils.h"
#include "gtest/gtest.h"

// The 'U' macro can be used to create a string or character literal of the platform type, i.e.
// utility::char_t. If you are using a library causing conflicts with 'U' macro, it can be turned
// off by defining the macro '_TURN_OFF_PLATFORM_STRING' before including the C++ REST SDK header
// files, and e.g. use '_XPLATSTR' instead.
#define _TURN_OFF_PLATFORM_STRING
#include "cpprest/http_client.h"
#include "restful/server/stdafx.h"
#include "client/cpp/restful/rest_client.h"
#include "server/lgraph_server.h"

#include "./graph_factory.h"
#include "./test_tools.h"
#include "./ut_utils.h"
#include "db/galaxy.h"
using namespace utility;               // Common utilities like string conversions
using namespace web;                   // Common features like URIs.
using namespace web::http;             // Common HTTP functionality
using namespace web::http::client;     // HTTP client features
using namespace concurrency::streams;  // Asynchronous streams

void test_client_other_branch(RestClient& client);

static void SetAuth(std::string header, http_request& request) {
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
class TestRestfulBaseOperation : public TuGraphTestWithParam<bool> {};

TEST_P(TestRestfulBaseOperation, RestfulBaseOperation) {
    using namespace fma_common;
    using namespace lgraph;
    bool enable_ssl = true;
    std::string host = "127.0.0.1";
    uint16_t port = 6464;
    std::string db_name = "default";
    std::shared_ptr<lgraph::GlobalConfig> gconfig = std::make_shared<lgraph::GlobalConfig>();
    enable_ssl = GetParam();
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
    std::string db_dir = "./testdb";

    // -----------------------------------
    // start server
    // build a test db
    AutoCleanDir _data_cleaner(db_dir);
    GraphFactory::create_modern(db_dir);
    // Build listener's URI from the configured address and the hard-coded path
    // "RestServer/Action"
#if 0
    lgraph::StateMachine::Config sm_config;
    sm_config.db_dir = "./testdb";
    lgraph::StateMachine state_machine(sm_config, &gconfig);
    auto res = state_machine.GetMasterRestAddr();
    UT_EXPECT_EQ(res, "");
    state_machine.Start();
    lgraph::RestServer::Config rest_config;
    rest_config.host = host;
    rest_config.port = port;
    rest_config.use_ssl = enable_ssl;
    gconfig.enable_audit_log = true;
    if (enable_ssl) {
        UT_EXPECT_ANY_THROW(lgraph::RestServer(&state_machine, rest_config, &gconfig));
        rest_config.server_key = key_path;
        UT_EXPECT_ANY_THROW(lgraph::RestServer(&state_machine, rest_config, &gconfig));
        rest_config.server_cert = cert_path;
    }
    // open the db_config api to set
    lgraph::RestServer rest_server(&state_machine, rest_config, &gconfig);
#else
    gconfig->bind_host = host;
    gconfig->http_port = port;
    gconfig->enable_ssl = enable_ssl;
    gconfig->enable_audit_log = true;
    gconfig->durable = true;
    gconfig->db_dir = db_dir;
    gconfig->server_key_file = key_path;
    gconfig->server_cert_file = cert_path;
    LGraphServer server(gconfig);
    server.Start();
#endif
    // -----------------------------------
    // server started, now start test
    std::string url;
    if (enable_ssl)
        url = fma_common::StringFormatter::Format("https://{}:{}/", host, port);
    else
        url = fma_common::StringFormatter::Format("http://{}:{}/", host, port);
    UT_LOG() << "  url:  " << url;

    UT_LOG() << "Testing login";
    // test invalid user login
    try {
        RestClient test_client(url, cert_path);
        json::value body;

        auto* client = static_cast<http_client*>(test_client.GetClient());
        // no user
        http_response response = client->request(methods::POST, _TU("/login"), body).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // no password
        body[_TU("user")] = json::value::string(_TU("admin"));
        response = client->request(methods::POST, _TU("/login"), body).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // invalid user name
        UT_EXPECT_ANY_THROW(test_client.Login("~`!@#$%^&*()-+=|\\;:'\",<>./?", "73@TuGraph"));
        // wrong password
        UT_EXPECT_ANY_THROW(test_client.Login("admin", "wrong_password"));
    } catch (std::exception& e) {
        UT_LOG() << e.what();
        UT_EXPECT_TRUE(false);
    }

    // now start client
    RestClient client(url, cert_path);
    client.Login(lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
    UT_LOG() << "Testing login succeeded";

    // server info
    UT_LOG() << "Testing ServerInfo";
    {
        RestClient::CPURate cpu_rate;
        RestClient::MemoryInfo memory_info;
        RestClient::DBSpace db_space;
        RestClient::DBConfig db_config;
        std::string lgraph_version;
        std::string node;
        std::string relationship;
        client.GetServerInfo(cpu_rate, memory_info, db_space, db_config, lgraph_version, node,
                             relationship);
        UT_EXPECT_EQ(cpu_rate.unit, "%");
        UT_EXPECT_EQ(memory_info.unit, "KB");
        UT_EXPECT_EQ(db_space.unit, "B");
        UT_EXPECT_EQ(db_config.valid, true);
        UT_EXPECT_EQ(db_config.enable_ip_check, false);
        UT_EXPECT_EQ(db_config.optimistic_txn, false);
        UT_EXPECT_EQ(db_config.enable_audit, true);
        UT_EXPECT_EQ(db_config.durable, true);
        UT_EXPECT_EQ(node, "/node");
        UT_EXPECT_EQ(relationship, "/relationship");

        http_client* hclient = static_cast<http_client*>(client.GetClient());
        const std::string db_name = "default";
        UT_LOG() << "\n=====get token of admin=====";
        json::value body;
        body[_TU("user")] = json::value::string(_TU("admin"));
        body[_TU("password")] = json::value::string(_TU("73@TuGraph"));
        auto response = hclient->request(methods::POST, _TU("/login"), body).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
        auto token = response.extract_json().get().at(_TU("jwt")).as_string();
        std::string author;
        author = "Bearer " + ToStdString(token);

        http_request request;
        SetAuth(author, request);
        request.set_request_uri(
            _TU("/info/"
                "log?begin_time=2018-09-10%2000:00:00&end_time=2020-09-10%2000:00:00&user=user1&"
                "num_log=100"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/cpu"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/disk"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/memory"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/db_space"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/db_config"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/peers"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/leader"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        SetAuth(author, request);
        request.set_request_uri(_TU("/info/statistics"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
    }
    UT_LOG() << "Testing ServerInfo succeeded";

    // schema
    UT_LOG() << "Testing schema";
    {
        std::vector<FieldSpec> fds;
        fds.emplace_back(FieldSpec("name", FieldType::STRING, false));
        fds.emplace_back(FieldSpec("address", FieldType::STRING, false));
        fds.emplace_back(FieldSpec("scale", FieldType::INT32, false));

        UT_EXPECT_EQ(client.AddVertexLabel(db_name, "company", fds, "name"), true);
        UT_EXPECT_ANY_THROW(client.AddVertexLabel(db_name, "company", fds, "name"));

        auto labels_v = client.ListVertexLabels(db_name);
        UT_EXPECT_EQ(labels_v.size(), 3);
        labels_v.clear();

        auto labels_e = client.ListEdgeLabels(db_name);
        UT_EXPECT_EQ(labels_e.size(), 2);
        labels_e.clear();

        auto schema = client.GetSchema(db_name, true, "person");
        UT_EXPECT_EQ(schema.size(), 3);
        schema.clear();

        schema = client.GetSchema(db_name, true, "software");
        UT_EXPECT_EQ(schema.size(), 3);
        schema.clear();

        schema = client.GetVertexSchema(db_name, "company");
        UT_EXPECT_EQ(schema.size(), 3);
        schema.clear();

        schema = client.GetSchema(db_name, false, "created");
        UT_EXPECT_EQ(schema.size(), 1);
        schema.clear();

        schema = client.GetEdgeSchema(db_name, "knows");
        UT_EXPECT_EQ(schema.size(), 2);
        schema.clear();

        UT_EXPECT_ANY_THROW(client.GetSchema(db_name, true, "person_erro"));
    }
    UT_LOG() << "Testing schema succeeded";

    // test refresh
    UT_LOG() << "Testing refresh";
    {
        auto* client1 = static_cast<http_client*>(client.GetClient());
        json::value body;
        body[_TU("user")] = json::value::string(_TU(lgraph::_detail::DEFAULT_ADMIN_NAME));
        body[_TU("password")] = json::value::string(_TU(lgraph::_detail::DEFAULT_ADMIN_PASS));
        auto re = client1->request(methods::POST, _TU("/login"), body).get();
        auto token = _TS(re.extract_json().get().at(_TU("jwt")).as_string());
        auto new_token = client.Refresh(token);
        UT_EXPECT_EQ(new_token != token, true);
    }
    UT_LOG() << "Testing refresh succeeded";

    // test update_token_time and get_token_time
    {
        auto* client1 = static_cast<http_client*>(client.GetClient());
        json::value body;
        body[_TU("user")] = json::value::string(_TU(lgraph::_detail::DEFAULT_ADMIN_NAME));
        body[_TU("password")] = json::value::string(_TU(lgraph::_detail::DEFAULT_ADMIN_PASS));
        auto re = client1->request(methods::POST, _TU("/login"), body).get();
        auto token = _TS(re.extract_json().get().at(_TU("jwt")).as_string());
        UT_EXPECT_EQ(client.GetTokenTime(token).first, 3600 * 24);
        UT_EXPECT_EQ(client.GetTokenTime(token).second, 3600 * 24);
        client.UpdateTokenTime(token, 200, 400);
        UT_EXPECT_EQ(client.GetTokenTime(token).first, 200);
        UT_EXPECT_EQ(client.GetTokenTime(token).second, 400);
    }

    // test logout
    UT_LOG() << "Testing logout";
    {
        auto* client1 = static_cast<http_client*>(client.GetClient());
        json::value body;
        body[_TU("user")] = json::value::string(_TU(lgraph::_detail::DEFAULT_ADMIN_NAME));
        body[_TU("password")] = json::value::string(_TU(lgraph::_detail::DEFAULT_ADMIN_PASS));
        auto response = client1->request(methods::POST, _TU("/login"), body).get();
        auto token = _TS(response.extract_json().get().at(_TU("jwt")).as_string());
        UT_EXPECT_EQ(client.Logout(token), true);
        UT_EXPECT_THROW_MSG(client.Logout(token), "Unauthorized: Authentication failed.");
        UT_EXPECT_THROW_MSG(client.Refresh(token), "Unauthorized: Authentication failed.");
        UT_EXPECT_THROW_MSG(client.EvalCypher(db_name, "CALL db.addIndex('person', 'age', false)"),
                            "Unauthorized: Authentication failed.");
    }
    UT_LOG() << "Testing logout succeeded";

    // index
    UT_LOG() << "Testing index";
    {
        client.Login(lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);

        UT_EXPECT_ANY_THROW(client.AddIndex(db_name, "person", "uid", 1));
        UT_EXPECT_EQ(client.AddIndex(db_name, "person", "name", 0), true);
        UT_EXPECT_EQ(client.AddIndex(db_name, "person", "age", 0), true);
        UT_EXPECT_EQ(client.AddIndex(db_name, "software", "name", 0), true);
        UT_EXPECT_EQ(client.AddIndex(db_name, "company", "address", 0), true);
        UT_EXPECT_ANY_THROW(client.AddIndex(db_name, "company", "address", 0));
        UT_EXPECT_ANY_THROW(client.AddIndex(db_name, "error", "address", 0));
        UT_EXPECT_ANY_THROW(client.AddIndex(db_name, "company", "error", 0));
#ifdef _WIN32
// TODO(hjk41): fix this on windows
#else
        client.DeleteIndex(db_name, "company", "address");
        UT_EXPECT_ANY_THROW(client.AddIndex(db_name, "error", "address", 0));

        auto indexes = client.ListIndexes(db_name);
        UT_EXPECT_EQ(indexes.size(), 6);
        indexes.clear();

        client.DeleteIndex(db_name, "software", "name");

        indexes = client.ListIndexes(db_name);
        UT_EXPECT_EQ(indexes.size(), 5);
        indexes.clear();

        auto index_of_labels = client.ListIndexesAboutLabel(db_name, "person");
        UT_EXPECT_EQ(index_of_labels.size(), 3);
        index_of_labels.clear();

        auto v1 = client.GetVidsByIndex(db_name, "person", "name", "peter");
        UT_EXPECT_EQ(v1.size(), 1);
        UT_EXPECT_EQ(v1[0], 5);
        v1.clear();

        auto v2 = client.GetVidsByIndex(db_name, "person", "age", "32");
        UT_EXPECT_EQ(v2.size(), 1);
        UT_EXPECT_EQ(v2[0], 3);
        v2.clear();
#endif
    }
    UT_LOG() << "Testing index succeeded";

    // vertex
    UT_LOG() << "Testing vertex";
    {
        std::vector<std::vector<FieldData>> field_values_vector;
        field_values_vector.push_back(std::vector<FieldData>{
            FieldData((int64_t)1007), FieldData("qin"), FieldData((int64_t)32)});
        field_values_vector.push_back(std::vector<FieldData>{
            FieldData((int64_t)1008), FieldData("wei"), FieldData((int64_t)32)});
        field_values_vector.push_back(std::vector<FieldData>{
            FieldData((int64_t)1009), FieldData("qw"), FieldData((int64_t)32)});

        auto ids = client.AddVertexes(
            db_name, "person", std::vector<std::string>{"uid", "name", "age"}, field_values_vector);
        UT_EXPECT_EQ(ids.size(), 3);
        UT_EXPECT_EQ(ids[0], 6);
        UT_EXPECT_EQ(ids[1], 7);
        UT_EXPECT_EQ(ids[2], 8);
        ids.clear();

        auto id =
            client.AddVertex(db_name, "person", std::vector<std::string>{"uid", "name", "age"},
                             std::vector<FieldData>{FieldData((int64_t)1010), FieldData("lebron"),
                                                    FieldData((int64_t)32)});
        UT_EXPECT_EQ(id, 9);
        id =
            client.AddVertex(db_name, "software", std::vector<std::string>({"uid", "lang", "name"}),
                             std::vector<FieldData>{FieldData((int64_t)1011), FieldData("python"),
                                                    FieldData("django")});
        UT_EXPECT_EQ(id, 10);

        auto v2 = client.GetVidsByIndex(db_name, "person", "age", "32");
        UT_EXPECT_EQ(v2.size(), 5);
        UT_EXPECT_EQ(v2[0], 3);
        UT_EXPECT_EQ(v2[1], 6);
        UT_EXPECT_EQ(v2[2], 7);
        UT_EXPECT_EQ(v2[3], 8);
        UT_EXPECT_EQ(v2[4], 9);
        v2.clear();

        auto field = client.GetVertexField(db_name, 2, std::string("name_err"));
        UT_EXPECT_EQ(field.type, FieldType::NUL);

        field = client.GetVertexField(db_name, 2, std::string("name"));
        UT_EXPECT_EQ(field.ToString(), "lop");

        auto fields = client.GetVertexFields(db_name, id);
        UT_EXPECT_EQ(fields.size(), 3);
        UT_EXPECT_EQ(fields["lang"].AsString(), "python");
        UT_EXPECT_EQ(fields["name"].AsString(), "django");
        UT_EXPECT_EQ(fields["uid"].AsInt64(), 1011);
        fields.clear();

        client.SetVertexProperty(
            db_name, id, std::vector<std::string>({"uid", "name"}),
            std::vector<FieldData>{FieldData((int64_t)1012), FieldData("chrome")});
        std::string label;
        fields = client.GetVertex(db_name, id, label);
        UT_EXPECT_EQ(label, "software");
        UT_EXPECT_EQ(fields["uid"].AsInt64(), 1012);
        UT_EXPECT_EQ(fields["name"].AsString(), "chrome");
        fields.clear();

        size_t n_in, n_out;
        client.DeleteVertex(db_name, 9, &n_in, &n_out);
        UT_EXPECT_EQ(n_in, 0);
        UT_EXPECT_EQ(n_out, 0);
    }
    UT_LOG() << "Testing vertex succeeded";

    // edge
    UT_LOG() << "Testing edge";
    {
        EdgeUid e =
            client.AddEdge(db_name, 1, 0, "knows", std::vector<std::string>{"weight", "since"},
                           std::vector<FieldData>{FieldData(0.9), FieldData((int64_t)2015)});
        UT_EXPECT_TRUE(e == EdgeUid(1, 0, 0, 0, 0));
        e = client.AddEdge(db_name, 1, 0, "knows", std::vector<std::string>{"weight", "since"},
                           std::vector<FieldData>{FieldData(2.7), FieldData((int64_t)2015)});
        UT_EXPECT_TRUE(e == EdgeUid(1, 0, 0, 0, 1));

        std::vector<std::vector<FieldData>> field_values_vector;
        std::vector<std::pair<int64_t, int64_t>> edges;
        edges.emplace_back(5, 6);
        edges.emplace_back(5, 7);
        edges.emplace_back(5, 8);
        field_values_vector.push_back(
            std::vector<FieldData>{FieldData(0.9), FieldData((int64_t)2015)});
        field_values_vector.push_back(
            std::vector<FieldData>{FieldData(0.9), FieldData((int64_t)2015)});
        field_values_vector.push_back(
            std::vector<FieldData>{FieldData(0.9), FieldData((int64_t)2015)});
        auto edgeUids =
            client.AddEdges(db_name, "knows", std::vector<std::string>{"weight", "since"}, edges,
                            field_values_vector);
        UT_EXPECT_EQ(edgeUids.size(), 3);
        UT_EXPECT_TRUE(edgeUids[0] == EdgeUid(5, 6, 0, 0, 0));
        UT_EXPECT_TRUE(edgeUids[1] == EdgeUid(5, 7, 0, 0, 0));
        UT_EXPECT_TRUE(edgeUids[2] == EdgeUid(5, 8, 0, 0, 0));
        edgeUids.clear();

        auto in_edges = client.ListInEdges(db_name, 3);
        UT_EXPECT_EQ(in_edges.size(), 1);
        UT_EXPECT_TRUE(in_edges[0] == EdgeUid(0, 3, 0, 0, 0));
        in_edges.clear();
        auto out_edges = client.ListOutEdges(db_name, 3);
        UT_EXPECT_EQ(out_edges.size(), 2);
        UT_EXPECT_TRUE(out_edges[0] == EdgeUid(3, 2, 1, 0, 0));
        UT_EXPECT_TRUE(out_edges[1] == EdgeUid(3, 4, 1, 0, 0));
        out_edges.clear();
        auto all_edges = client.ListAllEdges(db_name, 3);
        UT_EXPECT_EQ(all_edges.size(), 2);
        UT_EXPECT_EQ(all_edges[0].size(), 1);
        UT_EXPECT_EQ(all_edges[1].size(), 2);
        UT_EXPECT_TRUE(all_edges[0][0] == EdgeUid(0, 3, 0, 0, 0));
        UT_EXPECT_TRUE(all_edges[1][0] == EdgeUid(3, 2, 1, 0, 0));
        UT_EXPECT_TRUE(all_edges[1][1] == EdgeUid(3, 4, 1, 0, 0));
        all_edges[0].clear();
        all_edges[1].clear();
        all_edges.clear();

        auto fields = client.GetEdgeFields(db_name, EdgeUid(1, 0, 0, 0, 0));
        UT_EXPECT_TRUE(fabs(fields["weight"].AsDouble() - 0.9) < 0.000001);
        UT_EXPECT_EQ(fields["since"].AsInt64(), 2015);
        fields.clear();
        auto field = client.GetEdgeField(db_name, EdgeUid(1, 0, 0, 0, 0), "since");
        UT_EXPECT_EQ(field.integer(), 2015);

        client.SetEdgeProperty(db_name, EdgeUid(1, 0, 0, 0, 0), std::vector<std::string>{"since"},
                               std::vector<FieldData>{FieldData((int64_t)2019)});

        std::string label;
        fields = client.GetEdge(db_name, EdgeUid(1, 0, 0, 0, 0), label);
        UT_EXPECT_EQ(label, "knows");
        UT_EXPECT_EQ(fields["since"].AsInt64(), 2019);
        fields.clear();

        client.DeleteEdge(db_name, EdgeUid(3, 2, 1, 0, 0));
        UT_EXPECT_ANY_THROW(client.GetEdgeFields(db_name, EdgeUid(3, 2, 1, 0, 0)));

        all_edges = client.ListAllEdges(db_name, 3);
        UT_EXPECT_EQ(all_edges.size(), 2);
        UT_EXPECT_EQ(all_edges[0].size(), 1);
        UT_EXPECT_EQ(all_edges[1].size(), 1);
        UT_EXPECT_TRUE(all_edges[0][0] == EdgeUid(0, 3, 0, 0, 0));
        UT_EXPECT_TRUE(all_edges[1][0] == EdgeUid(3, 4, 1, 0, 0));
        all_edges[0].clear();
        all_edges[1].clear();
        all_edges.clear();

        size_t n_in, n_out;
        client.DeleteVertex(db_name, 1, &n_in, &n_out);

        UT_EXPECT_ANY_THROW(auto oute = client.ListOutEdges(db_name, 1));

        auto oute = client.ListOutEdges(db_name, 0);
        for (auto& oe : oute) {
            if (oe.dst == 1) ERR() << "  delete vertex err!";
        }

        UT_EXPECT_ANY_THROW(auto f = client.GetVertexField(db_name, 1, std::string("name")));
        UT_EXPECT_ANY_THROW(client.GetEdgeFields(db_name, EdgeUid(1, 0, 0, 0, 0)));
        UT_EXPECT_ANY_THROW(client.SetEdgeProperty(
            db_name, EdgeUid(1, 0, 0, 0, 1), std::vector<std::string>{"weight", "since"},
            std::vector<FieldData>{FieldData(2.0), FieldData((int64_t)1315)}));
    }
    UT_LOG() << "Testing Edge succeeded";

    UT_LOG() << "Testing Memory Limit";

#ifndef _WIN32
    // cypher
    UT_LOG() << "Testing Cypher";
    {
        std::string script_str("MATCH (n) RETURN n,n.name LIMIT 10");
        auto ret = client.EvalCypher(db_name, script_str);
        UT_EXPECT_EQ(ret[_TU("size")].as_number().to_int32(), 9);
        UT_LOG() << "  " << _TS(ret.serialize());

        UT_EXPECT_ANY_THROW(client.EvalCypher(db_name, "CALL db.addIndex('person', 'age', false)"));
        script_str.assign("MATCH (n:person {age:$param1}) RETURN n,n.name");
        std::map<std::string, FieldData> param;
        param["$param1"] = FieldData((int64_t)32);
        ret = client.EvalCypherWithParam(db_name, script_str, param);
        UT_EXPECT_EQ(ret[_TU("size")].as_number().to_int32(), 4);
        UT_LOG() << "  " << _TS(ret.serialize());

        script_str.assign("WITH [1,2,3] AS x RETURN x");
        ret = client.EvalCypher(db_name, script_str);
        UT_EXPECT_EQ(ret[_TU("size")].as_number().to_int32(), 1);
        UT_LOG() << "  " << _TS(ret.serialize());

        std::vector<int64_t> vec_vertex{2, 4, 3};
        ret = client.GetSubGraph(db_name, vec_vertex);
        UT_LOG() << "  " << _TS(ret.serialize());
        // set parameters online
        script_str.assign(
            UT_FMT("call dbms.config.update(\\{{}:true, {}:false, {}:true, {}:true\\})",
                   lgraph::_detail::OPT_IP_CHECK_ENABLE, lgraph::_detail::OPT_DB_DURABLE,
                   lgraph::_detail::OPT_TXN_OPTIMISTIC, lgraph::_detail::OPT_AUDIT_LOG_ENABLE));
        ret = client.EvalCypher(db_name, script_str);
        RestClient::CPURate cpuRate;
        RestClient::MemoryInfo memInfo;
        RestClient::DBSpace dbSpace;
        RestClient::DBConfig dbConfig;
        std::string lgraph_version;
        std::string node;
        std::string relationship;

        client.Login(lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
        client.GetServerInfo(cpuRate, memInfo, dbSpace, dbConfig, lgraph_version, node,
                             relationship);
        UT_LOG() << dbConfig.enable_ip_check << ":" << dbConfig.optimistic_txn << ":"
                 << dbConfig.enable_audit << ":" << dbConfig.durable;
        UT_EXPECT_EQ(dbConfig.enable_ip_check, true);
        UT_EXPECT_EQ(dbConfig.optimistic_txn, true);
        UT_EXPECT_EQ(dbConfig.enable_audit, true);
        UT_EXPECT_EQ(dbConfig.durable, false);
    }
    UT_LOG() << "Testing Cypher succeeded";
#endif

    const std::string& admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string& admin_pass = lgraph::_detail::DEFAULT_ADMIN_PASS;
    const std::string& admin_role = lgraph::_detail::ADMIN_ROLE;

    // user management
    UT_LOG() << "Testing user";
    {
        client.AddUser("guest1", false, "pd111");

        auto users = client.ListUsers();
        UT_EXPECT_EQ(users.size(), 2);
        users.clear();

        UT_EXPECT_ANY_THROW(client.SetUserRoles("no-such-user", {}));
        client.Login(admin_user, admin_pass);

        client.AddUser("guest2", false, "pd111");
        users = client.ListUsers();
        UT_EXPECT_EQ(users.size(), 3);
        users.clear();

        client.SetPassword("guest1", "pd111", "pd111111");
        client.Login(admin_user, admin_pass);

        users = client.ListUsers();
        UT_EXPECT_EQ(users.size(), 3);
        users.clear();

        client.DeleteUser("guest1");
        client.Login(admin_user, admin_pass);

        users = client.ListUsers();
        UT_EXPECT_EQ(users.size(), 2);
        users.clear();

        client.AddUser("qw", false, "111");

        {
            RestClient c2(url, cert_path);
            UT_EXPECT_TRUE(c2.Login("qw", "111"));
            UT_EXPECT_ANY_THROW(c2.ListUsers());
            UT_EXPECT_ANY_THROW(c2.AddUser("some_user", true, "pass"));
            UT_EXPECT_ANY_THROW(c2.DeleteUser("guest2"));
            UT_EXPECT_ANY_THROW(c2.SetUserRoles("qw", {admin_role}));  // cannot elevate yourself
            c2.SetPassword("qw", "111", "222");
            c2.Login("qw", "222");
            UT_EXPECT_ANY_THROW(c2.SetPassword("qw", "111", "222"));
            UT_EXPECT_ANY_THROW(c2.SetPassword("qw", "", "222"));
            UT_EXPECT_ANY_THROW(c2.SetPassword("guest1", "pd111", "222"));
            UT_EXPECT_ANY_THROW(c2.DeleteUser("qw"));  // cannot delete yourself
            c2.SetPassword("qw", "222", "111");
            c2.Login("qw", "111");

            // test normal user operate database
            auto* client1 = static_cast<http_client*>(c2.GetClient());
            json::value body;
            body[_TU("user")] = json::value::string(_TU("qw"));
            body[_TU("password")] = json::value::string(_TU("111"));
            auto response = client1->request(methods::POST, _TU("/login"), body).get();
            if (response.status_code() != status_codes::OK) {
                UT_LOG() << "DEBUG " << __LINE__ << "\n\n";
                throw std::runtime_error("authentication failed");
            }
            auto token = response.extract_json().get().at(_TU("jwt")).as_string();
            std::string author;
            author = "Bearer " + ToStdString(token);

            http_request request;
            SetAuth(author, request);
            request.set_request_uri(_TU("/task/9_1"));
            request.set_method(methods::DEL);
            response = client1->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::Unauthorized);

            request.set_request_uri(_TU("/task"));
            request.set_method(methods::GET);
            response = client1->request(request).get();
            UT_EXPECT_EQ(response.status_code(), status_codes::Unauthorized);
        }

        client.Login("admin", "73@TuGraph");
        client.SetUserRoles("qw", {admin_role});
        client.Login("admin", "73@TuGraph");
        UT_EXPECT_EQ(client.ListUsers().size(), 3);
    }
    UT_LOG() << "Testing user succeeded";

#ifndef _WIN32
    // plugin
    UT_LOG() << "Testing plugin";
    {
        // py plugin load
        std::string code = lgraph_api::base64::Decode(
            "IyEvdXNyL2Jpbi9weXRob24KIyAtKi0gY29kaW5nOiBVVEYtOCAtKgoKZGVmIFByb2Nlc3MoZGIsIGlucH"
            "V0KT"
            "oKICAgIHJldHVybiAoVHJ1ZSwgIlRlc3QgcHl0aG9uIHBsdWdpblxuIikK");
        std::string codeNeedInput = lgraph_api::base64::Decode(
            "IyEvdXNyL2Jpbi9weXRob24KIyAtKi0gY29kaW5nOiBVVEYtOCAtKgoKZGVmIFByb2Nlc3MoZGIsIGlucH"
            "V0KT"
            "oKICAgIHJldHVybiAoVHJ1ZSwgaW5wdXQpCg==");

#if LGRAPH_ENABLE_PYTHON_PLUGIN
        UT_EXPECT_EQ(client.LoadPlugin(db_name, lgraph_api::PluginCodeType::PY,
                                       PluginDesc("py_test", lgraph::plugin::PLUGIN_CODE_TYPE_PY,
                                                  "test python plugin ECHO",
                                                  lgraph::plugin::PLUGIN_VERSION_1, true, ""),
                                       code),
                     true);
        UT_EXPECT_EQ(
            client.LoadPlugin(db_name, lgraph_api::PluginCodeType::PY,
                              PluginDesc("py_test_input", lgraph::plugin::PLUGIN_CODE_TYPE_PY,
                                         "test python plugin ECHO with input",
                                         lgraph::plugin::PLUGIN_VERSION_1, false, ""),
                              codeNeedInput),
            true);

        UT_EXPECT_ANY_THROW(client.LoadPlugin(
            db_name, lgraph_api::PluginCodeType::PY,
            PluginDesc("py_test", lgraph::plugin::PLUGIN_CODE_TYPE_PY, "test python plugin ECHO",
                       lgraph::plugin::PLUGIN_VERSION_1, true, ""),
            code));
        // cpp
        auto plugins = client.GetPlugin(db_name, true);
        UT_EXPECT_EQ(plugins.size(), 0);
        plugins.clear();
        // py
        plugins = client.GetPlugin(db_name, false);
        UT_EXPECT_EQ(plugins.size(), 2);
        UT_EXPECT_EQ(plugins[0].name, "py_test");
        UT_EXPECT_EQ(plugins[1].name, "py_test_input");
        UT_EXPECT_EQ(plugins[0].desc, "test python plugin ECHO");
        UT_EXPECT_EQ(plugins[1].desc, "test python plugin ECHO with input");
        UT_EXPECT_EQ(plugins[0].read_only, true);
        UT_EXPECT_EQ(plugins[1].read_only, false);
        plugins.clear();

        UT_LOG() << "Testing plugin get detail";
        auto plugin = client.GetPluginDetail(db_name, "py_test", false);
        UT_EXPECT_EQ(plugin.name, "py_test");
        UT_EXPECT_EQ(plugin.desc, "test python plugin ECHO");
        UT_EXPECT_EQ(plugin.read_only, true);
        UT_EXPECT_EQ(plugin.code_type, "py");
        UT_EXPECT_EQ(plugin.code, lgraph_api::base64::Encode(code));
        UT_EXPECT_EQ(client.ExecutePlugin(db_name, false, "py_test"), "Test python plugin\n");
        UT_EXPECT_EQ(client.ExecutePlugin(db_name, false, "py_test_input",
                                          "test python plugin with input.\n"),
                     "test python plugin with input.\n");
        UT_EXPECT_ANY_THROW(client.ExecutePlugin(db_name, false, "py_test_not_exist"));
        client.DeletePlugin(db_name, false, "py_test_input");

        plugins = client.GetPlugin(db_name, false);
        UT_EXPECT_EQ(plugins.size(), 1);
        UT_EXPECT_EQ(plugins[0].name, "py_test");
        UT_EXPECT_EQ(plugins[0].read_only, true);
        UT_EXPECT_EQ(plugins[0].desc, "test python plugin ECHO");
        plugins.clear();

        UT_EXPECT_EQ(client.ExecutePlugin(db_name, false, "py_test"), "Test python plugin\n");
#endif
    }
    UT_LOG() << "Testing plugin succeeded";
#endif

    UT_LOG() << "Test Graph ACL";
    {
        http_client* client1 = static_cast<http_client*>(client.GetClient());
        json::value body;
        body[_TU("user")] = json::value::string(_TU(admin_user));
        body[_TU("password")] = json::value::string(_TU(admin_pass));
        auto response = client1->request(methods::POST, _TU("/login"), body).get();
        if (response.status_code() != status_codes::OK) {
            UT_LOG() << "DEBUG " << __LINE__ << "\n\n";
            throw std::runtime_error("authentication failed");
        }
        auto token = response.extract_json().get().at(_TU("jwt")).as_string();
        std::string author;
        author = "Bearer " + ToStdString(token);

        json::value conf;
        conf[_TU("max_size_GB")] = json::value(204);
        conf[_TU("async")] = json::value(false);
        body = json::value();
        body[_TU("name")] = json::value::string(_TU("graph1"));
        body[_TU("config")] = conf;
        http_request request;
        SetAuth(author, request);
        request.set_request_uri(_TU("/db"));
        request.set_method(methods::POST);
        request.set_body(body);
        response = client1->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        SetAuth(author, request);
        request.set_request_uri(_TU("/db"));
        request.set_method(methods::POST);
        request.set_body(body);
        response = client1->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        body.erase(_TU("name"));
        SetAuth(author, request);
        request.set_request_uri(_TU("/db"));
        request.set_method(methods::POST);
        request.set_body(body);
        response = client1->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);

        request.set_request_uri(_TU("/db/graph1"));
        SetAuth(author, request);
        request.set_method(methods::DEL);
        response = client1->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        request.set_request_uri(_TU("/db/graph1"));
        request.set_method(methods::DEL);
        SetAuth(author, request);
        response = client1->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // delete task
        request.set_request_uri(_TU("/task/9_1"));
        request.set_method(methods::DEL);
        SetAuth(author, request);
        response = client1->request(request).get();
        // waiting for the branch fix_delete_task merged.
        UT_EXPECT_EQ(response.status_code(), status_codes::BadRequest);
        // options
        request.set_request_uri(_TU(""));
        request.set_method(methods::OPTIONS);
        SetAuth(author, request);
        response = client1->request(request).get();

        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
    }
    UT_LOG() << "Test Graph ACL Succeeded ";

    UT_LOG() << "Test Graph DB Desc";
    {
        http_client* client1 = static_cast<http_client*>(client.GetClient());
        json::value body;
        body[_TU("user")] = json::value::string(_TU(admin_user));
        body[_TU("password")] = json::value::string(_TU(admin_pass));
        auto response = client1->request(methods::POST, _TU("/login"), body).get();
        if (response.status_code() != status_codes::OK) {
            UT_LOG() << "DEBUG " << __LINE__ << "\n\n";
            throw std::runtime_error("authentication failed");
        }
        auto token = response.extract_json().get().at(_TU("jwt")).as_string();
        std::string author;
        author = "Bearer " + ToStdString(token);

        json::value conf;
        conf[_TU("max_size_GB")] = json::value(204);
        conf[_TU("async")] = json::value(false);
        conf[_TU("description")] = json::value::string(_TU("graph desc"));
        body = json::value();
        body[_TU("name")] = json::value::string(_TU("graph1"));
        body[_TU("config")] = conf;

        http_request request;
        SetAuth(author, request);
        request.set_request_uri(_TU("/db"));
        request.set_method(methods::POST);
        request.set_body(body);
        response = client1->request(request).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        request.set_request_uri(_TU("/db/graph1"));
        SetAuth(author, request);
        request.set_method(methods::GET);
        response = client1->request(request).get();
        auto res = response.extract_json().get();
        UT_EXPECT_EQ(_TS(res.at(_TU("description")).as_string()), "graph desc");
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
    }
    UT_LOG() << "Test Graph DB Desc Succeeded";
    UT_LOG() << "\n" << __func__ << " succeeded";
    UT_LOG() << "\n" << __func__ << " exit";
    SleepS(3);  // waiting for memory reclaiming by async task
    server.Stop();
}

INSTANTIATE_TEST_CASE_P(TestRestfulBaseOperation, TestRestfulBaseOperation,
                        testing::Values(true, false));
