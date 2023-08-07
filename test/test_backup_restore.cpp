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
#include "fma-common/logger.h"

#include "tiny-process-library/process.hpp"
#include "./ut_utils.h"

#include "lgraph/lgraph_rpc_client.h"
#include "server/state_machine.h"

#include "./graph_factory.h"
#include "./test_tools.h"

class TestBackupRestore : public TuGraphTest {};

TEST_F(TestBackupRestore, BackupRestore) {
    using namespace lgraph;
    const std::string& admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string& admin_pass = lgraph::_detail::DEFAULT_ADMIN_PASS;

    std::string db_dir = "./testdb";
    std::string new_db_dir = "./newdb";
    uint16_t port = 17172;
    uint16_t rpc_port = 19192;

    AutoCleanDir d1(db_dir);
    AutoCleanDir d2(new_db_dir);
    auto StartServer = [&](const std::string& dir) -> std::unique_ptr<SubProcess> {
#ifndef __SANITIZE_ADDRESS__
        std::string server_cmd = FMA_FMT(
            "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
            " --enable_backup_log true --host 127.0.0.1 --verbose 1 --directory {}",
            port, rpc_port, dir);
#else
        std::string server_cmd = FMA_FMT(
            "./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
            " --enable_backup_log true --host 127.0.0.1 --verbose 1 --directory {} "
            "--use_pthread true",
            port, rpc_port, dir);
#endif
        auto server = std::unique_ptr<SubProcess>(new SubProcess(server_cmd));
        if (!server->ExpectOutput("Server started.")) {
            UT_WARN() << "Server failed to start, stderr:\n" << server->Stderr();
        }
        return server;
    };

    UT_LOG() << "Setting up env";

    UT_LOG() << "Create db";
    {
        auto server = StartServer(db_dir);
        // create yago files
        GraphFactory::WriteYagoFiles();
        SubProcess import_client(
            FMA_FMT("./lgraph_import --online true -c yago.conf -r http://127.0.0.1:{} -u {} -p {}",
                    port, admin_user, admin_pass));
        import_client.Wait();
        UT_EXPECT_EQ(import_client.GetExitCode(), 0);
        // ok, now check imported data
        RpcClient rpc_client(FMA_FMT("127.0.0.1:{}", rpc_port), admin_user, admin_pass);
        std::string res;
        bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        UT_EXPECT_EQ(succeed, true);
        web::json::value v = web::json::value::parse(res);
        UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 21);
    }
    std::string snapshot;
    auto& fs = fma_common::FileSystem::GetFileSystem(db_dir);
    auto dirs = fs.ListSubDirs(db_dir + "/snapshot");
    UT_EXPECT_EQ(dirs.size(), 1);
    snapshot = dirs[0];

    UT_LOG() << "Restore in remote mode.";
    {
        AutoCleanDir d2(new_db_dir);
        fs.CopyToLocal(snapshot, new_db_dir);
        auto server = StartServer(new_db_dir);
        RpcClient rpc_client(FMA_FMT("127.0.0.1:{}", rpc_port), admin_user, admin_pass);
        std::string res;
        bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        UT_EXPECT_EQ(succeed, true);
        // bug #790, match n return count(n) returns empty result when db is empty
        // FMA_ASSERT(resp.binary_result().result().empty() ||
        // resp.binary_result().result()[0].values()[0].int64_() == 0);
        // now restore
        SubProcess restore_client(
            FMA_FMT("./lgraph_binlog -a restore --host 127.0.0.1 --port {} -u {} -p {} -f {}",
                    rpc_port, admin_user, admin_pass, db_dir + "/binlog/*"));
        restore_client.Wait();
        UT_EXPECT_EQ(restore_client.GetExitCode(), 0);
        succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        UT_EXPECT_EQ(succeed, true);
        web::json::value v = web::json::value::parse(res);
        UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 21);
    }

    UT_LOG() << "Restore in local mode.";
    {
        AutoCleanDir d2(new_db_dir);
        fs.CopyToLocal(snapshot, new_db_dir);
        SubProcess restore_client(
            FMA_FMT("./lgraph_binlog -a restore --db_dir {} -u {} -p {} -f {}", new_db_dir,
                    admin_user, admin_pass, db_dir + "/binlog/*"));
        restore_client.Wait();
        UT_EXPECT_EQ(restore_client.GetExitCode(), 0);
        auto server = StartServer(new_db_dir);
        RpcClient rpc_client(FMA_FMT("127.0.0.1:{}", rpc_port), admin_user, admin_pass);
        std::string res;
        bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        UT_EXPECT_EQ(succeed, true);
        web::json::value v = web::json::value::parse(res);
        UT_EXPECT_EQ(v[0]["count(n)"].as_integer(), 21);
    }
}
