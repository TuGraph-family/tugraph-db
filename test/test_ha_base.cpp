/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include <omp.h>
#include <mutex>
#include <condition_variable>
#include <utility>
#include "gtest/gtest.h"

// The 'U' macro can be used to create a string or character literal of the platform type, i.e.
// utility::char_t. If you are using a library causing conflicts with 'U' macro, it can be turned
// off by defining the macro '_TURN_OFF_PLATFORM_STRING' before including the C++ REST SDK header
// files, and e.g. use '_XPLATSTR' instead.
#define _TURN_OFF_PLATFORM_STRING
#include "cpprest/http_client.h"

#include "fma-common/logger.h"
#include "./ut_utils.h"
#include "lgraph/lgraph.h"
#include "client/cpp/restful/rest_client.h"
#include "lgraph/lgraph_rpc_client.h"
#include "server/lgraph_server.h"

void build_ha_so() {
    const std::string INCLUDE_DIR = "../../include";
    const std::string DEPS_INCLUDE_DIR = "../../deps/install/include";
    const std::string LIBLGRAPH = "./liblgraph.so";
    int rt;
#ifndef __clang__
    std::string cmd_f =
        "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#elif __APPLE__
    std::string cmd_f =
        "clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -Xpreprocessor "
        "-fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#else
    std::string cmd_f =
        "clang++ -stdlib=libc++ -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#endif
    std::string cmd;
    cmd = FMA_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./sortstr.so",
                 "../../test/test_procedures/sortstr.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = FMA_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./scan_graph.so",
                 "../../test/test_procedures/scan_graph.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = FMA_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./echo_binary.so",
                 "../../test/test_procedures/echo_binary.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = FMA_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./add_label.so",
                 "../../test/test_procedures/add_label_v.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
}

class UnitTestBase {
    std::map<std::string, std::function<int()>> tests_;

 public:
    UnitTestBase() = default;
    virtual ~UnitTestBase() = default;
    void Run(const std::string& includes, const std::string& excludes) {
        auto incs = fma_common::Split(includes, ",");
        std::set<std::string> included(incs.begin(), incs.end());
        auto excs = fma_common::Split(excludes, ",");
        std::set<std::string> excluded(excs.begin(), excs.end());

        std::set<std::string> tests;
        if (included.empty())
            for (auto& kv : tests_) tests.insert(kv.first);
        else
            tests = included;
        for (auto& t : excluded) {
            if (tests.erase(t) == 0) {
                FMA_WARN() << "Excluded test [" << t << "] is not in the test list.";
            }
        }
        for (auto& t : tests) {
            auto it = tests_.find(t);
            UT_EXPECT_TRUE(it != tests_.end());
            // run test
            UT_LOG() << "\n------------------";
            UT_LOG() << "Testing " << t;
            UT_LOG() << "------------------";
            int r = it->second();
            UT_LOG() << "\n========";
            if (r == 0)
                UT_LOG() << "SUCCESS";
            else
                UT_LOG() << "FAIL";
            UT_LOG() << "========";
        }
    }

 protected:
    void RegisterTest(const std::string& name, const std::function<int()>& func) {
        auto it = tests_.find(name);
        UT_EXPECT_TRUE(it == tests_.end());
        tests_.emplace_hint(it, name, func);
    }
};

#define FMA_REG_TEST(method_name) RegisterTest(#method_name, [this]() { return method_name(); })

bool read_only_plugin = false;
bool use_rest = true;
size_t n_ops = 300;
std::string plugin_name = "scan";  // NOLINT

class HaUnitTest : public UnitTestBase {
 public:
    std::unique_ptr<lgraph::LGraphServer> server_;
    std::shared_ptr<lgraph::GlobalConfig> config_;
    std::vector<std::unique_ptr<RestClient>> rests_;
    std::vector<std::unique_ptr<lgraph::RpcClient>> rpcs_;

    explicit HaUnitTest(std::shared_ptr<lgraph::GlobalConfig> config, int n_clients = 16)
        : config_(std::move(config)), rests_(n_clients), rpcs_(n_clients) {
        omp_set_num_threads(n_clients);
        FMA_REG_TEST(AddLabel);
        FMA_REG_TEST(AddGraph);
        FMA_REG_TEST(Plugin);
        FMA_REG_TEST(GetHaInfo);
    }

    int AddLabel() {
        UT_EXPECT_TRUE(rests_[0]->AddVertexLabel(
            "default", "v",
            std::vector<lgraph_api::FieldSpec>{{"id", lgraph_api::FieldType::STRING, false}},
            "id"));
        // FMA_ASSERT(rests_[0]->AddVertexIndex("default", "v", "id", true));
        int64_t vid;
#pragma omp parallel for
        for (size_t i = 0; i < 1; i++) {
            vid = rests_[omp_get_thread_num()]->AddVertex(
                "default", "v", std::vector<std::string>{"id"}, std::vector<std::string>{"001"});
        }
        auto fields = rests_[0]->GetVertexFields("default", vid);
        UT_EXPECT_EQ(fields["id"].AsString(), "001");
        return 0;
    }

    int GetHaInfo() {
        using namespace web;
        using namespace http;
        using namespace client;
        auto* hclient = static_cast<http_client*>(rests_[0]->GetClient());
        const std::string db_name = "default";
        UT_LOG() << "\n=====get token of admin=====";
        web::json::value body;
        body[_TU("user")] = web::json::value::string(_TU("admin"));
        body[_TU("password")] = web::json::value::string(_TU("73@TuGraph"));
        auto response = hclient->request(methods::POST, _TU("/login"), body).get();
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
        auto token = response.extract_json().get().at(_TU("jwt")).as_string();
        std::string author;
        author = "Bearer " + ToStdString(token);

        http_request request;
        request.headers().clear();
        request.headers().add(_TU("Authorization"), _TU(author));
        request.headers().add(_TU("Content-Type"), _TU("application/json"));
        request.set_request_uri(_TU("/info/peers"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_LOG() << "+++++++++++++++++++++++++++++++++++++";
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);

        request.headers().clear();
        request.headers().add(_TU("Authorization"), _TU(author));
        request.headers().add(_TU("Content-Type"), _TU("application/json"));
        request.set_request_uri(_TU("/info/leader"));
        request.set_method(methods::GET);
        response = hclient->request(request).get();
        UT_LOG() << _TS(response.to_string());
        UT_LOG() << "--------------------------------------";
        UT_EXPECT_EQ(response.status_code(), status_codes::OK);
        return 0;
    }

    int AddGraph() {
        rests_[0]->EvalCypher("default", "call dbms.graph.createGraph('g2','desc for g2',1)");
        auto js = rests_[0]->EvalCypher("default", "call dbms.graph.listGraphs()");
        UT_LOG() << js.serialize();
        return 0;
    }

    int Plugin() {
        // AddLabel();
        build_ha_so();

        auto ReadCode = [](const std::string& path) {
            std::string ret;
            fma_common::InputFmaStream ifs(path);
            ret.resize(ifs.Size());
            ifs.Read(&ret[0], ret.size());
            return ret;
        };

        std::string plugin_path, plugin_param;
        if (plugin_name == "scan") {
            plugin_path = "./scan_graph.so";
            plugin_param = "{\"scan_edges\":true, \"times\":1}";
        } else if (plugin_name == "echo") {
            plugin_path = "./echo_binary.so";
            plugin_param = "";
        } else if (plugin_name == "bfs") {
            plugin_path = "./breadth_first_search.so";
            // plugin_param = "{\"root_id\":\"Liam Neeson\", \"label\":\"Person\",
            // \"field\":\"name\"}";
            plugin_param = "{\"root_id\":\"001\", \"label\":\"v\", \"field\":\"id\"}";
        }

        rests_[0]->LoadPlugin("default", lgraph_api::PluginCodeType::SO,
                              lgraph::PluginDesc(plugin_name,
                                                 lgraph::plugin::PLUGIN_CODE_TYPE_CPP,
                                                 plugin_name,
                                                 lgraph::plugin::PLUGIN_VERSION_1,
                                                 read_only_plugin, ""),
                              ReadCode(plugin_path));

#pragma omp parallel for
        for (int i = 0; i < (int)n_ops; i++) {
            if (use_rest) {
                UT_LOG() << rests_[omp_get_thread_num()]->ExecutePlugin("default", true,
                                                                         plugin_name, plugin_param);
            } else {
                std::string res;
                UT_LOG() << rpcs_[omp_get_thread_num()]->CallProcedure(
                    res, "CPP", plugin_name, plugin_param, 0, false, "default");
            }
        }

        return 0;
    }
};

class TestHABase : public TuGraphTest {
    std::shared_ptr<lgraph::GlobalConfig> config = std::make_shared<lgraph::GlobalConfig>();

 protected:
    void SetUp() override {
        using namespace fma_common;
        using namespace lgraph;
        std::string dir = "./testdb";
        uint16_t http_port = 7091;
        uint16_t rpc_port = 8091;
        int n_clients = 16;

        config->db_dir = dir;
        config->bind_host = "127.0.0.1";
        config->ha_conf = "127.0.0.1:8091";
        config->http_port = http_port;
        config->rpc_port = rpc_port;
        config->enable_ha = true;
        config->verbose = 1;
        t.reset(new HaUnitTest(config, n_clients));

        fma_common::file_system::RemoveDir(t->config_->db_dir);
        // GraphFactory::create_yago(config_->db_dir);
        t->server_.reset();
        t->server_ = std::make_unique<lgraph::LGraphServer>(t->config_);
        t->server_->Start();
        for (auto& c : t->rests_) {
            c.reset();
            c = std::make_unique<RestClient>(FMA_FMT("http://{}:{}",
                                           t->config_->bind_host, t->config_->http_port));
            c->Login(lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
        }
        if (!use_rest) {
            for (auto& c : t->rpcs_) {
                c = std::make_unique<lgraph::RpcClient>(FMA_FMT("{}:{}",
                                                      t->config_->bind_host, t->config_->rpc_port),
                                              lgraph::_detail::DEFAULT_ADMIN_NAME,
                                              lgraph::_detail::DEFAULT_ADMIN_PASS);
            }
        }
    }
    void TearDown() override {
        for (auto& c : t->rests_) c.reset();
        for (auto& c : t->rpcs_) c.reset();
        t->server_->Stop();
        t->server_.reset();
        fma_common::file_system::RemoveDir(t->config_->db_dir);
    }
    std::shared_ptr<HaUnitTest> t;
};

TEST_F(TestHABase, HA) {
    std::string includes;
    std::string excludes;
    t->Run(includes, excludes);
}
