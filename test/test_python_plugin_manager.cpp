/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/configuration.h"
#include "fma-common/logging.h"

#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "db/galaxy.h"
#include "plugin/plugin_manager.h"
#include "./ut_utils.h"
using namespace fma_common;
#if LGRAPH_ENABLE_PYTHON_PLUGIN

struct ParamPythonPlgin {
    size_t j;
    size_t n;
};

class TestPythonPlugin : public TuGraphTestWithParam<struct ParamPythonPlgin> {};

class PluginTester : public lgraph::SingleLanguagePluginManager {
    friend class TestPythonPlugin_PythonPlugin_Test;

 public:
    PluginTester(lgraph::LightningGraph* db, const std::string& d, const std::string& t, size_t n)
        : lgraph::SingleLanguagePluginManager(
              db, "default", d, t,
              std::unique_ptr<lgraph::PythonPluginManagerImpl>(
                  new lgraph::PythonPluginManagerImpl(db, "default", d, n))) {}
};

TEST_P(TestPythonPlugin, PythonPlugin) {
    using namespace lgraph;
    using namespace python_plugin;
    std::string plugin_dir = "./python_plugin";
    size_t n_workers = 2, n_jobs = 3;
    n_workers = GetParam().j;
    n_jobs = GetParam().n;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(plugin_dir, "d,dir", true).Comment("Directory where plugin files are stored.");
    config.Add(n_workers, "j,workers", true).Comment("Number of Python worker threads to use.");
    config.Add(n_jobs, "n,jobs", true).Comment("Number of Python jobs.");
    config.ParseAndFinalize(argc, argv);
    const char* sleep_plugin = "./sleep.py";
    const char* read_plugin = "./scan_graph.py";
    const char* write_plugin = "./write_vertex.py";
    std::string code_sleep;
    std::string code_read;
    std::string code_write;
    {
        UT_LOG() << "Create test files";
        code_sleep = R"(
import time
def Process(db, input):
    t = 1
    try:
        t = int(input)
    except:
        return (False, 'input must be a time duration')
    print('sleeping for {} seconds'.format(t))
    time.sleep(t)
    return (True, '')
)";
        code_read = R"(
def Process(db, input):
    n = 0
    try:
        n = int(input)
    except:
        pass
    if n == 0:
        n = 1000000
    txn = db.CreateReadTxn()
    it = txn.GetVertexIterator()
    nv = 0
    while it.IsValid() and nv < n:
        nv = nv + 1
        it.Next()
    return (True, str(nv))
)";
        code_write = R"(
def Process(db, input):
    db.AddVertexLabel("v", [FieldSpec("id",FieldType.INT32,False),FieldSpec("name",FieldType.STRING,True)])
    txn = db.CreateWriteTxn()
    id = txn.AddVertex("v", {"id":1, "name":"001"})
    txn.Commit()
    return (True, str(id))
)";
    }

    try {
        fma_common::FileSystem::GetFileSystem("./testdb").RemoveDir("./testdb");
        fma_common::file_system::RemoveDir(plugin_dir);
        Galaxy galaxy("./testdb");
        AccessControlledDB db = galaxy.OpenGraph("admin", "default");
        std::string token = galaxy.GetUserToken("admin", "73@TuGraph");
        if (token.empty()) UT_ERR() << "Bad user/password.";

        UT_LOG() << "Testing normal actions";
        PluginTester manager(db.GetLightningGraph(), plugin_dir, "python_plugin", n_workers);
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(token, "sleep", code_sleep, plugin::CodeType::PY,
                                                  "sleep for n seconds", true));
        UT_LOG() << "Testing normal actions1";
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(token, "scan_graph", code_read,
                                                  plugin::CodeType::PY,
                                                  "scan graph for at most n vertices", true));
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(token, "add", code_write, plugin::CodeType::PY,
                                                  "write a vertex", true));
        UT_EXPECT_EQ(manager.procedures_.size(), 3);
        auto plugins = manager.ListPlugins(token);
        UT_EXPECT_EQ(plugins.size(), 3);
        UT_EXPECT_EQ(plugins[0].name, "add");
        UT_EXPECT_EQ(plugins[0].read_only, true);
        UT_EXPECT_EQ(plugins[2].name, "sleep");
        UT_EXPECT_EQ(plugins[2].read_only, true);  // we will set read_only to false later

        PluginCode pc;
        UT_LOG() << "Test retrieving plugin (code_type: py)";
        UT_EXPECT_TRUE(manager.GetPluginCode(token, "sleep", pc));
        UT_EXPECT_TRUE(code_sleep.compare(pc.code) == 0 && pc.code_type == "so_or_py");
        UT_EXPECT_TRUE(pc.read_only);
        UT_EXPECT_EQ(pc.desc, "sleep for n seconds");
        UT_EXPECT_EQ(pc.name, "sleep");

        UT_EXPECT_TRUE(manager.GetPluginCode(token, "scan_graph", pc));
        UT_EXPECT_TRUE(code_read.compare(pc.code) == 0 && pc.code_type == "so_or_py");
        UT_EXPECT_TRUE(manager.GetPluginCode(token, "add", pc));
        UT_EXPECT_TRUE(code_write.compare(pc.code) == 0 && pc.code_type == "so_or_py");

        UT_LOG() << "Updating plugin";
        // already exists
        UT_EXPECT_TRUE(!manager.LoadPluginFromCode(token, "add", code_write, plugin::CodeType::PY,
                                                   "write v2", false));
        UT_EXPECT_TRUE(manager.DelPlugin(token, "add"));
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(token, "add", code_write, plugin::CodeType::PY,
                                                  "write v2", false));
        plugins = manager.ListPlugins(token);
        UT_EXPECT_EQ(plugins.size(), 3);
        UT_EXPECT_EQ(plugins[0].name, "add");
        UT_EXPECT_EQ(plugins[0].desc, "write v2");
        UT_EXPECT_FALSE(plugins[0].read_only);  // now read_only is false

        UT_LOG() << "Calling plugins";
        std::string output;
        UT_EXPECT_TRUE(manager.Call(token, &db, "sleep", "1", 0, true, output));
        UT_LOG() << "Calling plugin with seperate process";
        double t1 = GetTime();
        std::vector<std::thread> threads;
        for (size_t i = 0; i < n_jobs; i++) {
            threads.emplace_back([&]() {
                std::string output;
                UT_EXPECT_TRUE(manager.Call(token, &db, "sleep", "1", 0, false, output));
            });
        }
        for (auto& t : threads) t.join();
        double t2 = GetTime();
        // UT_EXPECT_LT((t2 - t1), (n_jobs + n_workers - 1) /n_workers + 0.5);
        UT_EXPECT_TRUE(manager.Call(token, &db, "scan_graph", "0", 0, true, output));
        UT_LOG() << "Scan graph returned: " << output;
    } catch (std::exception& e) {
        UT_EXPECT_TRUE(false);
        UT_ERR() << e.what();
    }
    {
        UT_LOG() << "clean up";
        auto& fs = FileSystem::GetFileSystem("/");
        fs.Remove(sleep_plugin);
        fs.Remove(read_plugin);
        fs.Remove(write_plugin);
    }
}
INSTANTIATE_TEST_CASE_P(TestPythonPlugin, TestPythonPlugin,
                        testing::Values(ParamPythonPlgin{10, 10}, ParamPythonPlgin{3, 7}));
#endif
