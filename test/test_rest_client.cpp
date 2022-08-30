/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/configuration.h"
#include "fma-common/logger.h"

#include "gtest/gtest.h"
#include "client/cpp/restful/rest_client.h"
#include "server/lgraph_server.h"

#include "./graph_factory.h"
#include "./test_tools.h"
#include "./ut_utils.h"

class TestRestClient : public TuGraphTestWithParam<bool> {};

TEST_P(TestRestClient, RestClient) {
    _ParseTests(&_ut_argc, &_ut_argv);
    const std::string& admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string& admin_pass = lgraph::_detail::DEFAULT_ADMIN_PASS;
    const std::string& admin_role = lgraph::_detail::ADMIN_ROLE;

    const std::string new_user = "new_user";
    const std::string new_pass = "new_pass";
    const std::string new_role = "new_role";
    const std::string new_desc1 = "description for user";
    const std::string new_desc2 = "modified description";

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
    std::string url = UT_FMT("http://{}:{}", host, port);
    if (enable_ssl) {
        lgraph::WriteCertFiles(cert_path, key_path);
        url = UT_FMT("https://{}:{}", host, port);
    }
    std::string db_dir = "./testdb";
    gconfig->bind_host = host;
    gconfig->http_port = port;
    gconfig->enable_ssl = enable_ssl;
    gconfig->enable_audit_log = true;
    gconfig->db_dir = db_dir;
    gconfig->server_key_file = key_path;
    gconfig->server_cert_file = cert_path;

    auto StartEmptyServer = [&]() {
        std::unique_ptr<LGraphServer> server(new LGraphServer(gconfig));
        server->Start();
        return server;
    };

    auto GetRestClient = [&]() {
        std::unique_ptr<RestClient> client(new RestClient(url, gconfig->server_cert_file));
        return client;
    };

    std::unique_ptr<LGraphServer> server;
    std::unique_ptr<RestClient> client;
    auto Setup = [&](bool login_as_admin = true) {
        client.reset();
        if (server) server->Stop();
        server.reset();
        fma_common::file_system::RemoveDir(gconfig->db_dir);
        server = StartEmptyServer();
        client = GetRestClient();
        client->Login(admin_user, admin_pass);
    };

    DefineTest("DeleteAdminRole") {
        Setup();
        UT_EXPECT_THROW(
            {
                try {
                    client->DeleteRole(admin_role);
                } catch (std::exception& e) {
                    // and this tests that it has the correct message
                    std::string result = e.what();
                    UT_EXPECT_NE(result.find("cannot"), std::string::npos);
                    throw;
                }
            },
            std::exception);
    }

    DefineTest("CreateUser") {
        Setup();
        client->AddUser(new_user, false, new_pass, new_desc1);
        auto uinfo = client->GetUserInfo(new_user);
        UT_EXPECT_EQ(uinfo.desc, new_desc1);
        UT_EXPECT_TRUE(uinfo.roles == std::set<std::string>({new_user}));
        UT_EXPECT_TRUE(!uinfo.disabled);
    }

    DefineTest("ModUser") {
        Setup();
        client->AddUser(new_user, false, new_pass, new_desc1);
        UT_EXPECT_EQ(client->GetUserInfo(new_user).desc, new_desc1);
        client->SetUserDesc(new_user, new_desc2);
        UT_EXPECT_EQ(client->GetUserInfo(new_user).desc, new_desc2);
    }
}

INSTANTIATE_TEST_CASE_P(TestRestClient, TestRestClient, testing::Values(true, false));
