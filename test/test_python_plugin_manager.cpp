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

#ifdef _WIN32
// disable python plugin on windows
// TODO(hjk41): re-enable python plugin on windows
#else

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
        : lgraph::SingleLanguagePluginManager(lgraph::plugin::PLUGIN_LANG_TYPE_PYTHON,
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
from liblgraph_python_api import *
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
        AccessControlledDB db = galaxy.OpenGraph(lgraph::_detail::DEFAULT_ADMIN_NAME, "default");
        std::string token = galaxy.GetUserToken(lgraph::_detail::DEFAULT_ADMIN_NAME, "73@TuGraph");
        if (token.empty()) UT_ERR() << "Bad user/password.";

        UT_LOG() << "Testing normal actions";
        PluginTester manager(db.GetLightningGraph(), plugin_dir, "python_plugin", n_workers);
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                "sleep", code_sleep, plugin::CodeType::PY, "sleep for n seconds", true, "v1"));
        UT_LOG() << "Testing normal actions1";
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                "scan_graph", code_read, plugin::CodeType::PY,
                                "scan graph for at most n vertices", true, "v1"));
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                "add", code_write, plugin::CodeType::PY, "write a vertex", true, "v1"));
        UT_EXPECT_EQ(manager.procedures_.size(), 3);
        auto plugins = manager.ListPlugins(lgraph::_detail::DEFAULT_ADMIN_NAME);
        UT_EXPECT_EQ(plugins.size(), 3);
        UT_EXPECT_EQ(plugins[0].name, "add");
        UT_EXPECT_EQ(plugins[0].read_only, true);
        UT_EXPECT_EQ(plugins[2].name, "sleep");
        UT_EXPECT_EQ(plugins[2].read_only, true);  // we will set read_only to false later

        PluginCode pc;
        UT_LOG() << "Test retrieving plugin (code_type: py)";
        UT_EXPECT_TRUE(manager.GetPluginCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sleep", pc));
        UT_EXPECT_TRUE(code_sleep.compare(pc.code) == 0 && pc.code_type == "py");
        UT_EXPECT_TRUE(pc.read_only);
        UT_EXPECT_EQ(pc.desc, "sleep for n seconds");
        UT_EXPECT_EQ(pc.name, "sleep");

        UT_EXPECT_TRUE(manager.GetPluginCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                                                    "scan_graph", pc));
        UT_EXPECT_TRUE(code_read.compare(pc.code) == 0 && pc.code_type == "py");
        UT_EXPECT_TRUE(manager.GetPluginCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "add", pc));
        UT_EXPECT_TRUE(code_write.compare(pc.code) == 0 && pc.code_type == "py");

        UT_LOG() << "Updating plugin";
        // already exists
        UT_EXPECT_TRUE(!manager.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                    "add", code_write, plugin::CodeType::PY, "write v2", false, "v1"));
        UT_EXPECT_TRUE(manager.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "add"));
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                    "add", code_write, plugin::CodeType::PY, "write v2", false, "v1"));
        plugins = manager.ListPlugins(lgraph::_detail::DEFAULT_ADMIN_NAME);
        UT_EXPECT_EQ(plugins.size(), 3);
        UT_EXPECT_EQ(plugins[0].name, "add");
        UT_EXPECT_EQ(plugins[0].desc, "write v2");
        UT_EXPECT_FALSE(plugins[0].read_only);  // now read_only is false

        UT_LOG() << "Calling plugins";
        std::string output;
        UT_EXPECT_TRUE(manager.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME,
                                            &db, "sleep", "1", 0, true, output));
        UT_LOG() << "Calling plugin with seperate process";
        std::vector<std::thread> threads;
        for (size_t i = 0; i < n_jobs; i++) {
            threads.emplace_back([&]() {
                std::string output;
                UT_EXPECT_TRUE(manager.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME,
                                            &db, "sleep", "1", 0, false, output));
            });
        }
        for (auto& t : threads) t.join();
        UT_EXPECT_TRUE(manager.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db,
                                            "scan_graph", "0", 0, true, output));
        UT_LOG() << "Scan graph returned: " << output;

        // reload plugin
        UT_EXPECT_TRUE(manager.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db,
                                    "sleep", "1", 0, true, output));
        // code sleep return ""
        UT_EXPECT_TRUE(output == "");
        UT_EXPECT_TRUE(manager.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "sleep"));
        UT_EXPECT_TRUE(manager.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                                  "sleep", code_read, plugin::CodeType::PY,
                                                  "code read but name sleep", true, "v1"));
        UT_EXPECT_TRUE(manager.GetPluginCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sleep", pc));
        UT_EXPECT_TRUE(code_read.compare(pc.code) == 0 && pc.code_type == "py");
        UT_EXPECT_TRUE(pc.read_only);
        UT_EXPECT_EQ(pc.desc, "code read but name sleep");
        UT_EXPECT_EQ(pc.name, "sleep");
        UT_EXPECT_TRUE(manager.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db,
                                    "sleep", "0", 0, true, output));
        // code read return "0"
        UT_EXPECT_TRUE(output == "0");
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
#endif
