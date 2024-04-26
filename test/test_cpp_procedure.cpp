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

#include <stdlib.h>

#include "fma-common/binary_buffer.h"
#include "fma-common/configuration.h"
#include "fma-common/string_formatter.h"
#include "./ut_utils.h"
#include "gtest/gtest.h"
#include "core/lightning_graph.h"
#include "db/galaxy.h"
#include "lgraph/lgraph.h"
#include "plugin/plugin_manager.h"

#include "./test_tools.h"
#include "./graph_factory.h"

class TestCppPlugin : public TuGraphTest {};

class PluginTester : public lgraph::SingleLanguagePluginManager {
    friend class TestCppPlugin_CppPlugin_Test;

 public:
    PluginTester(lgraph::LightningGraph* db, const std::string& d, const std::string& t)
        : lgraph::SingleLanguagePluginManager(
              lgraph::plugin::PLUGIN_LANG_TYPE_CPP, db, "default", d, t,
              std::unique_ptr<lgraph::CppPluginManagerImpl>(
                  new lgraph::CppPluginManagerImpl(db, "default", d))) {}
    ~PluginTester() {}
};

class CppPluginImplTester : public lgraph::CppPluginManagerImpl {
    friend class TestCppPlugin_CppPlugin_Test;
};

void build_so() {
    const std::string INCLUDE_DIR = "../../include";
    const std::string DEPS_INCLUDE_DIR = "../../deps/install/include";
    const std::string LIBLGRAPH = "./liblgraph.so";
    int rt;
#ifndef __clang__
#ifdef __SANITIZE_ADDRESS__
    std::string cmd_f =
        "g++ -fno-gnu-unique -fPIC -g --std=c++17  -Wl,-z,nodelete "
        " -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        " -o {} {} {} -shared ";
#else
    std::string cmd_f =
        "g++ -fno-gnu-unique -fPIC -g --std=c++17 -I {} -I {} -rdynamic -O3 -fopenmp -DNDEBUG "
        "-o {} {} {} -shared";
#endif
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
    cmd = UT_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./sortstr.so",
                 "../../test/test_procedures/sortstr.cpp", LIBLGRAPH);
    LOG_INFO() << "cmd_sort:" << cmd;
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = UT_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./scan_graph.so",
                 "../../test/test_procedures/scan_graph.cpp", LIBLGRAPH);
    LOG_INFO() << "cmd_scan_graph:" << cmd;
    rt = system(cmd.c_str());
    LOG_INFO() << "rt:" << rt;
    UT_EXPECT_EQ(rt, 0);
    cmd = UT_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./echo_binary.so",
                 "../../procedures/demo/echo_binary.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = UT_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./add_label.so",
                 "../../test/test_procedures/add_label_v.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = UT_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./bfs.so",
                 "../../test/test_procedures/bfs.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
    cmd = UT_FMT(cmd_f.c_str(), INCLUDE_DIR, DEPS_INCLUDE_DIR, "./v2_pagerank.so",
                 "../../test/test_procedures/v2_pagerank.cpp", LIBLGRAPH);
    rt = system(cmd.c_str());
    UT_EXPECT_EQ(rt, 0);
}

void read_code(const std::string& code_path, std::string& code) {
    auto& fs = fma_common::FileSystem::GetFileSystem(code_path);
    if (fs.FileExists(code_path)) {
        fma_common::InputFmaStream ifs(code_path, 0);
        size_t sz = ifs.Size();
        code.resize(sz);
        size_t ssz = ifs.Read(&code[0], sz);
        if (ssz != sz) {
            UT_ERR() << "Failed to read plugin file " << code_path;
        }
    } else {
        UT_ERR() << "Failed to find plugin file " << code_path;
    }
}

TEST_F(TestCppPlugin, CppPlugin) {
    using namespace fma_common;
    using namespace lgraph;
    std::string db_dir = "./lgraph_test_db";
    std::string plugin_table = "_cpp_plugin_";
    std::string plugin_dir = db_dir + "/cpp_plugin";

    AutoCleanDir _1(db_dir);
    AutoCleanDir _2(plugin_dir);

    GraphFactory::WriteYagoFiles();
    SubProcess import_client(
        FMA_FMT("./lgraph_import -c yago.conf -d {}", db_dir));
    import_client.Wait();
    UT_EXPECT_EQ(import_client.GetExitCode(), 0);

    UT_LOG() << "Test Begin...";
    Galaxy galaxy(db_dir);
    AccessControlledDB db = galaxy.OpenGraph("admin", "default");
    std::string token = galaxy.GetUserToken("admin", "73@TuGraph");
    if (token.empty()) UT_ERR() << "Bad user/password.";
#ifdef _WIN32
#if 0  // disable plugin test on windows for now. it is too hard to get it working
    // test load
    PluginTester pm(db.GetLightningGraph(), plugin_dir, plugin_table);
    std::string code_scan_graph, code_add_label;
    read_code("./plugin_scan_graph.dll", code_scan_graph);
    read_code("./plugin_add_label.dll", code_add_label);

    UT_LOG() << "Load scan_graph plugin";
    UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph",
            code_scan_graph, plugin::CodeType::SO, "scan graph v1", true, "v1"));
    // already exists
    UT_EXPECT_TRUE(!pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph",
                code_scan_graph, plugin::CodeType::SO, "scan graph", true, "v1"));
    UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"));
    UT_EXPECT_TRUE(!pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                                "scan_graph"));  // does not exist
    UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph",
                code_scan_graph, plugin::CodeType::SO, "scan graph v2", true, "v1"));

    UT_LOG() << "Load add_label plugin";
    UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label",
                code_add_label, plugin::CodeType::SO, "add label v1", false, "v1"));

    UT_LOG() << "Test loaded plugins info";
    auto& funcs = pm.procedures_;
    UT_EXPECT_EQ(funcs.size(), 2);
    auto& scan_graph = funcs["_fma_scan_graph"];
    UT_EXPECT_EQ(scan_graph->desc, "scan graph v2");
    UT_EXPECT_EQ(scan_graph->read_only, true);
    auto& add_label = funcs["_fma_add_label"];
    UT_EXPECT_EQ(add_label->desc, "add label v1");
    UT_EXPECT_EQ(add_label->read_only, false);

    // test call
    std::string output;
    UT_LOG() << "Test call plugin";
    UT_EXPECT_TRUE(!pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "no_such_plugin",
                            "{\"scan_edges\":true}", 2, true, output));
    // bad argument causes Process to return false and output is used to return error
    // message
    UT_EXPECT_THROW_CODE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "scan_graph",
        "{\"scan_edges\":true, \"times\":2}", 0, true, output),
                    InputError);
    UT_EXPECT_TRUE(StartsWith(output, "error parsing json: "));

    // calling add_label
    UT_EXPECT_TRUE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "add_label",
                           "{\"label\":\"vertex1\"}", 0, true, output));
    // second insert fails because label already exists and Process returns false
    UT_EXPECT_THROW_CODE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "add_label",
                            "{\"label\":\"vertex1\"}", 0, true, output),
                    InputError);

    {
        lgraph_api::GraphDB gdb(&db, true, false);
        auto txn = gdb.CreateReadTxn();
        auto labels = txn.ListVertexLabels();
        UT_EXPECT_EQ(labels.size(), 1);
        UT_EXPECT_EQ(labels.front(), "vertex1");
    }
    {
        UT_EXPECT_EQ(pm.IsReadOnlyPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"), 1);
        UT_EXPECT_EQ(pm.IsReadOnlyPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label"), 0);
        UT_EXPECT_EQ(pm.IsReadOnlyPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                                                        "does_not_exist"), -1);
    }
    {
        UT_LOG() << "Test delete";
        {
            // PluginTester pm(&db, plugin_dir, plugin_table);
            UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"));
            UT_EXPECT_TRUE(!pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"));
            UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label"));
            UT_EXPECT_TRUE(pm.procedures_.empty());
        }
        {
            PluginTester pm(db.GetLightningGraph(), plugin_dir, plugin_table);
            UT_EXPECT_TRUE(pm.procedures_.empty());
            UT_EXPECT_TRUE(pm.ListPlugins(lgraph::_detail::DEFAULT_ADMIN_NAME).empty());
        }
    }
#endif
#else
    std::string code_so = "";
    std::string code_cpp = "";
    std::string code_zip = "";
    std::string code_scan_graph = "";
    std::string code_add_label = "";
    std::string code_bfs = "";
    std::string code_v2_pagerank = "";
    std::string multi_procedure_name = "multi_files.cpp";
    std::string code_multi_procedure = "";
    std::string multi_core_name = "multi_files_core.cpp";
    std::string code_multi_core = "";
    std::string multi_header_name = "multi_files.h";
    std::string code_multi_header = "";
    {
        // read file to string
        std::string code_so_path = "./sortstr.so";
        std::string code_cpp_path = "../../test/test_procedures/sortstr.cpp";
        std::string multi_procedure_path = "../../test/test_procedures/multi_files.cpp";
        std::string multi_header_path = "../../test/test_procedures/multi_files.h";
        std::string multi_core_path = "../../test/test_procedures/multi_files_core.cpp";
#ifndef __clang__
        std::string code_zip_path = "../../test/test_procedures/sortstr.zip";
#elif __APPLE__
        std::string code_zip_path = "../../test/test_procedures/sortstr_osx.zip";
#else
        std::string code_zip_path = "../../test/test_procedures/sortstr_clang.zip";
#endif
        std::string code_scan_graph_path = "./scan_graph.so";
        std::string code_add_label_path = "./add_label.so";
        std::string code_bfs_path = "./bfs.so";
        std::string code_v2_pagerank_path = "./v2_pagerank.so";
        build_so();
        read_code(code_so_path, code_so);
        read_code(code_cpp_path, code_cpp);
        read_code(code_zip_path, code_zip);
        read_code(code_scan_graph_path, code_scan_graph);
        read_code(code_add_label_path, code_add_label);
        read_code(code_bfs_path, code_bfs);
        read_code(code_v2_pagerank_path, code_v2_pagerank);
        read_code(multi_procedure_path, code_multi_procedure);
        read_code(multi_core_path, code_multi_core);
        read_code(multi_header_path, code_multi_header);
        UT_EXPECT_NE(code_so, "");
        UT_EXPECT_NE(code_cpp, "");
        UT_EXPECT_NE(code_zip, "");
        UT_EXPECT_NE(code_scan_graph, "");
        UT_EXPECT_NE(code_bfs_path, "");
        UT_EXPECT_NE(code_v2_pagerank, "");
        UT_EXPECT_NE(code_multi_procedure, "");
        UT_EXPECT_NE(code_multi_core, "");
        UT_EXPECT_NE(code_multi_header, "");
    }
    {
        // test PluginInfo
        UT_LOG() << "Test PluginInfo serialize/deserialize";
        BinaryBuffer buf;
        std::unique_ptr<PluginInfoBase> pinfo(new CppPluginImplTester::PluginInfo());
        pinfo->desc = "scan graph";
        pinfo->read_only = true;
        size_t s = BinaryWrite(buf, *pinfo);
        std::unique_ptr<PluginInfoBase> pinfo2(new CppPluginImplTester::PluginInfo());
        size_t t = BinaryRead(buf, *pinfo2);
        UT_EXPECT_EQ(s, t);
        UT_EXPECT_EQ(pinfo->desc, pinfo2->desc);
        UT_EXPECT_EQ(pinfo->read_only, pinfo2->read_only);
    }
    {
        UT_LOG() << "Test loading empty DB";
        PluginTester pm(db.GetLightningGraph(), plugin_dir, plugin_table);
        UT_EXPECT_TRUE(pm.procedures_.empty());

        // test exception branch
        UT_EXPECT_THROW_CODE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "#name",
                                                   std::vector<std::string>{},
                                                   std::vector<std::string>{"invalid.so"},
                                                   plugin::CodeType::SO, "desc", true, "v1"),
                             InputError);

        UT_LOG() << "Test loading empty plugin code";
        UT_EXPECT_THROW_CODE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "name",
                                                   std::vector<std::string>{""},
                                                   std::vector<std::string>{"empty.so"},
                                                   plugin::CodeType::SO, "desc", true, "v1"),
                             InputError);

        UT_LOG() << "Test loading invalid plugin code (code_type: so)";
        UT_EXPECT_THROW_CODE(
            pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "name",
                                  std::vector<std::string>{"invalid_code"},
                                  std::vector<std::string>{"invalid.so"},
                                  plugin::CodeType::SO, "desc", true, "v1"),
            InputError);

        UT_LOG() << "Test loading invalid plugin code (code_type: zip)";

        UT_EXPECT_THROW_CODE(
            pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "name",
                                  std::vector<std::string>{"invalid_code"},
                                  std::vector<std::string>{"invalid.zip"},
                                  plugin::CodeType::ZIP, "desc", true, "v1"),
            InputError);

        UT_LOG() << "Test loading invalid plugin code (code_type: cpp)";
        UT_EXPECT_THROW_CODE(
            pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "name",
                                  std::vector<std::string>{"invalid_code"},
                                  std::vector<std::string>{"invalid.cpp"},
                                  plugin::CodeType::CPP, "desc", true, "v1"),
            InputError);

        UT_EXPECT_TRUE(pm.procedures_.empty());
    }
    {
        // test load
        PluginTester pm(db.GetLightningGraph(), plugin_dir, plugin_table);

        UT_LOG() << "Test loading of plugins (code_type: so)";
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_so",
                                             std::vector<std::string>{code_so},
                                             std::vector<std::string>{"sort.so"},
                                             plugin::CodeType::SO, "sortstr so", true,
                                             "v1"));

        UT_LOG() << "Test loading of plugins (code_type: cpp)";
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_cpp",
                                             std::vector<std::string>{code_cpp},
                                             std::vector<std::string>{"sort.cpp"},
                                             plugin::CodeType::CPP, "sortstr cpp", true,
                                             "v1"));

        UT_LOG() << "Test loading multiple files for cpp";
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "multi_file",
                                             std::vector<std::string>{
                                                 code_multi_core,
                                                 code_multi_procedure,
                                                 code_multi_header
                                             },
                                             std::vector<std::string>{
                                                 multi_core_name,
                                                 multi_procedure_name,
                                                 multi_header_name
                                             },
                                             plugin::CodeType::CPP, "multiple files", true,
                                             "v1"));

        UT_LOG() << "Test loading of plugins (code_type: zip)";
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_zip",
                                             std::vector<std::string>{code_zip},
                                             std::vector<std::string>{"sort.zip"},
                                             plugin::CodeType::ZIP, "sortstr zip", true,
                                             "v1"));

        PluginCode pc;
        UT_LOG() << "Test retrieving plugin (code_type: so)";
        UT_EXPECT_TRUE(pm.GetPluginCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_so", pc));
        UT_EXPECT_TRUE(code_so.compare(pc.code) == 0 && pc.code_type == "so_or_py");
        UT_EXPECT_EQ(pc.read_only, true);
        UT_EXPECT_EQ(pc.desc, "sortstr so");
        UT_EXPECT_EQ(pc.name, "sortstr_so");

        UT_LOG() << "Test retrieving plugin (code_type: zip)";
        UT_EXPECT_TRUE(pm.GetPluginCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_zip", pc));
        UT_EXPECT_TRUE(pc.code.compare(code_zip) == 0 && pc.code_type == "zip");

        UT_LOG() << "Test retrieving plugin (code_type: cpp)";
        UT_EXPECT_TRUE(pm.GetPluginCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_cpp", pc));
        std::string code_cpp_merged;
        UT_EXPECT_NO_THROW(code_cpp_merged = pm.MergeCodeFiles(std::vector<std::string>{code_cpp},
                                                               std::vector<std::string>{"sort.cpp"},
                                                               "sortstr_cpp"));
        UT_EXPECT_TRUE(pc.code.compare(code_cpp_merged) == 0 && pc.code_type == "cpp");

        UT_LOG() << "Load scan_graph plugin";
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph",
                                             std::vector<std::string>{code_scan_graph},
                                             std::vector<std::string>{"scan.so"},
                                             plugin::CodeType::SO, "scan graph v1",
                                             true, "v1"));
        UT_EXPECT_TRUE(!pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph",
                                              std::vector<std::string>{code_scan_graph},
                                              std::vector<std::string>{"scan.so"},
                                              plugin::CodeType::SO, "scan graph",
                                              true, "v1"));
        UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"));
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph",
                                             std::vector<std::string>{code_scan_graph},
                                             std::vector<std::string>{"scan.so"},
                                             plugin::CodeType::SO, "scan graph v2",
                                             true, "v1"));

        UT_LOG() << "Load add_label plugin";
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label",
                                             std::vector<std::string>{code_add_label},
                                             std::vector<std::string>{"add_label.so"},
                                             plugin::CodeType::SO, "add label v1",
                                             false, "v1"));
        UT_LOG() << "Load v2_pagerank plugin";
        UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "v2_pagerank",
                                             std::vector<std::string>{code_v2_pagerank},
                                             std::vector<std::string>{"v2_pagerank.so"},
                                             plugin::CodeType::SO,
                                             "v2 pagerank", true, "v2"));

        UT_LOG() << "Test loaded plugins info";
        auto& funcs = pm.procedures_;
        UT_EXPECT_EQ(funcs.size(), 7);
        auto& zip = funcs["_fma_sortstr_zip"];
        UT_EXPECT_EQ(zip->desc, "sortstr zip");
        UT_EXPECT_EQ(zip->read_only, true);
        auto& scan_graph = funcs["_fma_scan_graph"];
        UT_EXPECT_EQ(scan_graph->desc, "scan graph v2");
        UT_EXPECT_EQ(scan_graph->read_only, true);
        auto& add_label = funcs["_fma_add_label"];
        UT_EXPECT_EQ(add_label->desc, "add label v1");
        UT_EXPECT_EQ(add_label->read_only, false);
        auto& v2_pagerank = funcs["_fma_v2_pagerank"];
        UT_EXPECT_EQ(v2_pagerank->desc, "v2 pagerank");
        UT_EXPECT_EQ(v2_pagerank->read_only, true);

        // test call
        std::string output;
        UT_LOG() << "Test call plugin";
        UT_EXPECT_TRUE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "sortstr_so",
                               "dbac", 0, true, output));
        UT_EXPECT_EQ(output, "abcd");

        UT_EXPECT_TRUE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "scan_graph",
                               "{\"scan_edges\":true, \"times\":2}", 0, true, output));
        UT_EXPECT_EQ(output, "{\"num_edges\":56,\"num_vertices\":42}");
        UT_EXPECT_TRUE(!pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "no_such_plugin",
                                "{\"scan_edges\":true}", 2, true, output));
        // bad argument causes Process to return false and output is used to return error
        // message
        UT_EXPECT_THROW_CODE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db,
                                     "scan_graph", "", 0, true, output),
                        InputError);
        UT_EXPECT_TRUE(StartsWith(output, "error parsing json: "));

        // calling add_label
        UT_EXPECT_TRUE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "add_label",
                               "{\"label\":\"vertex1\"}", 0, true, output));
        // second insert fails because label already exists and Process returns false
        UT_EXPECT_THROW_CODE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "add_label",
                                "{\"label\":\"vertex1\"}", 0, true, output),
                        InputError);

        {  // test call v2
            Result output_v2;
            UT_LOG() << "Test call v2 plugin";
            lgraph_api::GraphDB gdb(&db, true, false);
            auto txn = gdb.CreateReadTxn();
            UT_EXPECT_TRUE(pm.CallV2(&txn, lgraph::_detail::DEFAULT_ADMIN_NAME,
                                     nullptr, "v2_pagerank",
                                     "{\"num_iteration\": 10}",
                                     0, true, output_v2));
            UT_EXPECT_EQ(output_v2.Size(), txn.GetNumVertices());
        }

        {
            lgraph_api::GraphDB gdb(&db, true, false);
            auto txn = gdb.CreateReadTxn();
            auto labels = txn.ListVertexLabels();
            UT_EXPECT_EQ(labels.size(), 4);
            UT_EXPECT_EQ(labels.back(), "vertex1");
        }
        {
            UT_EXPECT_EQ(pm.IsReadOnlyPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"), 1);
            UT_EXPECT_EQ(pm.IsReadOnlyPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label"), 0);
        }
        {
            UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label"));
            UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label",
                                                 std::vector<std::string>{code_add_label},
                                                 std::vector<std::string>{"add_label.so"},
                                                 plugin::CodeType::SO,
                                                 "add label v2", true, "v1"));
            // since add_label is now declared read-only, it should fail with an exception
            UT_EXPECT_THROW_CODE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME,
                                         &db, "add_label", "{\"label\":\"vertex1\"}",
                                         0, true, output),
                            InputError);
        }
        {
            UT_LOG() << "Test delete";
            {
                // PluginTester pm(&db, plugin_dir, plugin_table);
                UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"));
                UT_EXPECT_TRUE(!pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph"));
                UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "add_label"));
                UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_so"));
                UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_cpp"));
                UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_zip"));
                UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "multi_file"));
                UT_EXPECT_TRUE(
                    pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "v2_pagerank"));

                // pm.UnloadAllPlugins();
                // pm.DeleteAllPlugins("admin");
                // UT_LOG() << "del succ";

                UT_EXPECT_ANY_THROW(
                    pm.IsReadOnlyPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                        "scan_graph"));
                UT_EXPECT_TRUE(pm.procedures_.empty());
            }
            {
                PluginTester pm(db.GetLightningGraph(), plugin_dir, plugin_table);
                UT_EXPECT_TRUE(pm.procedures_.empty());
            }
        }

        {
            UT_LOG() << "Testing repeated load/delete";
            UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "scan_graph",
                                                 std::vector<std::string>{code_scan_graph},
                                                 std::vector<std::string>{"scan.so"},
                                                 plugin::CodeType::SO,
                                                 "scan graph v1", true, "v1"));
            UT_EXPECT_TRUE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db, "scan_graph",
                                   "{\"scan_edges\":true, \"times\":2}", 0, true, output));
            for (size_t i = 0; i < 300; i++) {
                UT_EXPECT_TRUE(pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                                                     "sortstr_so",
                                                     std::vector<std::string>{code_so},
                                                     std::vector<std::string>{"sort.so"},
                                                     plugin::CodeType::SO,
                                                     "sortstr so", true, "v1"));
                std::string output;
                UT_EXPECT_TRUE(pm.Call(nullptr, lgraph::_detail::DEFAULT_ADMIN_NAME, &db,
                                       "sortstr_so", "dbac", 0, true, output));
                UT_EXPECT_TRUE(pm.DelPlugin(lgraph::_detail::DEFAULT_ADMIN_NAME, "sortstr_so"));
            }
            pm.DeleteAllPlugins(lgraph::_detail::DEFAULT_ADMIN_NAME);
            UT_EXPECT_ANY_THROW(
                pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME, "testa",
                                      std::vector<std::string>{"#include <stdlib.h>"},
                                      std::vector<std::string>{"testa.cpp"},
                                      (plugin::CodeType)6,
                                      "test", true, "v1"));
        }
#ifndef __SANITIZE_ADDRESS__
        {
            UT_LOG() << "Testing load many plugins";
            for (int i = 0; i < 32; i++) {
                UT_LOG() << "try load bfs_" << i;
                bool r = false;
                UT_EXPECT_NO_THROW(
                    r = pm.LoadPluginFromCode(lgraph::_detail::DEFAULT_ADMIN_NAME,
                         "bfs_" + std::to_string(i),
                         std::vector<std::string>{code_bfs}, std::vector<std::string>{"bfs.so"},
                         plugin::CodeType::SO,
                         "bfs v1", true, "v1"));
                UT_EXPECT_TRUE(r);
                fma_common::SleepS(1);
            }
            pm.DeleteAllPlugins(lgraph::_detail::DEFAULT_ADMIN_NAME);
        }
#endif  // __SANITIZE_ADDRESS__
    }
#endif
}
