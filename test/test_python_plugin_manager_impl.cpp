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

#include <boost/interprocess/ipc/message_queue.hpp>

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/utils.h"

#include "gtest/gtest.h"

#include "core/lightning_graph.h"
#include "db/galaxy.h"
#include "plugin/plugin_context.h"
#include "tiny-process-library/process.hpp"
#include "plugin/python_plugin.h"
#include "./ut_utils.h"

#if LGRAPH_ENABLE_PYTHON_PLUGIN
class TestPythonPluginManagerImpl : public TuGraphTest {};

void printout(const char* bytes, size_t n) { UT_LOG() << std::string(bytes, n); }

class Tester : public lgraph::PythonPluginManagerImpl {
    friend class TestPythonPluginManagerImpl_PythonPluginManagerImpl_Test;

 public:
    Tester(lgraph::LightningGraph* db, const std::string& dir)
        : lgraph::PythonPluginManagerImpl(db, "default", dir) {}

    Tester(const std::string& db_dir, size_t db_size, const std::string& plugin_dir,
           int max_idle_seconds)
        : lgraph::PythonPluginManagerImpl("default", db_dir, db_size, plugin_dir,
                                          max_idle_seconds) {}

    size_t GetNFree() {
        std::lock_guard<std::mutex> l(_mtx);
        return _free_processes.size();
    }
};

void WriteScanGraphPlugin(const std::string& path) {
    fma_common::OutputFmaStream out(path, 0);
    const std::string code =
        R"(
import json
from lgraph_python import *

def Process(db, input):
    scan_edges = False
    try:
        param = json.loads(input)
        scan_edges = param['scan_edges']
    except:
        pass
    txn = db.CreateReadTxn()
    num_vertices = 0
    num_edges = 0
    vit = txn.GetVertexIterator()
    while vit.IsValid():
        num_vertices += 1
        if scan_edges:
            eit = vit.GetOutEdgeIterator()
            while eit.IsValid():
                num_edges += 1
                eit.Next()
        vit.Next()
    output = {"num_vertices":num_vertices}
    if scan_edges:
        output["num_edges"] = num_edges
    return (True, json.dumps(output))
        )";
    out.Write(code.data(), code.size());
    out.Close();
}

void WriteEchoPlugin(const std::string& path) {
    fma_common::OutputFmaStream out(path, 0);
    const std::string code = R"(
import traceback
import time

def Process(db, input):
    print('echo ({})'.format(input))
    return (True, input)
    )";
    out.Write(code.data(), code.size());
    out.Close();
}

void WriteSleepPlugin(const std::string& path) {
    fma_common::OutputFmaStream out(path, 0);
    const std::string code = R"(
import time
def Process(db, input):
    t = 1
    try:
        t = float(input)
    except:
        return (False, 'input must be a time duration')
    print('sleeping for {} seconds'.format(t))
    time.sleep(t)
    return (True, 'slept for {} seconds'.format(t))
    )";
    out.Write(code.data(), code.size());
    out.Close();
}

TEST_F(TestPythonPluginManagerImpl, PythonPluginManagerImpl) {
    using namespace fma_common;
    using namespace lgraph;
    std::string plugin_dir = "./python_plugin";
    std::string db_dir = "./testdb";
    int n_processes = 3;
    int max_idle_seconds = 2;
    int argc = _ut_argc;
    char** argv = _ut_argv;
    Configuration config;
    config.Add(db_dir, "db", true).Comment("DB dir");
    config.Add(plugin_dir, "plugin", true).Comment("Plugin dir");
    config.Add(n_processes, "n,processes", true).Comment("Number of processes");
    config.Add(max_idle_seconds, "i,idle", true).Comment("Max idle seconds for python process");
    config.ParseAndFinalize(argc, argv);
    fma_common::file_system::RemoveDir(db_dir);

    Galaxy galaxy(db_dir);
    std::string user = "admin";
    AccessControlledDB db = galaxy.OpenGraph(user, "default");

    {
        UT_WARN() << "Testing load plugin";
        Tester manager(db_dir, (size_t)1 << 30, plugin_dir, max_idle_seconds);
        fma_common::FileSystem::GetFileSystem("./").Mkdir(plugin_dir);
        WriteEchoPlugin(plugin_dir + "/echo.py");
        auto* pinfo = manager.CreatePluginInfo();
        pinfo->read_only = true;
        manager.LoadPlugin(user, "echo", pinfo);
        // current implementation allows repeated load
        manager.LoadPlugin(user, "echo", pinfo);

        UT_LOG() << "Testing call plugin";
        std::string output;
        UT_LOG() << "Testing call plugin1111";

        manager.DoCall(user, &db, "echo", pinfo, "hello", 0, true, output);
        UT_EXPECT_EQ(output, "hello");

        // currently delete has no effect, just return success
        UT_LOG() << "Testing del plugin";
        manager.UnloadPlugin(user, "echo", pinfo);

        // since delete has no effect, this should work
        UT_LOG() << "Testing call plugin";
        manager.DoCall(user, &db, "echo", pinfo, "hello", 0, true, output);
        UT_EXPECT_EQ(output, "hello");

        // test timeout
        UT_LOG() << "Testing timeout with sleep";
        delete pinfo;
        pinfo = manager.CreatePluginInfo();
        WriteSleepPlugin(plugin_dir + "/sleep.py");
        manager.LoadPlugin(user, "sleep", pinfo);
        UT_LOG() << "Calling sleep for 0.1";
        manager.DoCall(user, &db, "sleep", pinfo, "0.1", 0, true, output);
        UT_LOG() << "output: " << output;
        UT_LOG() << "Calling sleep for 1.5 with timeout 1";
        UT_EXPECT_THROW(manager.DoCall(user, &db, "sleep", pinfo, "1.5", 0.5, true, output),
                        InputError);
        UT_LOG() << "output: " << output;
        manager.DoCall(user, &db, "sleep", pinfo, "0.1", 0, true, output);
        UT_LOG() << "Calling sleep for 0.2 with timeout 2";
        manager.DoCall(user, &db, "sleep", pinfo, "0.2", 2, true, output);
        UT_LOG() << "output: " << output;

        // test idle process kill
        UT_LOG() << "Testing idle process handling";
        UT_EXPECT_EQ(manager.GetNFree(), 1);
        // now start more processes
        std::vector<std::thread> thrs;
        for (size_t i = 0; i < n_processes; i++) {
            thrs.emplace_back([i, &manager, &db, pinfo, &user]() {
                std::string out;
                manager.DoCall(user, &db, "sleep", pinfo, "0.1", 2, true, out);
            });
        }
        for (auto& thr : thrs) thr.join();
        UT_EXPECT_EQ(manager.GetNFree(), n_processes);
        fma_common::SleepS(max_idle_seconds + 0.4);
        // this call will clean all the processes
        manager.DoCall(user, &db, "sleep", pinfo, "0.1", 2, true, output);
        UT_EXPECT_EQ(manager.GetNFree(), 0);
        thrs.clear();
        for (size_t i = 0; i < 3; i++) {
            thrs.emplace_back([i, &manager, &db, pinfo, &user]() {
                std::string out;
                manager.DoCall(user, &db, "sleep", pinfo, std::to_string(i), 0, true, out);
            });
        }
        for (auto& thr : thrs) thr.join();
        manager.DoCall(user, &db, "sleep", pinfo, "1.4", 2, true, output);
        UT_EXPECT_EQ(manager.GetNFree(), std::min(max_idle_seconds - 1, 3));
        std::string path("sleep");
        auto res = manager.GetPluginPath(path);
        UT_LOG() << res;
        // manager.GetTaskName();
        UT_LOG() << manager.GetPluginDir();
        UT_EXPECT_ANY_THROW(manager.LoadPlugin(user, "testpy", pinfo));
    }
}
#endif
