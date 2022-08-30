/* Copyright (c) 2022 AntGroup. All Rights Reserved. */

#include "fma-common/configuration.h"
#include "fma-common/logger.h"
#include "fma-common/unit_test_utils.h"

#include "tiny-process-library/process.hpp"

#include "lgraph/lgraph_rpc_client.h"
#include "server/state_machine.h"

#include "./graph_factory.h"
#include "./test_tools.h"

class TestBackupRestore : public TuGraphTest {};

TEST_F(TestBackupRestore, BackupRestore) {
    using namespace lgraph;
    const std::string& admin_user = lgraph::_detail::DEFAULT_ADMIN_NAME;
    const std::string& admin_pass = lgraph::_detail::DEFAULT_ADMIN_PASS;
#define FMT fma_common::StringFormatter::Format

    std::string db_dir = "./testdb";
    std::string new_db_dir = "./newdb";
    uint16_t port = 17172;
    uint16_t rpc_port = 19192;

    auto StartServer = [&](const std::string& dir) -> std::unique_ptr<SubProcess> {
        std::string server_cmd =
            FMT("./lgraph_server -c lgraph_standalone.json --port {} --rpc_port {}"
                " --enable_backup_log true --host 127.0.0.1 --verbose 1 --directory {}" ,
                port, rpc_port, dir);
        auto server = std::unique_ptr<SubProcess>(new SubProcess(server_cmd));
        if (!server->ExpectOutput("Server started.")) {
            FMA_WARN() << "Server failed to start, stderr:\n" << server->Stderr();
        }
        return server;
    };

    UT_LOG() << "Setting up env";
    AutoCleanDir d1(db_dir);

    UT_LOG() << "Create db";
    {
        auto server = StartServer(db_dir);
        // create yago files
        GraphFactory::WriteYagoFiles();
        SubProcess import_client(
            FMT("./lgraph_import --online true -c yago.conf -r http://127.0.0.1:{} -u {} -p {}",
                port, admin_user, admin_pass));
        import_client.Wait();
        FMA_CHECK_EQ(import_client.GetExitCode(), 0);
        // ok, now check imported data
        RpcClient rpc_client(FMT("127.0.0.1:{}", rpc_port), admin_user, admin_pass);
        std::string res;
        bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        FMA_CHECK_EQ(succeed, true);
        web::json::value v = web::json::value::parse(res);
        FMA_CHECK_EQ(v["count(n)"].as_integer(), 21);
    }
    std::string snapshot;
    auto& fs = fma_common::FileSystem::GetFileSystem(db_dir);
    auto dirs = fs.ListSubDirs(db_dir + "/snapshot");
    FMA_CHECK_EQ(dirs.size(), 1);
    snapshot = dirs[0];

    UT_LOG() << "Restore in remote mode.";
    {
        AutoCleanDir d2(new_db_dir);
        fs.CopyToLocal(snapshot, new_db_dir);
        auto server = StartServer(new_db_dir);
        RpcClient rpc_client(FMT("127.0.0.1:{}", rpc_port), admin_user, admin_pass);
        std::string res;
        bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        FMA_CHECK_EQ(succeed, true);
        // bug #790, match n return count(n) returns empty result when db is empty
        // FMA_ASSERT(resp.binary_result().result().empty() ||
        // resp.binary_result().result()[0].values()[0].int64_() == 0);
        // now restore
        SubProcess restore_client(
            FMT("./lgraph_binlog -a restore --host 127.0.0.1 --port {} -u {} -p {} -f {}", rpc_port,
                admin_user, admin_pass, db_dir + "/binlog/*"));
        restore_client.Wait();
        FMA_CHECK_EQ(restore_client.GetExitCode(), 0);
        succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        FMA_CHECK_EQ(succeed, true);
        web::json::value v = web::json::value::parse(res);
        FMA_CHECK_EQ(v["count(n)"].as_integer(), 21);
    }

    UT_LOG() << "Restore in local mode.";
    {
        AutoCleanDir d2(new_db_dir);
        fs.CopyToLocal(snapshot, new_db_dir);
        SubProcess restore_client(FMT("./lgraph_binlog -a restore --db_dir {} -u {} -p {} -f {}",
                                      new_db_dir, admin_user, admin_pass, db_dir + "/binlog/*"));
        restore_client.Wait();
        FMA_CHECK_EQ(restore_client.GetExitCode(), 0);
        auto server = StartServer(new_db_dir);
        RpcClient rpc_client(FMT("127.0.0.1:{}", rpc_port), admin_user, admin_pass);
        std::string res;
        bool succeed = rpc_client.CallCypher(res, "match (n) return count(n)");
        FMA_CHECK_EQ(succeed, true);
        web::json::value v = web::json::value::parse(res);
        FMA_CHECK_EQ(v["count(n)"].as_integer(), 21);
    }
    return 0;
}
