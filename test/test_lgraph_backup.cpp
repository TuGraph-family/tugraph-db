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

#include "fma-common/logger.h"
#include "fma-common/string_util.h"

#include "gtest/gtest.h"
#include "./test_tools.h"
#include "client/cpp/restful/rest_client.h"
#include "server/lgraph_server.h"
#include "lgraph/lgraph.h"

extern void build_so();

class TestLGraphBackupTool : public TuGraphTest {};

TEST_F(TestLGraphBackupTool, LGraphBackupTool) {
    std::shared_ptr<lgraph::GlobalConfig> config_ = std::make_shared<lgraph::GlobalConfig>();
    std::unique_ptr<lgraph::LGraphServer> server_;
    std::unique_ptr<RestClient> rest_;
    config_->db_dir = "testdb";
    config_->bind_host = "127.0.0.1";
    config_->http_port = 17173;
    config_->rpc_port = 19193;
    config_->verbose = 2;
    std::string username = "admin";
    std::string password = "73@TuGraph";
    std::string plugin_name = "scan_graph";
    std::string plugin_path = "./scan_graph.so";
    std::string plugin_param = "{\"scan_edges\":true, \"times\":1}";
    std::string backup_path = "testdb.bak";
    bool read_only_plugin = true;
    // server
    fma_common::file_system::RemoveDir(config_->db_dir);
    fma_common::file_system::RemoveDir(backup_path);
    server_ = std::make_unique<lgraph::LGraphServer>(config_);
    server_->Start();

    // client
    rest_ = std::make_unique<RestClient>(
        UT_FMT("http://{}:{}", config_->bind_host, config_->http_port));
    rest_->Login(username, password);
    // install
    auto ReadCode = [](const std::string& path) {
        std::string ret;
        fma_common::InputFmaStream ifs(path);
        ret.resize(ifs.Size());
        ifs.Read(&ret[0], ret.size());
        return ret;
    };
    build_so();
    rest_->LoadPlugin(
        "default", lgraph_api::PluginCodeType::SO,
        lgraph::PluginDesc(plugin_name, lgraph::plugin::PLUGIN_CODE_TYPE_CPP, plugin_name,
                           lgraph::plugin::PLUGIN_VERSION_1, read_only_plugin, ""),
        ReadCode(plugin_path));
    // backup
    lgraph::SubProcess dumper(
        UT_FMT("./lgraph_backup --src {} --dst {}", config_->db_dir, backup_path));
    dumper.Wait();
    // restart server
    server_->Stop();
    config_->db_dir = backup_path;
    server_ = std::make_unique<lgraph::LGraphServer>(config_);
    server_->Start();
    // call plugin
    rest_ = std::make_unique<RestClient>(
        UT_FMT("http://{}:{}", config_->bind_host, config_->http_port));
    rest_->Login(username, password);
    UT_LOG() << rest_->ExecutePlugin("default", true, plugin_name, plugin_param);
    server_->Stop();
}
