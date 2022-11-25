/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/configuration.h"
#include "fma-common/logging.h"

#include "gtest/gtest.h"
#include "./ut_utils.h"

#include "restful/server/rest_server.h"
#include "client/cpp/restful/rest_client.h"
#include "fma-common/string_formatter.h"
#include "import/import_online.h"
#include "./graph_factory.h"

#include "./test_tools.h"

using namespace lgraph::import_v2;
using namespace fma_common;
using namespace lgraph;

class EmbeddedServer {
    std::thread thr_;
    std::atomic<bool> exit_flag_;

 public:
    EmbeddedServer(const std::string &db_dir, const std::string &host, uint16_t port)
        : exit_flag_(false) {
        thr_ = std::thread([&]() {
            lgraph::StateMachine::Config sm_config;
            sm_config.db_dir = db_dir;
            fma_common::file_system::RemoveDir(sm_config.db_dir);
            lgraph::StateMachine state_machine(sm_config, nullptr);
            auto res = state_machine.GetMasterRestAddr();
            UT_EXPECT_EQ(res, "");
            state_machine.Start();
            lgraph::RestServer::Config rest_config;
            rest_config.host = host;
            rest_config.port = port;
            rest_config.use_ssl = false;
            std::string url;
            lgraph::RestServer rest_server(&state_machine, rest_config);
            while (!exit_flag_) fma_common::SleepS(0);
        });
    }

    ~EmbeddedServer() {
        exit_flag_ = true;
        thr_.join();
    }
};

class TestImportOnline : public TuGraphTest {
 protected:
    std::string host;
    uint16_t port;
    uint16_t rpc_port;
    std::string db_dir;
    std::string db_name;

    void SetUp() {
        TuGraphTest::SetUp();
        host = "127.0.0.1";
        port = 6464;
        rpc_port = 16464;
        db_dir = "./lgraph_db";
        db_name = "default";
    }

    void WriteFile(const std::string &name, const std::string &content) {
        OutputFmaStream out(name);
        out.Write(content.data(), content.size());
    }
};

TEST_F(TestImportOnline, AddLabel) {
    int argc = _ut_argc;
    char **argv = _ut_argv;
    Configuration config;
    config.Add(host, "host", true).Comment("Host address");
    config.Add(port, "port", true).Comment("HTTP port");
    config.ParseAndFinalize(argc, argv);

    UT_LOG() << "Testing AddLabel.";
    // start server
    lgraph::StateMachine::Config sm_config;
    sm_config.db_dir = db_dir;
    fma_common::file_system::RemoveDir(sm_config.db_dir);
    lgraph::StateMachine state_machine(sm_config, nullptr);
    auto res = state_machine.GetMasterRestAddr();
    UT_EXPECT_EQ(res, "");
    state_machine.Start();
    lgraph::RestServer::Config rest_config;
    rest_config.host = host;
    rest_config.port = port;
    rest_config.use_ssl = false;

    std::string url;
    lgraph::RestServer rest_server(&state_machine, rest_config);
    url = fma_common::StringFormatter::Format("http://{}:{}/", host, port);
    RestClient client(url);
    client.Login(lgraph::_detail::DEFAULT_ADMIN_NAME, lgraph::_detail::DEFAULT_ADMIN_PASS);
    std::vector<FieldSpec> fds;
    fds.emplace_back(FieldSpec("name", FieldType::STRING, false));
    fds.emplace_back(FieldSpec("address", FieldType::STRING, false));
    fds.emplace_back(FieldSpec("scale", FieldType::INT32, true));
    auto res_b = client.AddVertexLabel("default", "vertex", fds, "name");
    UT_EXPECT_EQ(res_b, true);
    res_b = client.AddEdgeLabel("default", "vertex", fds);
    UT_EXPECT_EQ(res_b, true);
}
struct Store_store {
    std::string config_label_index = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "bool", "type":"BOOL",  "index":true},
                {"name" : "int8", "type":"INT8",   "index":true},
                {"name" : "int16", "type":"INT16",  "index":true},
                {"name" : "int32", "type":"INT32",  "index":true},
                {"name" : "int64", "type":"INT64",  "index":true},
                {"name" : "float", "type":"FLOAT",  "index":true},
                {"name" : "double", "type":"DOUBLE","index":true},
                {"name" : "data", "type":"DATE",    "index":true},
                {"name" : "datetime", "type":"DATETIME", "index":true},
                {"name" : "string", "type":"STRING", "index":true}
            ]
        },
        {
            "label" : "node1",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ]
}
                      )";
    std::string config_label_compatible = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ]
}
                      )";
    std::string config_add_schema_and_csv = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ]
}
                          )";
    std::string config_blob = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "img1", "type":"BLOB", "optional":true},
                {"name" : "img2", "type" :"BLOB", "optional":true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "img", "type":"BLOB", "optional":true},
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["img1","id","img2"]
        },
        {
            "path" : "./import_data/node2.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["img1","id","img2"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","img","weight"]
        },
        {
            "path" : "./import_data/edge2.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","img","weight"]
        }
    ]
}
                      )";
    std::string config_jsonline = R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "id2", "type":"INT64", "unique":true},
                {"name" : "comment", "type":"STRING", "optional":true}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "name", "type":"STRING", "optional":true},
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","id2","SKIP","comment"]
        },
        {
            "path" : "./import_data/node2.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","id2", "comment"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","name","SKIP", "weight"]
        },
        {
            "path" : "./import_data/edge2.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","name","weight"]
        }
    ]
}
                      )";
} data_source;
TEST_F(TestImportOnline, ImportOnline) {
    {
        std::string dir = "./import_data";
        AutoCleanDir cleaner(dir);
        AutoCleanDir db_cleaner(db_dir);

        // std::string import_cfg = dir + "/import.config";
        // std::string schema_cfg = dir + "/schema.config";
        std::string config_file = dir + "/import.conf";
        std::string v1 = dir + "/node1.csv";
        std::string v2 = dir + "/node2.csv";
        std::string e1 = dir + "/edge1.csv";
        std::string e2 = dir + "/edge2.csv";

        auto StartServer = [&]() {
            std::string server_cmd = UT_FMT(
                "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
                " --enable_backup_log true --host 127.0.0.1 --verbose 1 --directory {}" ,
                port, rpc_port, db_dir);
            UT_LOG() << "cmd: " << server_cmd;
            auto server = std::unique_ptr<SubProcess>(new SubProcess(server_cmd));
            if (!server->ExpectOutput("Server started.")) {
                UT_WARN() << "Server failed to start, stderr:\n" << server->Stderr();
            }
            return server;
        };

        auto StartServerWithFTIndextEnabled = [&]() {
            std::ifstream ifd("lgraph_standalone.json");
            nlohmann::json config;
            ifd >> config;
            ifd.close();
            config["enable_fulltext_index"] = true;
            std::ofstream ofd("lgraph_standalone_ft.json");
            ofd << config;
            ofd.close();
            std::string server_cmd = UT_FMT(
                "./lgraph_server -c lgraph_standalone_ft.json --port {} --rpc_port {}"
                " --enable_backup_log true --host 127.0.0.1 --verbose 1 --directory {}",
                port, rpc_port, db_dir);
            UT_LOG() << "cmd: " << server_cmd;
            auto server = std::unique_ptr<SubProcess>(new SubProcess(server_cmd));
            if (!server->ExpectOutput("Server started.")) {
                UT_WARN() << "Server failed to start, stderr:\n" << server->Stderr();
            }
            return server;
        };

        auto TryImport = [&](const std::string &expect_output, int expect_ec,
                             std::string config_file_, bool continue_on_error = false) {
            std::string import_cmd = UT_FMT(
                "./lgraph_import --online true --config_file \"{}\" -r http://127.0.0.1:{} "
                "--continue_on_error {} -u {} -p {}",
                config_file_, port, continue_on_error, lgraph::_detail::DEFAULT_ADMIN_NAME,
                lgraph::_detail::DEFAULT_ADMIN_PASS);

            UT_LOG() << "cmd: " << import_cmd;
            SubProcess proc(import_cmd);
            UT_EXPECT_TRUE(proc.ExpectOutput(expect_output, 10 * 1000));
            proc.Wait();
            UT_EXPECT_EQ(expect_ec, proc.GetExitCode());
        };

        auto ValidateGraph = [&](const std::function<void(lgraph_api::GraphDB &)> &validate) {
            lgraph_api::Galaxy galaxy(db_dir);
            galaxy.SetCurrentUser(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                  lgraph::_detail::DEFAULT_ADMIN_PASS);
            lgraph_api::GraphDB graph = galaxy.OpenGraph("default");
            validate(graph);
        };

        {
            UT_LOG() << "Testing label index";
            db_cleaner.Clean();
            auto server = StartServer();

            WriteFile(config_file, data_source.config_label_index);
            TryImport("Import finished", 0, config_file);
            server.reset();
        }

        {
            UT_LOG() << "Testing add label compatible";
            db_cleaner.Clean();
            auto server = StartServer();
            WriteFile(config_file, data_source.config_label_compatible);
            TryImport("Import finished", 0, config_file);

            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "name", "type":"STRING"}
            ]
        },
        {
            "label" : "node2",
            "type" : "VERTEX",
            "primary" : "uid",
            "properties" : [
                {"name" : "uid", "type":"INT32"}
            ]
        },
        {
            "label" : "edge2",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"DOUBLE"}
            ]
        }
    ]
}
                      )");
            TryImport("Import finished", 0, config_file);
            server.reset();
        }

        {
            UT_LOG() << "Testing skip";
            db_cleaner.Clean();
            auto server = StartServer();

            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["SKIP","id","SKIP"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SKIP","SRC_ID","SKIP","DST_ID","weight","SKIP"]
        }
    ]
}
                )");
            WriteFile(v1, "skip,1,skip\nskip,2,skip\n");
            WriteFile(e1, "skip,1,skip,2,0.1,skip\nskip,1,skip,1,0.2,skip\n");
            TryImport("Import finished", 0, config_file);
            server.reset();
        }

        {
            UT_LOG() << "Testing add schema and csv individual";
            WriteFile(v1, "1\n2\n");
            WriteFile(e1, "1,2,0.1\n1,1,0.2\n");
            db_cleaner.Clean();
            {
                WriteFile(config_file, data_source.config_add_schema_and_csv);

                // online schema only
                auto server = StartServer();
                TryImport("Import finished", 0, config_file);
                server.reset();
            }
            {
                WriteFile(config_file, R"(
{
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "CSV",
            "label" : "node",
            "columns" : ["id"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node",
            "DST_ID" : "node",
            "columns" : ["SRC_ID","DST_ID","weight"]
        }
    ]
}
                          )");
                // online csv only
                auto server = StartServer();
                TryImport("Import finished", 0, config_file);
                server.reset();
            }

            ValidateGraph([](lgraph_api::GraphDB &db) {
                auto txn = db.CreateReadTxn();
                size_t num_vertices = 0;
                size_t num_edges = 0;
                for (auto vit = txn.GetVertexIterator(); vit.IsValid(); vit.Next()) {
                    num_vertices += 1;
                    num_edges += vit.GetNumOutEdges();
                }
                UT_LOG() << "scaned num_vertex :" << num_vertices
                         << " scaned num_edge :" << num_edges;
                UT_EXPECT_EQ(2, num_vertices);
                UT_EXPECT_EQ(2, num_edges);
            });
        }

        {
            UT_LOG() << "Testing lgraph_import --online blob";
            WriteFile(config_file, data_source.config_blob);
            WriteFile(v1, StringFormatter::Format("{},1,{}\n{},2,{}", std::string(8196, 'a'),
                                                  std::string(8192, 'a'), std::string(52, 'b'),
                                                  std::string(56, 'b')));
            WriteFile(v2, StringFormatter::Format("{},3,{}\n{},4,{}", "", std::string(1024, 'a'),
                                                  "", std::string(2048, 'b')));
            WriteFile(e1, StringFormatter::Format("1,2,{},1.0\n2,3,{},2.0", std::string(100, 'e'),
                                                  std::string(204, 'f')));
            WriteFile(e2, StringFormatter::Format("3,4,{},3.0\n4,1,{},4.0", std::string(1048, 'e'),
                                                  std::string(2096, 'f')));
            db_cleaner.Clean();
            auto server = StartServer();
            TryImport("Import finished", 0, config_file);
            server.reset();
            ValidateGraph([](lgraph_api::GraphDB &g) {
                auto txn = g.CreateReadTxn();
                auto it1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
                UT_EXPECT_EQ(it1.GetField("img1").AsBase64Blob(), std::string(8196, 'a'));
                auto eit12 = it1.GetOutEdgeIterator();
                UT_EXPECT_EQ(txn.GetVertexIterator(eit12.GetDst()).GetField("id").AsInt32(), 2);
                UT_EXPECT_EQ(eit12.GetField("img").AsBase64Blob(), std::string(100, 'e'));
                auto it4 = txn.GetVertexByUniqueIndex("node", "id", FieldData(4));
                UT_EXPECT_TRUE(it4.GetField("img1").IsNull());
                auto eit34 = it4.GetInEdgeIterator();
                UT_EXPECT_EQ(txn.GetVertexIterator(eit34.GetSrc()).GetField("id").AsInt32(), 3);
                UT_EXPECT_EQ(eit34.GetField("img").AsBase64Blob(), std::string(1048, 'e'));
            });
        }

        {
            UT_LOG() << "Testing import jsonline";
            WriteFile(config_file, data_source.config_jsonline);
            WriteFile(v1, R"(
[1,  2, "test", "aaaa"]
[2,  3, null, "aaaa"]
["3",4, "test", null]
["4",5, "test", "aaaa"]
                      )");
            WriteFile(v2, R"(
[5,  6, "test", "cccc"]
[6,  7, null, ""]
["7",8, "test", null]
["8",9, "test", "aaaa"]
                      )");
            WriteFile(e1, R"(
[1,  2, "test", "skip",  5]
[1,  3, null,   "skip",  7]
[1,  4, "test", "skip",  8]
[1,  5, "",     "",      9]
                      )");
            WriteFile(e2, R"(
[2,  5, "test", 10]
[3,  4, null,   11]
[4,  3, "test", 12]
[5,  2, "",     13]
                      )");
            db_cleaner.Clean();
            auto server = StartServer();
            TryImport("Import finished", 0, config_file);
            server.reset();
            ValidateGraph([](lgraph_api::GraphDB &g) {
                auto txn = g.CreateReadTxn();
                UT_EXPECT_EQ(txn.GetNumVertices(), 8);
                auto iter1 = txn.GetVertexByUniqueIndex("node", "id", FieldData(1));
                UT_EXPECT_EQ(iter1.GetField("id2").AsInt64(), 2);

                auto iter2 = txn.GetVertexByUniqueIndex("node", "id", FieldData(3));
                UT_EXPECT_EQ(iter2.GetField("id2").AsInt64(), 4);
                UT_EXPECT_TRUE(iter2.GetField("comment").is_null());
                auto eiter1 = iter2.GetOutEdgeIterator();
                UT_EXPECT_EQ(eiter1.GetField("weight").AsFloat(), 11);
                UT_EXPECT_TRUE(eiter1.GetField("name").is_null());

                auto eiter2 = iter2.GetInEdgeIterator();
                UT_EXPECT_EQ(eiter2.GetField("weight").AsFloat(), 7);
                eiter2.Next();
                UT_EXPECT_TRUE(eiter2.IsValid());
                UT_EXPECT_EQ(eiter2.GetField("weight").AsFloat(), 12);
                eiter2.Next();
                UT_EXPECT_TRUE(!eiter2.IsValid());
            });
        }

        {
            UT_LOG() << "Testing import jsonline with eatra space";
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                      )");
            WriteFile(v1,
                      R"(
    ["1","name1"]
            
["2","name2"]

   ["3","name3"]
["4","name4"]
               ["5","name5"]
                    )");
            db_cleaner.Clean();
            auto server = StartServer();
            TryImport("Import finished", 0, config_file);
            server.reset();
            ValidateGraph([](lgraph_api::GraphDB &g) {
                auto txn = g.CreateReadTxn();
                UT_EXPECT_EQ(txn.GetNumVertices(), 5);
                auto iter1 = txn.GetVertexByUniqueIndex("node", "id", FieldData("1"));
                UT_EXPECT_EQ(iter1.GetField("name").AsString(), "name1");
                auto iter2 = txn.GetVertexByUniqueIndex("node", "id", FieldData("2"));
                UT_EXPECT_EQ(iter2.GetField("name").AsString(), "name2");
                auto iter3 = txn.GetVertexByUniqueIndex("node", "id", FieldData("3"));
                UT_EXPECT_EQ(iter3.GetField("name").AsString(), "name3");
                auto iter4 = txn.GetVertexByUniqueIndex("node", "id", FieldData("4"));
                UT_EXPECT_EQ(iter4.GetField("name").AsString(), "name4");
                auto iter5 = txn.GetVertexByUniqueIndex("node", "id", FieldData("5"));
                UT_EXPECT_EQ(iter5.GetField("name").AsString(), "name5");
            });
        }

        {
            UT_LOG() << "Testing import jsonline with continue_on_error = true";
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"},
                {"name" : "name", "type":"STRING"}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","name"]
        }
    ]
}
                      )");
            WriteFile(v1, R"(
["1","name1"]
["2","name2"]
[sdacdsc]
["3","name3"]
["4"]
["4","name4"]
12345
["5","name5"]
                      )");
            db_cleaner.Clean();
            auto server = StartServer();
            TryImport("Import finished", 0, config_file, true);
            server.reset();
            ValidateGraph([](lgraph_api::GraphDB &g) {
                auto txn = g.CreateReadTxn();
                UT_EXPECT_EQ(txn.GetNumVertices(), 5);
                auto iter1 = txn.GetVertexByUniqueIndex("node", "id", FieldData("1"));
                UT_EXPECT_EQ(iter1.GetField("name").AsString(), "name1");
                auto iter2 = txn.GetVertexByUniqueIndex("node", "id", FieldData("2"));
                UT_EXPECT_EQ(iter2.GetField("name").AsString(), "name2");
                auto iter3 = txn.GetVertexByUniqueIndex("node", "id", FieldData("3"));
                UT_EXPECT_EQ(iter3.GetField("name").AsString(), "name3");
                auto iter4 = txn.GetVertexByUniqueIndex("node", "id", FieldData("4"));
                UT_EXPECT_EQ(iter4.GetField("name").AsString(), "name4");
                auto iter5 = txn.GetVertexByUniqueIndex("node", "id", FieldData("5"));
                UT_EXPECT_EQ(iter5.GetField("name").AsString(), "name5");
            });
        }

        {
            UT_LOG() << "Testing import jsonline exception, null column";
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "id2", "type":"INT64", "unique":true},
                {"name" : "comment", "type":"STRING", "optional":true}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","id2","SKIP","comment"]
        }
    ]
}
                      )");
            WriteFile(v1, R"(
[null,  null, "test", "aaaa"]
[2,  3, null, "aaaa"]
["3",4, "test", null]
["4",5, "test", "aaaa"]
                      )");

            db_cleaner.Clean();
            auto server = StartServer();
            TryImport("json reading failed", 1, config_file);
        }

        {
            UT_LOG() << "Testing import jsonline exception, missing column";
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "id2", "type":"INT64", "unique":true},
                {"name" : "comment", "type":"STRING", "optional":true}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","id2","SKIP","comment"]
        }
    ]
}
                      )");
            WriteFile(v1, R"(
[1,  2, "test", "aaaa"]
[2,  3]
["3",4, "test", null]
["4",5, "test", "aaaa"]
                      )");

            db_cleaner.Clean();
            auto server = StartServer();
            TryImport("json reading failed", 1, config_file);
        }

        {
            UT_LOG() << "Testing import jsonline exception, type cast error";
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "id2", "type":"INT64", "unique":true},
                {"name" : "comment", "type":"STRING", "optional":true}
            ]
        }
    ],
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node",
            "columns" : ["id","id2","SKIP","comment"]
        }
    ]
}
                      )");
            WriteFile(v1, R"(
[1,  2, "test", "aaaa"]
[2,  3, "test", "aaaa"]
["aaaaa",4, "test", null]
["4",5, "test", "aaaa"]
                      )");

            db_cleaner.Clean();
            auto server = StartServer();
            TryImport("json reading failed", 1, config_file);
        }

        {
            db_cleaner.Clean();
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node1", 
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        },
        {
            "label" : "node2", 
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ],
            "constraints" : [["node1", "node2"]]
        }
    ]
}
                          )");

            // online schema only
            auto server = StartServer();
            TryImport("Import finished", 0, config_file);

            WriteFile(config_file, R"(
{
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "CSV",
            "label" : "node1",
            "columns" : ["id"]
        },
        {
            "path" : "./import_data/node2.csv",
            "format" : "CSV",
            "label" : "node2",
            "columns" : ["id"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "CSV",
            "label" : "edge",
            "SRC_ID" : "node1",
            "DST_ID" : "node2",
            "columns" : ["SRC_ID","DST_ID","weight"]
        }
    ]
}
                          )");
            WriteFile(v1, "1\n2\n3\n");
            WriteFile(v2, "1\n2\n3\n");
            WriteFile(e1, "1,1,1.1\n2,2,2.2\n3,3,3.3\n");
            TryImport("Import finished", 0, config_file);

            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "edge1",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ],
            "constraints" : [["node1", "node1"]]
        }
    ]
}
                          )");
            TryImport("Import finished", 0, config_file);

            WriteFile(config_file, R"(
{
    "files" : [
        {
            "path" : "./import_data/edge1.csv",
            "format" : "CSV",
            "label" : "edge1",
            "SRC_ID" : "node1",
            "DST_ID" : "node2",
            "columns" : ["SRC_ID","DST_ID","weight"]
        }
    ]
}
                          )");
            WriteFile(e1, "1,1,1.1\n2,2,2.2\n3,3,3.3\n");
            TryImport("not meet the edge constraints", 1, config_file);
            server.reset();
        }

        {
            db_cleaner.Clean();
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node1",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"},
                {"name" : "name", "type":"STRING", "fulltext" : true}
            ]
        },
        {
            "label" : "node2",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"INT32"}
            ]
        },
        {
            "label" : "edge",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"},
                {"name" : "desc", "type":"STRING", "fulltext" : true}
            ],
            "constraints" : [["node1", "node2"]]
        }
    ]
}
                          )");
            auto server = StartServerWithFTIndextEnabled();
            TryImport("Import finished", 0, config_file);
            WriteFile(config_file, R"(
{
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node1",
            "columns" : ["id", "name"]
        },
        {
            "path" : "./import_data/node2.csv",
            "format" : "JSON",
            "label" : "node2",
            "columns" : ["id"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "JSON",
            "label" : "edge",
            "SRC_ID" : "node1",
            "DST_ID" : "node2",
            "columns" : ["SRC_ID","DST_ID","weight", "desc"]
        }
    ]
}
                          )");
            WriteFile(v1, R"([1,"name name1"]
[2, "name name2"]
[3, "name name3"]
)");
            WriteFile(v2, R"([1]
[2]
[3]
)");
            WriteFile(e1, R"([1,1,1.1, "desc desc1"]
[2,2,1.1, "desc desc2"]
[3,3,1.1, "desc desc3"]
)");
            TryImport("Import finished", 0, config_file);
        }
        {
            db_cleaner.Clean();
            auto server = StartServer();
            WriteFile(config_file, R"(
{
    "schema": [
        {
            "label" : "node1",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"}
            ]
        },
        {
            "label" : "node2",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"}
            ]
        },
        {
            "label" : "node3",
            "type" : "VERTEX",
            "primary" : "id",
            "properties" : [
                {"name" : "id", "type":"STRING"}
            ]
        },
        {
            "label" : "node1_node2",
            "type" : "EDGE",
            "properties" : [
                {"name" : "weight", "type":"FLOAT"}
            ],
            "constraints" : [["node1", "node2"]]
        }
    ]
}
                          )");
            TryImport("Import finished", 0, config_file);
            WriteFile(config_file, R"(
{
    "files" : [
        {
            "path" : "./import_data/node1.csv",
            "format" : "JSON",
            "label" : "node1",
            "columns" : ["id"]
        },
        {
            "path" : "./import_data/node2.csv",
            "format" : "JSON",
            "label" : "node3",
            "columns" : ["id"]
        },
        {
            "path" : "./import_data/edge1.csv",
            "format" : "JSON",
            "label" : "node1_node2",
            "SRC_ID" : "node1",
            "DST_ID" : "node3",
            "columns" : ["SRC_ID","DST_ID","weight"]
        }
    ]
}
                          )");
            WriteFile(v1, R"(["1"])");
            WriteFile(v2, R"(["3"])");
            WriteFile(e1, R"(["1","3",1.1])");
            TryImport("Does not meet the edge constraints", 1, config_file);
        }
    }
}
